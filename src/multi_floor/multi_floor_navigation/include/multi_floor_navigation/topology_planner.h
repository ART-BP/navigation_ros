#pragma once

#include <geometry_msgs/PoseStamped.h>
#include <floor_msgs/NavigationRoute.h>

#include <string>

#include "multi_floor_navigation/topo_map.h"

namespace multi_floor_navigation
{

class TopologyPlanner
{
public:
  TopologyPlanner(const TopoGraph& graph, std::string map_frame);

  bool plan(const geometry_msgs::PoseStamped& start,
            int start_floor,
            const geometry_msgs::PoseStamped& goal,
            int goal_floor,
            floor_msgs::NavigationRoute& route,
            std::string& message) const;

private:
  const TopoGraph& graph_;
  std::string map_frame_;
};

}  // namespace multi_floor_navigation
