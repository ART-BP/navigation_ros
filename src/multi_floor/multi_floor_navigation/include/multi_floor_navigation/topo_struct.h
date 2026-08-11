#pragma once

#include <vector>

namespace multi_floor_navigation
{

struct Pose2D
{
  double x;
  double y;
  double yaw;
};

struct StairRoute
{
  int to_node_id;
  std::vector<Pose2D> primitives;
};

struct TopoNode
{
  int map_id;
  Pose2D pose;
};

enum class EdgeType
{
  FLAT_NAV,
  STAIR_UP,
  STAIR_DOWN
};

struct TopoEdge
{
  int to;
  EdgeType type;
  double cost;
};

struct Vertex
{
  TopoNode node;
  std::vector<TopoEdge> edges;
};

}  // namespace multi_floor_navigation
