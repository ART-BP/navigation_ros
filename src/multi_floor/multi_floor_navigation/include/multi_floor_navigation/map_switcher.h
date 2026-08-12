#pragma once

#include <ros/ros.h>

#include <string>

#include "multi_floor_navigation/topology_map.h"

namespace multi_floor_navigation
{

class MapSwitcher
{
public:
  MapSwitcher(ros::NodeHandle& node,
              const TopologyGraph& graph,
              std::string change_map_service,
              std::string clear_costmaps_service);

  void set_current_floor(int floor_id);
  int current_floor() const;
  bool switch_to(int floor_id, std::string& message);

private:
  const TopologyGraph& graph_;
  ros::ServiceClient change_map_client_;
  ros::ServiceClient clear_costmaps_client_;
  int current_floor_;
};

}  // namespace multi_floor_navigation
