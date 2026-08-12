#include "multi_floor_navigation/topology_planner.h"

#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

namespace multi_floor_navigation
{
namespace
{

struct Parent
{
  int from;
  EdgeType edge_type;
  double edge_cost;
};

double distance(double from_x, double from_y, double to_x, double to_y)
{
  return std::hypot(to_x - from_x, to_y - from_y);
}

geometry_msgs::PoseStamped node_pose(const TopologyNode& node, const std::string& frame_id)
{
  geometry_msgs::PoseStamped pose;
  pose.header.stamp = ros::Time::now();
  pose.header.frame_id = frame_id;
  pose.pose.position.x = node.pose.x;
  pose.pose.position.y = node.pose.y;
  pose.pose.position.z = node.pose.z;
  tf2::Quaternion orientation;
  orientation.setRPY(0.0, 0.0, node.pose.yaw);
  pose.pose.orientation = tf2::toMsg(orientation);
  return pose;
}

floor_msgs::RouteSegment make_segment(std::uint8_t type,
                                      int from_floor,
                                      int to_floor,
                                      int from_node_id,
                                      int to_node_id,
                                      const geometry_msgs::PoseStamped& start,
                                      const geometry_msgs::PoseStamped& goal)
{
  floor_msgs::RouteSegment segment;
  segment.type = type;
  segment.from_floor = from_floor;
  segment.to_floor = to_floor;
  segment.from_node_id = from_node_id;
  segment.to_node_id = to_node_id;
  segment.start_pose = start;
  segment.goal_pose = goal;
  return segment;
}

std::uint8_t message_edge_type(EdgeType type)
{
  return type == EdgeType::FLAT_NAV ? floor_msgs::RouteSegment::FLAT
                                    : floor_msgs::RouteSegment::STAIR;
}

}  // namespace

TopologyPlanner::TopologyPlanner(const TopologyGraph& graph, std::string map_frame)
  : graph_(graph), map_frame_(std::move(map_frame))
{
}

bool TopologyPlanner::plan(const geometry_msgs::PoseStamped& start,
                           int start_floor,
                           const geometry_msgs::PoseStamped& goal,
                           int goal_floor,
                           floor_msgs::NavigationRoute& route,
                           std::string& message) const
{
  route = floor_msgs::NavigationRoute();
  route.header.stamp = ros::Time::now();
  route.header.frame_id = map_frame_;
  route.start_floor = start_floor;
  route.goal_floor = goal_floor;

  if (!graph_.has_floor(start_floor))
  {
    message = "start floor is not configured: " + std::to_string(start_floor);
    return false;
  }
  if (!graph_.has_floor(goal_floor))
  {
    message = "goal floor is not configured: " + std::to_string(goal_floor);
    return false;
  }

  // 同层规划返回-1 -》 -1的平层段，跨层规划返回-1 -》 node_id的平层段，node_id -》 node_id的楼梯段，node_id -》 -1的平层段。
  if (start_floor == goal_floor)
  {
    route.segments.push_back(make_segment(floor_msgs::RouteSegment::FLAT,
                                          start_floor,
                                          goal_floor,
                                          -1,
                                          -1,
                                          start,
                                          goal));
    route.total_cost = distance(start.pose.position.x,
                                start.pose.position.y,
                                goal.pose.position.x,
                                goal.pose.position.y);
    message = "same-floor route";
    return true;
  }

  using QueueItem = std::pair<double, int>;
  std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<QueueItem>> queue;
  std::unordered_map<int, double> costs;
  std::unordered_map<int, Parent> parents;

  // 初始化起点楼层的所有节点，计算从起点到这些节点的初始成本，并将它们加入优先队列
  for (const auto& item : graph_.floor(start_floor).nodes)
  {
    const double initial_cost = distance(start.pose.position.x,
                                         start.pose.position.y,
                                         item.second.pose.x,
                                         item.second.pose.y);
    costs[item.first] = initial_cost;
    parents[item.first] = Parent{-1, EdgeType::FLAT_NAV, initial_cost};
    queue.push(QueueItem(initial_cost, item.first));
  }
  if (queue.empty())
  {
    message = "start floor has no topology nodes";
    return false;
  }

  // Dijkstra算法搜索最短路径
  while (!queue.empty())
  {
    const double current_cost = queue.top().first;
    const int current_id = queue.top().second;
    queue.pop();
    if (current_cost > costs.at(current_id))
    {
      continue;
    }

    const TopologyNode& current_node = graph_.node(current_id);
    const int current_floor_id = TopologyGraph::floor_id_from_node_id(current_id);

    const auto relax = [&](int to_node_id, EdgeType edge_type, double edge_cost) {
      const double candidate = current_cost + edge_cost;
      const auto known = costs.find(to_node_id);
      if (known != costs.end() && candidate >= known->second)
      {
        return;
      }
      costs[to_node_id] = candidate;
      parents[to_node_id] = Parent{current_id, edge_type, edge_cost};
      queue.push(QueueItem(candidate, to_node_id));
    };

    // 同层节点天然全部连通，不在图中永久保存O(N^2)条平层边。
    for (const auto& item : graph_.floor(current_floor_id).nodes)
    {
      if (item.first == current_id)
      {
        continue;
      }
      relax(item.first,
            EdgeType::FLAT_NAV,
            distance(current_node.pose.x,
                     current_node.pose.y,
                     item.second.pose.x,
                     item.second.pose.y));
    }

    for (const TopologyEdge& edge : current_node.edges)
    {
      relax(edge.to_node_id, edge.type, edge.cost);
    }
  }

  int final_node = -1;
  double final_cost = std::numeric_limits<double>::infinity();
  for (const auto& item : graph_.floor(goal_floor).nodes)
  {
    const auto reached = costs.find(item.first);
    if (reached == costs.end())
    {
      continue;
    }
    const double candidate = reached->second + distance(item.second.pose.x,
                                                         item.second.pose.y,
                                                         goal.pose.position.x,
                                                         goal.pose.position.y);
    if (candidate < final_cost)
    {
      final_cost = candidate;
      final_node = item.first;
    }
  }
  if (final_node < 0)
  {
    message = "no topology path connects the requested floors";
    return false;
  }

  std::vector<int> path;
  for (int current = final_node; current >= 0; current = parents.at(current).from)
  {
    path.push_back(current);
  }
  std::reverse(path.begin(), path.end());

  const int first_node_id = path.front();
  const TopologyNode& first_node = graph_.node(first_node_id);
  route.segments.push_back(make_segment(floor_msgs::RouteSegment::FLAT,
                                        start_floor,
                                        start_floor,
                                        -1,
                                        first_node_id,
                                        start,
                                        node_pose(first_node, map_frame_)));

  for (std::size_t i = 1; i < path.size(); ++i)
  {
    const int from_id = path[i - 1];
    const int to_id = path[i];
    const TopologyNode& from_node = graph_.node(from_id);
    const TopologyNode& to_node = graph_.node(to_id);
    const Parent& parent = parents.at(to_id);
    route.segments.push_back(make_segment(message_edge_type(parent.edge_type),
                                          TopologyGraph::floor_id_from_node_id(from_id),
                                          TopologyGraph::floor_id_from_node_id(to_id),
                                          from_id,
                                          to_id,
                                          node_pose(from_node, map_frame_),
                                          node_pose(to_node, map_frame_)));
  }

  const TopologyNode& last_node = graph_.node(final_node);
  route.segments.push_back(make_segment(floor_msgs::RouteSegment::FLAT,
                                        goal_floor,
                                        goal_floor,
                                        final_node,
                                        -1,
                                        node_pose(last_node, map_frame_),
                                        goal));
  route.total_cost = final_cost;
  message = "topology route found";
  return true;
}

}  // namespace multi_floor_navigation
