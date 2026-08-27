#include <actionlib/server/simple_action_server.h>
#include <floor_msgs/GetTopologyPlan.h>
#include <floor_msgs/MultiFloorNavigationAction.h>
#include <ros/package.h>
#include <ros/ros.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

#include <boost/bind.hpp>

#include <cmath>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>

#include "multi_floor_navigation/map_switcher.h"
#include "multi_floor_navigation/go2w_motion_manager.h"
#include "multi_floor_navigation/route_executor.h"
#include "multi_floor_navigation/stair_action_client.h"
#include "multi_floor_navigation/topology_map.h"
#include "multi_floor_navigation/topology_planner.h"

namespace multi_floor_navigation
{

class MultiFloorNavigationNode
{
public:
  MultiFloorNavigationNode()
    : node_()
    , private_node_("~")
    , tf_listener_(tf_buffer_)
  {
    const std::string package_path = ros::package::getPath("multi_floor_navigation");
    std::string topology_file = package_path + "/config/multimap.yaml";
    map_frame_ = "map";
    base_frame_ = "base_link";
    tf_timeout_ = 0.2;
    std::string initial_floor_name = "B4";
    std::string plan_service_name = "get_topology_plan";
    std::string navigate_action_name = "navigate";
    std::string move_base_action = "/move_base";
    std::string stair_action = "/stair_navigation";
    std::string change_map_service = "/change_map";
    std::string clear_costmaps_service = "/move_base/clear_costmaps";
    double server_timeout = 5.0;
    double stair_server_timeout = 5.0;
    double segment_timeout = 0.0;
    double move_base_stop_timeout = 2.0;
    bool motion_manager_enabled = true;
    std::string network_interface = "eth0";

    private_node_.param("topology_file", topology_file, topology_file);
    private_node_.param("map_frame", map_frame_, map_frame_);
    private_node_.param("base_frame", base_frame_, base_frame_);
    private_node_.param("tf_timeout", tf_timeout_, tf_timeout_);
    private_node_.param("initial_floor", initial_floor_name, initial_floor_name);
    private_node_.param("plan_service", plan_service_name, plan_service_name);
    private_node_.param("navigate_action", navigate_action_name, navigate_action_name);
    private_node_.param("move_base_action", move_base_action, move_base_action);
    private_node_.param("stair_action", stair_action, stair_action);
    private_node_.param("change_map_service", change_map_service, change_map_service);
    private_node_.param("clear_costmaps_service", clear_costmaps_service, clear_costmaps_service);
    private_node_.param("server_timeout", server_timeout, server_timeout);
    private_node_.param("stair_server_timeout", stair_server_timeout, stair_server_timeout);
    private_node_.param("segment_timeout", segment_timeout, segment_timeout);
    private_node_.param("move_base_stop_timeout",
                        move_base_stop_timeout,
                        move_base_stop_timeout);
    private_node_.param("motion_manager_enabled",
                        motion_manager_enabled,
                        motion_manager_enabled);
    private_node_.param("network_interface", network_interface, network_interface);

    graph_.load_topology(topology_file);
    const int initial_floor = TopologyGraph::floor_id_from_name(initial_floor_name);

    if (tf_timeout_ < 0.0)
    {
      throw std::invalid_argument("tf_timeout must not be negative");
    }

    planner_.reset(new TopologyPlanner(graph_, map_frame_));
    map_switcher_.reset(
        new MapSwitcher(node_, graph_, change_map_service, clear_costmaps_service));
    map_switcher_->set_current_floor(initial_floor);
    stair_executor_.reset(new StairActionClient(stair_action, stair_server_timeout));
    if (motion_manager_enabled)
    {
      motion_manager_.reset(new Go2WMotionManager());
      if (!motion_manager_->init(network_interface) || !motion_manager_->setNormalMode())
      {
        throw std::runtime_error("failed to initialize Go2W in flat mode 0");
      }
    }
    executor_.reset(new RouteExecutor(graph_,
                                      *map_switcher_,
                                      move_base_action,
                                      server_timeout,
                                      segment_timeout,
                                      move_base_stop_timeout,
                                      stair_executor_.get(),
                                      motion_manager_.get()));

    plan_service_ = node_.advertiseService(plan_service_name,
                                           &MultiFloorNavigationNode::plan_callback,
                                           this);
    navigation_server_.reset(new NavigationServer(
        node_,
        navigate_action_name,
        boost::bind(&MultiFloorNavigationNode::execute_navigation, this, _1),
        false));
    navigation_server_->start();

    ROS_INFO_STREAM("multi_floor_navigation loaded " << graph_.node_count()
                                                       << " topology nodes from " << topology_file);
    ROS_INFO_STREAM("Stair execution is connected to action server " << stair_action);
    ROS_INFO_STREAM("Navigation start pose is read from TF " << map_frame_ << " -> "
                                                               << base_frame_);
    ROS_INFO_STREAM("Go2W motion mode switching is "
                    << (motion_manager_enabled ? "enabled" : "disabled"));
  }

private:
  using NavigationServer = actionlib::SimpleActionServer<floor_msgs::MultiFloorNavigationAction>;

