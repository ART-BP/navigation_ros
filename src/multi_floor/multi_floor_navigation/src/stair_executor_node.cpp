#include <ros/ros.h>

#include <exception>

#include "multi_floor_navigation/stair_controller.h"

int main(int argc, char** argv)
{
  ros::init(argc, argv, "stair_executor");
  ros::NodeHandle node;
  ros::NodeHandle private_node("~");
  try
  {
    multi_floor_navigation::StairController controller(node, private_node);
    ros::spin();
  }
  catch (const std::exception& error)
  {
    ROS_FATAL_STREAM("Failed to start stair executor: " << error.what());
    return 1;
  }
  return 0;
}
