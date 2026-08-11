#include "multi_floor_navigation/topo_map.h"

#include <yaml-cpp/yaml.h>

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

std::string resolve_path(const std::string& topology_path, const std::string& configured_path)
{
  if (configured_path.empty() || configured_path[0] == '/')
  {
    return configured_path;
  }
  return directory_name(topology_path) + "/" + configured_path;
}

Pose2D read_pose(const YAML::Node& node, const std::string& context)
{
  if (!node || !node.IsSequence() || node.size() != 3)
  {
    throw config_error(context, "expected [x, y, yaw]");
  }

  Pose2D pose;
  try
  {
    pose.x = node[0].as<double>();
    pose.y = node[1].as<double>();
    pose.yaw = node[2].as<double>();
  }
  catch (const YAML::Exception&)
  {
    throw config_error(context, "x, y and yaw must be numeric");
  }
  if (!std::isfinite(pose.x) || !std::isfinite(pose.y) || !std::isfinite(pose.yaw))
  {
    throw config_error(context, "x, y and yaw must be finite");
  }
  return pose;
}

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

std::vector<Pose2D> read_route(const YAML::Node& stair, const std::string& context)
{
  const YAML::Node route = stair["route"];
  if (!route || !route.IsSequence() || route.size() < 2)
  {
    throw config_error(context + ".route", "expected at least an entrance and an exit pose");
  }

  std::vector<Pose2D> primitives;
  primitives.reserve(route.size());
  for (std::size_t i = 0; i < route.size(); ++i)
  {
    primitives.push_back(read_pose(route[i], context + ".route[" + std::to_string(i) + "]"));
  }
  return primitives;
}

double distance(const Pose2D& from, const Pose2D& to)
{
  return std::hypot(to.x - from.x, to.y - from.y);
}

double route_cost(const std::vector<Pose2D>& route)
{
  double cost = 0.0;
  for (std::size_t i = 1; i < route.size(); ++i)
  {
    cost += distance(route[i - 1], route[i]);
  }
  return cost;
}

std::vector<Pose2D> reversed_route(const std::vector<Pose2D>& route)
{
  const double pi = std::acos(-1.0);
  std::vector<Pose2D> reversed;
  reversed.reserve(route.size());
  for (std::vector<Pose2D>::const_reverse_iterator it = route.rbegin(); it != route.rend(); ++it)
  {
    Pose2D pose = *it;
    pose.yaw = std::atan2(std::sin(pose.yaw + pi), std::cos(pose.yaw + pi));
    reversed.push_back(pose);
  }
  return reversed;
}

EdgeType stair_type(int from_floor, int to_floor)
{
  return to_floor > from_floor ? EdgeType::STAIR_UP : EdgeType::STAIR_DOWN;
}

void add_edge(std::unordered_map<int, Vertex>& vertices,
              int from,
              int to,
              EdgeType type,
              double cost)
{
  vertices.at(from).edges.push_back(TopoEdge{to, type, cost});
}

}  // namespace

int TopoGraph::floor_id_from_name(const std::string& floor_name)
{
  return parse_floor_id(floor_name);
}

int TopoGraph::node_id(int floor_id, int node_index)
{
  return checked_node_id(floor_id, node_index, "node_index");
}

const std::unordered_map<int, std::string>& TopoGraph::floor_map_paths() const
{
  return floor_map_paths_;
}

const std::unordered_map<int, Vertex>& TopoGraph::vertices() const
{
  return vertices_;
}

const std::unordered_map<int, StairRoute>& TopoGraph::stair_routes() const
{
  return stair_routes_;
}

const std::string& TopoGraph::floor_map_path(int floor_id) const
{
  return floor_map_paths_.at(floor_id);
}

const Vertex& TopoGraph::vertex(int requested_node_id) const
{
  return vertices_.at(requested_node_id);
}

const StairRoute& TopoGraph::stair_route(int entry_node_id) const
{
  return stair_routes_.at(entry_node_id);
}

void TopoGraph::load_floor_maps(const YAML::Node& root, const std::string& topology_path)
{
  const YAML::Node floors = root["floors"];
  if (!floors || !floors.IsMap() || floors.size() == 0)
  {
    throw config_error("floors", "expected a non-empty mapping");
  }

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
    if (!floor_map_paths_.emplace(floor_id, resolve_path(topology_path, map_path)).second)
    {
      throw config_error("floors." + floor_name, "floor id collides with another floor");
    }
  }
}

void TopoGraph::load_nodes_and_edges(const YAML::Node& root)
{
  const YAML::Node floors = root["floors"];
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
      if (floor_map_paths_.count(to_floor) == 0)
      {
        throw config_error(context + ".to", "target floor is not declared under floors");
      }

      const int index = read_node_index(stair, context);
      const int from_node = checked_node_id(from_floor, index, context + ".node_index");
      const int to_node = checked_node_id(to_floor, index, context + ".node_index");
      if (vertices_.count(from_node) != 0 || vertices_.count(to_node) != 0)
      {
        throw config_error(context + ".node_index", "node id is already used on one endpoint floor");
      }

      const std::vector<Pose2D> route = read_route(stair, context);
      const std::vector<Pose2D> reverse_route = reversed_route(route);
      vertices_.emplace(from_node, Vertex{TopoNode{from_floor, route.front()}, {}});
      vertices_.emplace(to_node, Vertex{TopoNode{to_floor, reverse_route.front()}, {}});

      const double cost = route_cost(route);
      add_edge(vertices_, from_node, to_node, stair_type(from_floor, to_floor), cost);
      add_edge(vertices_, to_node, from_node, stair_type(to_floor, from_floor), cost);

      stair_routes_.emplace(from_node, StairRoute{to_node, route});
      stair_routes_.emplace(to_node, StairRoute{from_node, reverse_route});
    }
  }
}

void TopoGraph::connect_floor_nodes()
{
  std::unordered_map<int, std::vector<int>> nodes_by_floor;
  for (const auto& item : vertices_)
  {
    nodes_by_floor[item.second.node.map_id].push_back(item.first);
  }

  for (const auto& floor : nodes_by_floor)
  {
    const std::vector<int>& ids = floor.second;
    for (std::size_t i = 0; i < ids.size(); ++i)
    {
      for (std::size_t j = i + 1; j < ids.size(); ++j)
      {
        const double cost = distance(vertices_.at(ids[i]).node.pose, vertices_.at(ids[j]).node.pose);
        add_edge(vertices_, ids[i], ids[j], EdgeType::FLAT_NAV, cost);
        add_edge(vertices_, ids[j], ids[i], EdgeType::FLAT_NAV, cost);
      }
    }
  }
}

void TopoGraph::load_topology(const std::string& file_path)
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

  TopoGraph loaded;
  loaded.load_floor_maps(root, file_path);
  loaded.load_nodes_and_edges(root);
  loaded.connect_floor_nodes();

  floor_map_paths_.swap(loaded.floor_map_paths_);
  vertices_.swap(loaded.vertices_);
  stair_routes_.swap(loaded.stair_routes_);
}

}  // namespace multi_floor_navigation
