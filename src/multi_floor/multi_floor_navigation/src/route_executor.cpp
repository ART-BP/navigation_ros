#include "multi_floor_navigation/route_executor.h"

#include <floor_msgs/MultiFloorNavigationFeedback.h>

#include <exception>
#include <utility>

namespace multi_floor_navigation
{

RouteExecutor::RouteExecutor(const TopologyGraph& graph,
                             MapSwitcher& map_switcher,
                             std::string move_base_action,
                             double server_timeout,
                             double segment_timeout,
                             StairExecutor* stair_executor)
  : graph_(graph)
  , map_switcher_(map_switcher)
  , move_base_client_(std::move(move_base_action), true)
  , server_timeout_(server_timeout)
  , segment_timeout_(segment_timeout)
  , stair_executor_(stair_executor)
{
}

RouteExecutor::~RouteExecutor()
{
  cancel();
}

void RouteExecutor::set_stair_executor(StairExecutor* stair_executor)
{
  stair_executor_ = stair_executor;
}

bool RouteExecutor::execute_flat(const floor_msgs::RouteSegment& segment,
                                 const CancelCheck& cancel_requested,
                                 std::string& message)
{
  if (!move_base_client_.isServerConnected() &&
      !move_base_client_.waitForServer(ros::Duration(server_timeout_)))
  {
    message = "move_base action server is unavailable";
    return false;
  }

  move_base_msgs::MoveBaseGoal goal;
  goal.target_pose = segment.goal_pose;
  goal.target_pose.header.stamp = ros::Time::now();
  move_base_client_.sendGoal(goal);

  const ros::WallTime started = ros::WallTime::now();
  while (ros::ok())
  {
    if (cancel_requested && cancel_requested())
    {
      move_base_client_.cancelGoal();
      message = "route execution canceled";
      return false;
    }
    if (move_base_client_.waitForResult(ros::Duration(0.1)))
    {
      const actionlib::SimpleClientGoalState state = move_base_client_.getState();
      if (state == actionlib::SimpleClientGoalState::SUCCEEDED)
      {
        message = "flat segment completed";
        return true;
      }
      message = "move_base failed: " + state.toString();
      return false;
    }
    if (segment_timeout_ > 0.0 && (ros::WallTime::now() - started).toSec() >= segment_timeout_)
    {
      move_base_client_.cancelGoal();
      message = "move_base segment timeout";
      return false;
    }
  }

  move_base_client_.cancelGoal();
  message = "ROS shutdown during route execution";
  return false;
}

bool RouteExecutor::execute(const floor_msgs::NavigationRoute& route,
                            const FeedbackCallback& feedback,
                            const CancelCheck& cancel_requested,
                            std::string& message)
{
  if (route.segments.empty())
  {
    message = "route contains no segments";
    return false;
  }

  if (map_switcher_.current_floor() != route.start_floor)
  {
    if (feedback)
    {
      feedback(floor_msgs::MultiFloorNavigationFeedback::SWITCHING_MAP,
               0,
               map_switcher_.current_floor());
    }
    if (!map_switcher_.switch_to(route.start_floor, message))
    {
      return false;
    }
  }

  for (std::size_t i = 0; i < route.segments.size(); ++i)
  {
    if (cancel_requested && cancel_requested())
    {
      cancel();
      message = "route execution canceled";
      return false;
    }

    const floor_msgs::RouteSegment& segment = route.segments[i];
    if (segment.type == floor_msgs::RouteSegment::FLAT)
    {
      if (segment.from_floor != map_switcher_.current_floor() ||
          segment.to_floor != segment.from_floor)
      {
        message = "flat segment floor does not match the active map";
        return false;
      }
      if (feedback)
      {
        feedback(floor_msgs::MultiFloorNavigationFeedback::FLAT_NAV,
                 i,
                 map_switcher_.current_floor());
      }
      if (!execute_flat(segment, cancel_requested, message))
      {
        return false;
      }
      continue;
    }

    if (segment.type != floor_msgs::RouteSegment::STAIR)
    {
      message = "route contains an unknown segment type";
      return false;
    }
    if (segment.from_floor != map_switcher_.current_floor())
    {
      message = "stair segment starts on a floor different from the active map";
      return false;
    }
    if (stair_executor_ == nullptr)
    {
      message = "stair controller is not configured";
      return false;
    }

    const TopologyEdge* stair_edge = nullptr;
    try
    {
      stair_edge = &graph_.edge(segment.from_node_id, segment.to_node_id);
    }
    catch (const std::out_of_range&)
    {
      message = "no stair edge from node " + std::to_string(segment.from_node_id) +
                " to node " + std::to_string(segment.to_node_id);
      return false;
    }
    if (stair_edge->type == EdgeType::FLAT_NAV || stair_edge->primitives.size() < 2)
    {
      message = "planned stair edge has no valid stair path";
      return false;
    }

    if (feedback)
    {
      feedback(floor_msgs::MultiFloorNavigationFeedback::STAIR,
               i,
               map_switcher_.current_floor());
    }
    move_base_client_.cancelAllGoals();
    if (!stair_executor_->execute(segment.from_node_id,
                                  segment.from_floor,
                                  segment.to_floor,
                                  *stair_edge,
                                  cancel_requested,
                                  message))
    {
      return false;
    }

    if (feedback)
    {
      feedback(floor_msgs::MultiFloorNavigationFeedback::SWITCHING_MAP,
               i,
               segment.to_floor);
    }
    if (!map_switcher_.switch_to(segment.to_floor, message))
    {
      return false;
    }
  }

  message = "route execution completed";
  return true;
}

void RouteExecutor::cancel()
{
  move_base_client_.cancelAllGoals();
  if (stair_executor_ != nullptr)
  {
    stair_executor_->cancel();
  }
}

}  // namespace multi_floor_navigation
