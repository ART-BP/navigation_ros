#pragma once

#include <actionlib/client/simple_action_client.h>
#include <floor_msgs/StairNavigationAction.h>

#include <string>

#include "multi_floor_navigation/route_executor.h"

namespace multi_floor_navigation
{

class StairActionClient : public StairExecutor
{
public:
  StairActionClient(std::string action_name, double server_timeout);

  bool execute(int entry_node_id,
               int from_floor,
               int to_floor,
               const TopologyEdge& edge,
               const std::function<bool()>& cancel_requested,
               std::string& message) override;
  void cancel() override;

private:
  using Client = actionlib::SimpleActionClient<floor_msgs::StairNavigationAction>;

  Client client_;
  double server_timeout_;
};

}  // namespace multi_floor_navigation
