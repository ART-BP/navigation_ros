#pragma once

#include <actionlib/client/simple_action_client.h>
#include <floor_msgs/NavigationRoute.h>
#include <move_base_msgs/MoveBaseAction.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

#include "multi_floor_navigation/map_switcher.h"
#include "multi_floor_navigation/topology_map.h"


namespace multi_floor_navigation
{

//根据获得的拓扑图和规划的路径，执行路径规划
class StairExecutor
{
public:
  virtual ~StairExecutor() = default;

  virtual bool execute(int entry_node_id,
                       int from_floor,
                       int to_floor,
                       const TopologyEdge& edge,
                       const std::function<bool()>& cancel_requested,
                       std::string& message) = 0;
  virtual void cancel() = 0;
};

class RouteExecutor
{
public:
  using FeedbackCallback = std::function<void(std::uint8_t, std::size_t, int)>;
  using CancelCheck = std::function<bool()>;

  RouteExecutor(const TopologyGraph& graph,
                MapSwitcher& map_switcher,
                std::string move_base_action,
                double server_timeout,
                double segment_timeout,
                StairExecutor* stair_executor = nullptr);
  ~RouteExecutor();

  void set_stair_executor(StairExecutor* stair_executor);
  bool execute(const floor_msgs::NavigationRoute& route,
               const FeedbackCallback& feedback,
               const CancelCheck& cancel_requested,
               std::string& message);
  void cancel();

private:
  using MoveBaseClient = actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction>;

  // 调用拓扑搜索
  bool execute_flat(const floor_msgs::RouteSegment& segment,
                    const CancelCheck& cancel_requested,
                    std::string& message);

  const TopologyGraph& graph_;
  MapSwitcher& map_switcher_;
  MoveBaseClient move_base_client_;
  double server_timeout_;
  double segment_timeout_;
  StairExecutor* stair_executor_;
};

}  // namespace multi_floor_navigation
