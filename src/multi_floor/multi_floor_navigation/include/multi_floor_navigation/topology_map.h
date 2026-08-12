#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>

#include "multi_floor_navigation/topology_struct.h"

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

class TopologyGraph
{
public:
  static int floor_id_from_name(const std::string& floor_name);
  static int floor_id_from_node_id(int node_id);
  static int node_id(int floor_id, int node_index);

  // 加载成功后替换当前图；配置错误时当前图保持不变。
  void load_topology(const std::string& file_path);

  const std::unordered_map<int, Floor>& floors() const;
  bool has_floor(int floor_id) const;
  std::size_t node_count() const;
  const Floor& floor(int floor_id) const;
  const std::string& floor_map_path(int floor_id) const;
  const TopologyNode& node(int requested_node_id) const;
  const TopologyEdge& edge(int from_node_id, int to_node_id) const;

private:
  void load_floors(const YAML::Node& root, const std::string& topology_path);

  std::unordered_map<int, Floor> floors_;
};

}  // namespace multi_floor_navigation
