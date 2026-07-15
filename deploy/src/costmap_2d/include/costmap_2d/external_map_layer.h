#ifndef EXTERNAL_MAP_LAYER_H_
#define EXTERNAL_MAP_LAYER_H_

#include <ros/ros.h>
#include <costmap_2d/costmap_layer.h>
#include <costmap_2d/layered_costmap.h>
#include <nav_msgs/OccupancyGrid.h>
#include <geometry_msgs/PoseStamped.h>
#include <costmap_2d/GenericPluginConfig.h>
#include <dynamic_reconfigure/server.h>

namespace costmap_2d
{
class ExternalMapLayer : public CostmapLayer
{
public:
  ExternalMapLayer(); // 构造函数
  virtual ~ExternalMapLayer();
  virtual void onInitialize(); // 初始化
  virtual void updateBounds(double robot_x, double robot_y, double robot_yaw, 
                           double* min_x, double* min_y, double* max_x, double* max_y); // 更新边界
  virtual void updateCosts(costmap_2d::Costmap2D& master_grid, int min_i, int min_j, int max_i, int max_j); // 更新代价

  virtual void activate(); // 激活图层
  virtual void deactivate(); // 停用图层
  virtual void reset(); // 重置图层
  void externalMapCallback(const nav_msgs::OccupancyGridConstPtr& message); // 外部地图回调函数

private:
  
  void reconfigureCB(costmap_2d::GenericPluginConfig &config, uint32_t level);
  void processExternalMap();

  ros::Subscriber external_map_sub_;
  nav_msgs::OccupancyGrid external_map_;
  bool new_data_available_;
  bool has_updated_data_;
  std::string global_frame_;
  dynamic_reconfigure::Server<costmap_2d::GenericPluginConfig> *dsrv_;

  double last_robot_x_, last_robot_y_;
  double update_min_distance_;

};

} // namespace costmap_2d
#endif