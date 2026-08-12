#include "multi_floor_navigation/map_switcher.h"
#include <ros/ros.h>
#include <nav_msgs/LoadMap.h>
#include <std_srvs/Empty.h>
#include <exception>
#include <ros/package.h>

int main(int argc, char** argv)
{
  ros::init(argc, argv, "map_switcher_test");
  ros::NodeHandle node;

  multi_floor_navigation::TopologyGraph graph;
    const std::string package_path = ros::package::getPath("multi_floor_navigation");
    std::string topology_file = package_path + "/config/multimap.yaml";
  graph.load_topology(topology_file);

  multi_floor_navigation::MapSwitcher switcher(node,
                                               graph,
                                               "/change_map",
                                               "/move_base/clear_costmaps");

  std::string message;
  if (!switcher.switch_to(10400, message))
  {
    ROS_ERROR_STREAM("Failed to switch to floor B4: " << message);
    return 1;
  }
  ROS_INFO_STREAM("Switched to floor B4: " << message);

  if (!switcher.switch_to(10500, message))
  {
    ROS_ERROR_STREAM("Failed to switch to floor B5: " << message);
    return 1;
  }
  ROS_INFO_STREAM("Switched to floor B5: " << message);

  return 0;
}