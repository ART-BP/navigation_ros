#include <road_yield/road_yield_manager.h>

#include <ros/ros.h>

#include <exception>
#include <string>

int main(int argc, char** argv)
{
  ros::init(argc, argv, "road_yield");
  ros::NodeHandle nh;
  ros::NodeHandle private_nh("~");

  std::string config_path;
  private_nh.param<std::string>("config", config_path, std::string());
  if (config_path.empty() && argc >= 2)
  {
    config_path = argv[1];
  }
  if (config_path.empty())
  {
    ROS_FATAL("Set ~config or pass a road-yield YAML path as the first argument");
    return 2;
  }

  try
  {
    road_yield::RoadYieldManager manager(nh, config_path);
    ros::AsyncSpinner spinner(3);
    spinner.start();
    ros::waitForShutdown();
    manager.shutdown();
  }
  catch (const std::exception& exception)
  {
    ROS_FATAL_STREAM("road_yield failed: " << exception.what());
    return 1;
  }
  return 0;
}
