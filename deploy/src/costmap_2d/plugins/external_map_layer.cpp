
#include <pluginlib/class_list_macros.h>
#include <costmap_2d/external_map_layer.h>
#include <costmap_2d/costmap_math.h>

PLUGINLIB_EXPORT_CLASS(costmap_2d::ExternalMapLayer, costmap_2d::Layer)

using costmap_2d::LETHAL_OBSTACLE;
using costmap_2d::INSCRIBED_INFLATED_OBSTACLE;
using costmap_2d::NO_INFORMATION;

namespace costmap_2d
{

  ExternalMapLayer::ExternalMapLayer() :
  new_data_available_(false), 
  has_updated_data_(false),
  dsrv_(NULL)
{
}

ExternalMapLayer::~ExternalMapLayer()
{
  if (dsrv_)
    delete dsrv_;
}

void ExternalMapLayer::onInitialize()
{
  ros::NodeHandle nh("~/" + name_);
  current_ = true;
  enabled_ = true;
  new_data_available_ = false;
  has_updated_data_ = false;

  update_min_distance_ = 0.1; // 默认移动距离阈值
  last_robot_x_ = last_robot_y_ = std::numeric_limits<double>::infinity();

  // 获取全局坐标系
  global_frame_ = layered_costmap_->getGlobalFrameID();
  
  // 订阅外部地图话题
  ros::NodeHandle g_nh;
  external_map_sub_ = g_nh.subscribe("/perception/local_mapping_cartographer", 1, 
                                    &ExternalMapLayer::externalMapCallback, this);
  
  // 动态参数配置
  dsrv_ = new dynamic_reconfigure::Server<costmap_2d::GenericPluginConfig>(nh);
  dynamic_reconfigure::Server<costmap_2d::GenericPluginConfig>::CallbackType cb = 
      boost::bind(&ExternalMapLayer::reconfigureCB, this, _1, _2);
  dsrv_->setCallback(cb);
}

void ExternalMapLayer::reconfigureCB(costmap_2d::GenericPluginConfig &config, uint32_t level)
{
  enabled_ = config.enabled;
}

void ExternalMapLayer::activate()
{
  onInitialize();
}

void ExternalMapLayer::deactivate()
{
  // 清理资源
  external_map_sub_.shutdown();
}

void ExternalMapLayer::reset()
{
  new_data_available_ = false;
  has_updated_data_ = false;
  deactivate();
  onInitialize();
}

void ExternalMapLayer::externalMapCallback(const nav_msgs::OccupancyGrid::ConstPtr& msg)
{
  if (!enabled_) return;
  
  // // 检查坐标系是否匹配
  // if (msg->header.frame_id != global_frame_)
  // {
  //   ROS_WARN_THROTTLE(1.0, "External map frame (%s) does not match global frame (%s)", 
  //                    msg->header.frame_id.c_str(), global_frame_.c_str());
  //   return;
  // }
  
  // 检查地图参数是否合理
  if (msg->info.width == 0 || msg->info.height == 0 || msg->info.resolution <= 0.0)
  {
    ROS_WARN_THROTTLE(1.0, "Received invalid external map parameters");
    return;
  }
  
  external_map_ = *msg;
  new_data_available_ = true;
  has_updated_data_ = true;
  
  ROS_DEBUG("Received external map with size %dx%d, resolution: %f", 
           msg->info.width, msg->info.height, msg->info.resolution);
}

void ExternalMapLayer::updateBounds(double robot_x, double robot_y, double robot_yaw,
                               double* min_x, double* min_y, double* max_x, double* max_y)
{
  if (!enabled_ || !new_data_available_) 
    return;
    

  // 只有当外部地图有实质性变化时才更新边界
  double distance_moved = sqrt(pow(robot_x - last_robot_x_, 2) + 
  pow(robot_y - last_robot_y_, 2));
    
  // 只有移动超过一定距离或收到新数据时才更新
 if (distance_moved > update_min_distance_ || new_data_available_) 
  {
    processExternalMap();
    // 更新边界

      // 扩展边界，确保膨胀层有足够的工作空间
    double inflation_padding = 2.0;  // 根据您的膨胀半径调整

    double origin_x = external_map_.info.origin.position.x;
    double origin_y = external_map_.info.origin.position.y;
    double map_width = external_map_.info.width * external_map_.info.resolution;
    double map_height = external_map_.info.height * external_map_.info.resolution;
    
    *min_x = std::min(*min_x, origin_x - inflation_padding);
    *min_y = std::min(*min_y, origin_y - inflation_padding);
    *max_x = std::max(*max_x, origin_x + map_width + inflation_padding);
    *max_y = std::max(*max_y, origin_y + map_height + inflation_padding);
    
    new_data_available_ = false;
    last_robot_x_ = robot_x;
    last_robot_y_ = robot_y;
  }else{
    double inflation_padding = 0.5;
    *min_x = std::min(*min_x, robot_x - inflation_padding);
    *min_y = std::min(*min_y, robot_y - inflation_padding);
    *max_x = std::max(*max_x, robot_x + inflation_padding);
    *max_y = std::max(*max_y, robot_y + inflation_padding);
  }
}

void ExternalMapLayer::updateCosts(costmap_2d::Costmap2D& master_grid, int min_i, int min_j, 
                               int max_i, int max_j)
{
  if (!enabled_ || !has_updated_data_)
    return;
  
  // 使用互斥锁确保线程安全
  boost::unique_lock<costmap_2d::Costmap2D::mutex_t> lock(*(master_grid.getMutex()));
  has_updated_data_ = false;

  // 将外部地图数据合并到主代价地图
  unsigned int mx, my;
  double wx, wy;
  
  for (unsigned int i = 0; i < external_map_.info.width; ++i)
  {
    for (unsigned int j = 0; j < external_map_.info.height; ++j)
    {
      int index = j * external_map_.info.width + i;
      if (index < 0 || index >= (int)external_map_.data.size())
        continue;
        
      // 跳过未知区域
      if (external_map_.data[index] == -1)
        continue;
        
      // 计算世界坐标
      wx = external_map_.info.origin.position.x + (i + 0.5) * external_map_.info.resolution;
      wy = external_map_.info.origin.position.y + (j + 0.5) * external_map_.info.resolution;
      
      // 转换为地图坐标
      if (master_grid.worldToMap(wx, wy, mx, my))
      {
        // 检查坐标是否在更新范围内
        if (mx < (unsigned int)min_i || mx >= (unsigned int)max_i ||
            my < (unsigned int)min_j || my >= (unsigned int)max_j)
          continue;
        
        // 转换代价值
        unsigned char cost = NO_INFORMATION;
        int8_t data_value = external_map_.data[index];
        
        if (data_value >= 20)  // 障碍物
          cost = LETHAL_OBSTACLE;
        else if (data_value == 0)  // 自由空间
          cost = FREE_SPACE;
        else if (data_value > 0 && data_value < 20)  // 中间值
          cost = (unsigned char)(data_value * 2.55);
        
        // 使用最大值合并策略
        unsigned char old_cost = master_grid.getCost(mx, my);
        if (cost != NO_INFORMATION && cost > old_cost)
        {
          master_grid.setCost(mx, my, cost);
        }
      }
    }
  }

}

void ExternalMapLayer::processExternalMap()
{
  // 这里可以添加额外的处理逻辑
  ROS_DEBUG("Processing external map with size %dx%d, resolution: %f", 
           external_map_.info.width, external_map_.info.height, 
           external_map_.info.resolution);
}

}  // namespace costmap_2d