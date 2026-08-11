#include <actionlib/server/simple_action_server.h>
#include <floor_msgs/GetTopologyPlan.h>
#include <floor_msgs/MultiFloorNavigationAction.h>
#include <ros/package.h>
#include <ros/ros.h>

#include <boost/bind.hpp>

#include <exception>
#include <memory>
#include <string>

#include "multi_floor_navigation/map_switcher.h"
#include "multi_floor_navigation/route_executor.h"
#include "multi_floor_navigation/stair_action_client.h"
#include "multi_floor_navigation/topo_map.h"
#include "multi_floor_navigation/topology_planner.h"

namespace multi_floor_navigation
{

class MultiFloorNavigationNode
{
public:
  MultiFloorNavigationNode()
    : node_()
    , private_node_("~")
  {
    const std::string package_path = ros::package::getPath("multi_floor_navigation");
    std::string topology_file = package_path + "/config/multimap.yaml";
    std::string map_frame = "map";
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

    private_node_.param("topology_file", topology_file, topology_file);
    private_node_.param("map_frame", map_frame, map_frame);
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

    graph_.load_topology(topology_file);
    const int initial_floor = TopoGraph::floor_id_from_name(initial_floor_name);

    planner_.reset(new TopologyPlanner(graph_, map_frame));
    map_switcher_.reset(
        new MapSwitcher(node_, graph_, change_map_service, clear_costmaps_service));
    map_switcher_->set_current_floor(initial_floor);
    stair_executor_.reset(new StairActionClient(stair_action, stair_server_timeout));
    executor_.reset(new RouteExecutor(graph_,
                                      *map_switcher_,
                                      move_base_action,
                                      server_timeout,
                                      segment_timeout,
                                      stair_executor_.get()));

    plan_service_ = node_.advertiseService(plan_service_name,
                                           &MultiFloorNavigationNode::plan_callback,
                                           this);
    navigation_server_.reset(new NavigationServer(
        node_,
        navigate_action_name,
        boost::bind(&MultiFloorNavigationNode::execute_navigation, this, _1),
        false));
    navigation_server_->start();

    ROS_INFO_STREAM("multi_floor_navigation loaded " << graph_.vertices().size()
                                                       << " topology nodes from " << topology_file);
    ROS_INFO_STREAM("Stair execution is connected to action server " << stair_action);
  }

private:
  using NavigationServer = actionlib::SimpleActionServer<floor_msgs::MultiFloorNavigationAction>;

  bool plan_callback(floor_msgs::GetTopologyPlan::Request& request,
                     floor_msgs::GetTopologyPlan::Response& response)
  {
    try
    {
      response.success = planner_->plan(request.start,
                                        request.start_floor,
                                        request.goal,
                                        request.goal_floor,
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
    floor_msgs::MultiFloorNavigationFeedback feedback;
    feedback.state = floor_msgs::MultiFloorNavigationFeedback::PLANNING;
    feedback.current_floor = goal->start_floor;
    feedback.segment_index = 0;
    navigation_server_->publishFeedback(feedback);

    std::string message;
    try
    {
      if (!planner_->plan(goal->start,
                          goal->start_floor,
                          goal->goal,
                          goal->goal_floor,
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
  TopoGraph graph_;
  std::unique_ptr<TopologyPlanner> planner_;
  std::unique_ptr<MapSwitcher> map_switcher_;
  std::unique_ptr<StairActionClient> stair_executor_;
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