  bool current_pose(geometry_msgs::PoseStamped& pose, std::string& message)
  {
    try
    {
      const geometry_msgs::TransformStamped transform = tf_buffer_.lookupTransform(
          map_frame_, base_frame_, ros::Time(0), ros::Duration(tf_timeout_));
      pose.header.stamp = transform.header.stamp;
      pose.header.frame_id = map_frame_;
      pose.pose.position.x = transform.transform.translation.x;
      pose.pose.position.y = transform.transform.translation.y;
      pose.pose.position.z = transform.transform.translation.z;
      pose.pose.orientation = transform.transform.rotation;

      const geometry_msgs::Point& position = pose.pose.position;
      const geometry_msgs::Quaternion& orientation = pose.pose.orientation;
      if (!std::isfinite(position.x) || !std::isfinite(position.y) ||
          !std::isfinite(position.z) || !std::isfinite(orientation.x) ||
          !std::isfinite(orientation.y) || !std::isfinite(orientation.z) ||
          !std::isfinite(orientation.w))
      {
        message = "TF contains a non-finite navigation start pose";
        return false;
      }
      return true;
    }
    catch (const tf2::TransformException& error)
    {
      message = "failed to lookup navigation start TF " + map_frame_ + " -> " +
                base_frame_ + ": " + error.what();
      return false;
    }
  }

  bool goal_in_map(const geometry_msgs::PoseStamped& input,
                   geometry_msgs::PoseStamped& output,
                   std::string& message)
  {
    try
    {
      if (input.header.frame_id.empty() || input.header.frame_id == map_frame_)
      {
        output = input;
        output.header.frame_id = map_frame_;
      }
      else
      {
        tf_buffer_.transform(input, output, map_frame_, ros::Duration(tf_timeout_));
      }
    }
    catch (const tf2::TransformException& error)
    {
      message = "failed to transform navigation goal from '" + input.header.frame_id +
                "' to '" + map_frame_ + "': " + error.what();
      return false;
    }

    geometry_msgs::Point& position = output.pose.position;
    geometry_msgs::Quaternion& orientation = output.pose.orientation;
    if (!std::isfinite(position.x) || !std::isfinite(position.y) ||
        !std::isfinite(position.z) || !std::isfinite(orientation.x) ||
        !std::isfinite(orientation.y) || !std::isfinite(orientation.z) ||
        !std::isfinite(orientation.w))
    {
      message = "navigation goal contains a non-finite pose";
      return false;
    }

    const double orientation_norm = std::sqrt(orientation.x * orientation.x +
                                              orientation.y * orientation.y +
                                              orientation.z * orientation.z +
                                              orientation.w * orientation.w);
    if (orientation_norm < 1e-9)
    {
      orientation.x = 0.0;
      orientation.y = 0.0;
      orientation.z = 0.0;
      orientation.w = 1.0;
    }
    else
    {
      orientation.x /= orientation_norm;
      orientation.y /= orientation_norm;
      orientation.z /= orientation_norm;
      orientation.w /= orientation_norm;
    }
    output.header.stamp = ros::Time::now();
    return true;
  }

