#include "multi_floor_navigation/topology_map.h"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace multi_floor_navigation
{
namespace
{

std::runtime_error config_error(const std::string& context, const std::string& message)
{
  return std::runtime_error("invalid topology configuration at " + context + ": " + message);
}

// 将楼层名称转换为唯一的floor_id，B4 -> 10400
int parse_floor_id(const std::string& name)
{
  if (name == "outdoor")
  {
    return kOutdoorFloorId;
  }
  if (name.size() < 2)
  {
    throw config_error("floors." + name, "expected a name such as B4");
  }

  const char area = static_cast<char>(std::tolower(static_cast<unsigned char>(name[0])));
  if (area < 'a' || area > 'z')
  {
    throw config_error("floors." + name, "the first character must be A-Z");
  }

  std::size_t parsed = 0;
  int floor_number = 0;
  try
  {
    floor_number = std::stoi(name.substr(1), &parsed);
  }
  catch (const std::exception&)
  {
    throw config_error("floors." + name, "floor number must be an integer");
  }
  if (parsed != name.size() - 1 || floor_number < 0 || floor_number >= kFloorsPerArea)
  {
    throw config_error("floors." + name,
                       "floor number must be in [0, " + std::to_string(kFloorsPerArea - 1) + "]");
  }

  const int area_index = area - 'a';
  return (area_index * kFloorsPerArea + floor_number) * kNodesPerFloor;
}

// 检查node_index是否在有效范围内，并生成唯一的node_id
int checked_node_id(int floor_id, int node_index, const std::string& context)
{
  if (node_index < 0 || node_index >= kNodesPerFloor)
  {
    throw config_error(context,
                       "node_index must be in [0, " + std::to_string(kNodesPerFloor - 1) + "]");
  }
  return floor_id + node_index;
}

std::string directory_name(const std::string& path)
{
  const std::size_t separator = path.find_last_of("/\\");
  if (separator == std::string::npos)
  {
    return ".";
  }
  return separator == 0 ? path.substr(0, 1) : path.substr(0, separator);
}

// 获取.yaml文件所在目录的上一级目录
std::string ddirectory_name(const std::string& yaml_path)
{
  return directory_name(directory_name(yaml_path));
}

// 获得配置文件中指定的地图路径，如果是相对路径，则将其解析为绝对路径
std::string resolve_path(const std::string& topology_path, const std::string& configured_path)
{
  if (configured_path.empty() || configured_path[0] == '/')
  {
    return configured_path;
  }
  return ddirectory_name(topology_path) + "/maps/" + configured_path;
}

// 楼梯配置只保存位置，行驶方向由相邻两点的连线计算。
Pose3D read_route_point(const YAML::Node& node, const std::string& context)
{
  if (!node || !node.IsSequence() || node.size() != 3)
  {
    throw config_error(context, "expected [x, y, z]");
  }

  Pose3D pose;
  try
  {
    pose.x = node[0].as<double>();
    pose.y = node[1].as<double>();
    pose.z = node[2].as<double>();
    pose.yaw = 0.0;
  }
  catch (const YAML::Exception&)
  {
    throw config_error(context, "x, y and z must be numeric");
  }
  if (!std::isfinite(pose.x) || !std::isfinite(pose.y) || !std::isfinite(pose.z))
  {
    throw config_error(context, "x, y and z must be finite");
  }
  return pose;
}

void assign_route_headings(std::vector<Pose3D>& route, const std::string& context)
{
  for (std::size_t i = 0; i + 1 < route.size(); ++i)
  {
    const double dx = route[i + 1].x - route[i].x;
    const double dy = route[i + 1].y - route[i].y;
    if (std::hypot(dx, dy) <= 1e-9)
    {
      throw config_error(context + "[" + std::to_string(i + 1) + "]",
                         "consecutive route points must be different");
    }
    route[i].yaw = std::atan2(dy, dx);
  }
  // 最后一个点只作为位置终点，不需要有效yaw。
  route.back().yaw = 0.0;
}

// 读取楼梯节点的索引
int read_node_index(const YAML::Node& stair, const std::string& context)
{
  const YAML::Node value = stair["node_index"];
  if (!value || !value.IsScalar())
  {
    throw config_error(context + ".node_index", "expected an integer");
  }
  try
  {
    return value.as<int>();
  }
  catch (const YAML::Exception&)
  {
    throw config_error(context + ".node_index", "expected an integer");
  }
}

// 读取楼梯节点的路径
std::vector<Pose3D> read_route(const YAML::Node& stair, const std::string& context)
{
  const YAML::Node route = stair["route"];
  if (!route || !route.IsSequence() || route.size() < 2)
  {
    throw config_error(context + ".route", "expected at least an entrance and an exit pose");
  }

  std::vector<Pose3D> primitives;
  primitives.reserve(route.size());
  for (std::size_t i = 0; i < route.size(); ++i)
  {
    primitives.push_back(
        read_route_point(route[i], context + ".route[" + std::to_string(i) + "]"));
  }
  assign_route_headings(primitives, context + ".route");
  return primitives;
}

// 计算两点之间的欧几里得距离
double distance(const Pose3D& from, const Pose3D& to)
{
  // 当前拓扑代价仍为平面距离，z只存储，不参与计算。
  return std::hypot(to.x - from.x, to.y - from.y);
}

// 计算路径的总成本
double route_cost(const std::vector<Pose3D>& route)
{
  double cost = 0.0;
  for (std::size_t i = 1; i < route.size(); ++i)
  {
    cost += distance(route[i - 1], route[i]);
  }
  return cost;
}

// 获取反转后的路径
std::vector<Pose3D> reversed_route(const std::vector<Pose3D>& route)
{
  std::vector<Pose3D> reversed(route.rbegin(), route.rend());
  assign_route_headings(reversed, "generated reverse route");
  return reversed;
}

// 获取楼梯的类型
EdgeType stair_type(int from_floor, int to_floor)
{
  return to_floor > from_floor ? EdgeType::STAIR_UP : EdgeType::STAIR_DOWN;
}

}  // namespace

int TopologyGraph::floor_id_from_name(const std::string& floor_name)
{
  return parse_floor_id(floor_name);
}

int TopologyGraph::floor_id_from_node_id(int requested_node_id)
{
  if (requested_node_id < 0)
  {
    throw std::out_of_range("node id must be non-negative");
  }
  return (requested_node_id / kNodesPerFloor) * kNodesPerFloor;
}

int TopologyGraph::node_id(int floor_id, int node_index)
{
  return checked_node_id(floor_id, node_index, "node_index");
}

const std::unordered_map<int, Floor>& TopologyGraph::floors() const
{
  return floors_;
}

bool TopologyGraph::has_floor(int floor_id) const
{
  return floors_.count(floor_id) != 0;
}

std::size_t TopologyGraph::node_count() const
{
  std::size_t count = 0;
  for (const auto& item : floors_)
  {
    count += item.second.nodes.size();
  }
  return count;
}

const Floor& TopologyGraph::floor(int floor_id) const
{
  return floors_.at(floor_id);
}

const std::string& TopologyGraph::floor_map_path(int floor_id) const
{
  return floor(floor_id).map_path;
}

const TopologyNode& TopologyGraph::node(int requested_node_id) const
{
  return floor(floor_id_from_node_id(requested_node_id)).nodes.at(requested_node_id);
}

const TopologyEdge& TopologyGraph::edge(int from_node_id, int to_node_id) const
{
  const std::vector<TopologyEdge>& edges = node(from_node_id).edges;
  const auto found = std::find_if(edges.begin(), edges.end(), [to_node_id](const TopologyEdge& edge) {
    return edge.to_node_id == to_node_id;
  });
  if (found == edges.end())
  {
    throw std::out_of_range("no topology edge from node " + std::to_string(from_node_id) +
                            " to node " + std::to_string(to_node_id));
  }
  return *found;
}
// 生成楼层节点的唯一ID，保存楼层地图路径，楼层节点和楼梯节点的路径信息
void TopologyGraph::load_floors(const YAML::Node& root, const std::string& topology_path)
{
  const YAML::Node floors = root["floors"];
  if (!floors || !floors.IsMap() || floors.size() == 0)
  {
    throw config_error("floors", "expected a non-empty mapping");
  }

  // 读取楼层信息，保存楼层地图路径
  for (YAML::const_iterator it = floors.begin(); it != floors.end(); ++it)
  {
    if (!it->first.IsScalar() || !it->second.IsMap())
    {
      throw config_error("floors", "expected floor-name mappings");
    }

    const std::string floor_name = it->first.as<std::string>();
    const int floor_id = parse_floor_id(floor_name);
    const YAML::Node map = it->second["map"];
    if (!map || !map.IsScalar())
    {
      throw config_error("floors." + floor_name + ".map", "expected a map yaml path");
    }

    const std::string map_path = map.as<std::string>();
    if (map_path.empty())
    {
      throw config_error("floors." + floor_name + ".map", "path must not be empty");
    }
    if (!floors_.emplace(
            floor_id,
            Floor{floor_id, resolve_path(topology_path, map_path), {}}).second)
    {
      throw config_error("floors." + floor_name, "floor id collides with another floor");
    }
  }

  // 读取楼梯信息，生成楼梯节点的唯一ID，保存楼梯节点的路径信息
  for (YAML::const_iterator floor_it = floors.begin(); floor_it != floors.end(); ++floor_it)
  {
    const std::string floor_name = floor_it->first.as<std::string>();
    const int from_floor = parse_floor_id(floor_name);
    const YAML::Node stairs = floor_it->second["stairs"];
    if (!stairs)
    {
      continue;
    }
    if (!stairs.IsSequence())
    {
      throw config_error("floors." + floor_name + ".stairs", "expected a sequence");
    }

    for (std::size_t i = 0; i < stairs.size(); ++i)
    {
      const std::string context =
          "floors." + floor_name + ".stairs[" + std::to_string(i) + "]";
      const YAML::Node stair = stairs[i];
      if (!stair.IsMap())
      {
        throw config_error(context, "expected a mapping");
      }

      const YAML::Node to = stair["to"];
      if (!to || !to.IsScalar())
      {
        throw config_error(context + ".to", "expected a target floor name");
      }
      const std::string to_floor_name = to.as<std::string>();
      const int to_floor = parse_floor_id(to_floor_name);
      if (to_floor == from_floor)
      {
        throw config_error(context + ".to", "a stair must connect different floors");
      }
      if (!has_floor(to_floor))
      {
        throw config_error(context + ".to", "target floor is not declared under floors");
      }

      const int index = read_node_index(stair, context);
      const int from_node = checked_node_id(from_floor, index, context + ".node_index");
      const int reverse_index = kNodesPerFloor - 1 - index;
      const int to_node =
          checked_node_id(to_floor, reverse_index, context + ".node_index");
      Floor& source_floor = floors_.at(from_floor);
      Floor& target_floor = floors_.at(to_floor);
      if (source_floor.nodes.count(from_node) != 0 || target_floor.nodes.count(to_node) != 0)
      {
        throw config_error(context + ".node_index", "node id is already used on one endpoint floor");
      }

      const std::vector<Pose3D> route = read_route(stair, context);
      const std::vector<Pose3D> reverse_route = reversed_route(route);
      source_floor.nodes.emplace(from_node, TopologyNode{route.front(), {}});
      target_floor.nodes.emplace(to_node, TopologyNode{reverse_route.front(), {}});

      const double cost = route_cost(route);
      source_floor.nodes.at(from_node).edges.push_back(
          TopologyEdge{to_node, stair_type(from_floor, to_floor), cost, route});
      target_floor.nodes.at(to_node).edges.push_back(
          TopologyEdge{from_node, stair_type(to_floor, from_floor), cost, reverse_route});
    }
  }
}

void TopologyGraph::load_topology(const std::string& file_path)
{
  YAML::Node root;
  try
  {
    root = YAML::LoadFile(file_path);
  }
  catch (const YAML::Exception& error)
  {
    throw std::runtime_error("failed to load topology file '" + file_path + "': " + error.what());
  }
  if (!root.IsMap())
  {
    throw config_error("root", "expected a mapping");
  }

  TopologyGraph loaded;
  loaded.load_floors(root, file_path);
  floors_.swap(loaded.floors_);
}

}  // namespace multi_floor_navigation
