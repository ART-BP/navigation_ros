#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace multi_floor_navigation
{

// z当前只保存和传递，不参与距离、方向或控制计算。
struct Pose3D
{
  double x;
  double y;
  double z;
  double yaw;
};

enum class EdgeType
{
  FLAT_NAV,
  STAIR_UP,
  STAIR_DOWN
};

struct TopologyEdge
{
  int to_node_id;
  EdgeType type;
  double cost;
  // 只用于楼梯边；反向边保存倒序且航向反转后的路径。
  std::vector<Pose3D> primitives;
};

struct TopologyNode
{
  Pose3D pose;
  // 同层连通由规划器动态展开，这里只保存跨楼层的楼梯边。
  std::vector<TopologyEdge> edges;
};

struct Floor
{
  int floor_id;
  std::string map_path;

  // key就是全局node_id，不在TopologyNode中重复保存。
  std::unordered_map<int, TopologyNode> nodes;
};
}  // namespace multi_floor_navigation