  bool plan_callback(floor_msgs::GetTopologyPlan::Request& request,
                     floor_msgs::GetTopologyPlan::Response& response)
  {
    try
    {
      geometry_msgs::PoseStamped start;
      geometry_msgs::PoseStamped goal;
      if (!current_pose(start, response.message) ||
          !goal_in_map(request.goal, goal, response.message))
      {
        response.success = false;
        return true;
      }

      const int start_floor = map_switcher_->current_floor();
      const int goal_floor = request.goal_floor < 0 ? start_floor : request.goal_floor;
      response.success = planner_->plan(start,
                                        start_floor,
                                        goal,
                                        goal_floor,
                                        response.route,
                                        response.message);
    }
    catch (const std::exception& error)
    {
      response.success = false;
      response.message = error.what();
    }
    return true;
  }

  void execute_navigation(const floor_msgs::MultiFloorNavigationGoalConstPtr& goal)
  {
    floor_msgs::MultiFloorNavigationResult result;
    std::string message;
    try
    {
      const int start_floor = map_switcher_->current_floor();
      const int goal_floor = goal->goal_floor < 0 ? start_floor : goal->goal_floor;

      floor_msgs::MultiFloorNavigationFeedback feedback;
      feedback.state = floor_msgs::MultiFloorNavigationFeedback::PLANNING;
      feedback.current_floor = start_floor;
      feedback.segment_index = 0;
      navigation_server_->publishFeedback(feedback);

      geometry_msgs::PoseStamped start;
      geometry_msgs::PoseStamped target;
      if (!current_pose(start, message) || !goal_in_map(goal->goal, target, message))
      {
        result.success = false;
        result.message = message;
        navigation_server_->setAborted(result, message);
        return;
      }

      if (navigation_server_->isPreemptRequested())
      {
        result.success = false;
        result.message = "navigation canceled before planning";
        navigation_server_->setPreempted(result, result.message);
        return;
      }

      if (!planner_->plan(start,
                          start_floor,
                          target,
                          goal_floor,
                          result.route,
                          message))
      {
        result.success = false;
        result.message = message;
        navigation_server_->setAborted(result, message);
        return;
      }

      const bool success = executor_->execute(
          result.route,
          [this](std::uint8_t state, std::size_t segment_index, int current_floor) {
            floor_msgs::MultiFloorNavigationFeedback current_feedback;
            current_feedback.state = state;
            current_feedback.current_floor = current_floor;
            current_feedback.segment_index = static_cast<std::uint32_t>(segment_index);
            navigation_server_->publishFeedback(current_feedback);
          },
          [this]() { return navigation_server_->isPreemptRequested() || !ros::ok(); },
          message);

      result.success = success;
      result.message = message;
      if (success)
      {
        navigation_server_->setSucceeded(result, message);
      }
      else if (navigation_server_->isPreemptRequested())
      {
        executor_->cancel();
        navigation_server_->setPreempted(result, message);
      }
      else
      {
        navigation_server_->setAborted(result, message);
      }
    }
    catch (const std::exception& error)
    {
      executor_->cancel();
      result.success = false;
      result.message = error.what();
      navigation_server_->setAborted(result, result.message);
    }
  }

  ros::NodeHandle node_;
  ros::NodeHandle private_node_;
  tf2_ros::Buffer tf_buffer_;
  tf2_ros::TransformListener tf_listener_;
  std::string map_frame_;
  std::string base_frame_;
  double tf_timeout_;
  TopologyGraph graph_;
  std::unique_ptr<TopologyPlanner> planner_;
  std::unique_ptr<MapSwitcher> map_switcher_;
  std::unique_ptr<StairActionClient> stair_executor_;
  std::unique_ptr<Go2WMotionManager> motion_manager_;
  std::unique_ptr<RouteExecutor> executor_;
  ros::ServiceServer plan_service_;
  std::unique_ptr<NavigationServer> navigation_server_;
};

}  // namespace multi_floor_navigation

int main(int argc, char** argv)
{
  ros::init(argc, argv, "multi_floor_navigation");
  try
  {
    multi_floor_navigation::MultiFloorNavigationNode node;
    ros::spin();
  }
  catch (const std::exception& error)
  {
    ROS_FATAL_STREAM("Failed to start multi_floor_navigation: " << error.what());
    return 1;
  }
  return 0;
}
