#pragma once

#include <string>
#include <unordered_map>

#include "multi_floor_navigation/topo_struct.h"

namespace YAML
{
class Node;
}

namespace multi_floor_navigation
{

// floor_id的低2位用于node_index：每层最多100个拓扑节点。
// 区域和楼层号分别预留100个槽位。
constexpr int kNodesPerFloor = 100;
constexpr int kFloorsPerArea = 100;
constexpr int kOutdoorFloorId = 100 * kFloorsPerArea * kNodesPerFloor;

class TopoGraph
{
public:
  static int floor_id_from_name(const std::string& floor_name);
  static int node_id(int floor_id, int node_index);

  // 加载成功后替换当前图；配置错误时当前图保持不变。
  void load_topology(const std::string& file_path);

  const std::unordered_map<int, std::string>& floor_map_paths() const;
  const std::unordered_map<int, Vertex>& vertices() const;
  const std::unordered_map<int, StairRoute>& stair_routes() const;

  const std::string& floor_map_path(int floor_id) const;
  const Vertex& vertex(int requested_node_id) const;
  const StairRoute& stair_route(int entry_node_id) const;

private:
  void load_floors(const YAML::Node& root, const std::string& topology_path);
  void connect_floor_nodes();

  std::unordered_map<int, std::string> floor_map_paths_;
  std::unordered_map<int, Vertex> vertices_;
  std::unordered_map<int, StairRoute> stair_routes_;
};

}  // namespace multi_floor_navigation
