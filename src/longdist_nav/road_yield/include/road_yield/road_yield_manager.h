#ifndef ROAD_YIELD_ROAD_YIELD_MANAGER_H_
#define ROAD_YIELD_ROAD_YIELD_MANAGER_H_

#include <ros/node_handle.h>

#include <memory>
#include <string>

namespace road_yield
{

class RoadYieldManager
{
public:
  RoadYieldManager(ros::NodeHandle nh, const std::string& config_path);
  ~RoadYieldManager();

  void shutdown();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace road_yield

#endif  // ROAD_YIELD_ROAD_YIELD_MANAGER_H_
