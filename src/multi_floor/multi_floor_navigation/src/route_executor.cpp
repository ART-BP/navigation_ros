#include "multi_floor_navigation/route_executor.h"

#include <floor_msgs/MultiFloorNavigationFeedback.h>

#include <exception>
#include <stdexcept>
#include <utility>

#include "multi_floor_navigation/go2w_motion_manager.h"

namespace multi_floor_navigation
{

RouteExecutor::RouteExecutor(const TopologyGraph& graph,
                             MapSwitcher& map_switcher,
                             std::string move_base_action,
                             double server_timeout,
                             double segment_timeout,
                             double move_base_stop_timeout,
                             StairExecutor* stair_executor,
                             Go2WMotionManager* motion_manager)
  : graph_(graph)
  , map_switcher_(map_switcher)
  , move_base_client_(std::move(move_base_action), true)
  , server_timeout_(server_timeout)
  , segment_timeout_(segment_timeout)
  , move_base_stop_timeout_(move_base_stop_timeout)
  , move_base_goal_active_(false)
  , stair_executor_(stair_executor)
  , motion_manager_(motion_manager)
{
  if (move_base_stop_timeout_ <= 0.0)
  {
    throw std::invalid_argument("move_base_stop_timeout must be positive");
  }
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
  // 平层导航仍由二维 move_base 执行。拓扑和楼梯路径中的 z 保留，但不传给二维控制器。
  goal.target_pose.pose.position.z = 0.0;
  move_base_client_.sendGoal(goal);
  move_base_goal_active_ = true;

  const ros::WallTime started = ros::WallTime::now();
  while (ros::ok())
  {
    if (cancel_requested && cancel_requested())
    {
      std::string stop_message;
      if (!stop_move_base(stop_message))
      {
        message = "route execution canceled; " + stop_message;
        return false;
      }
      message = "route execution canceled";
      return false;
    }
    if (move_base_client_.waitForResult(ros::Duration(0.1)))
    {
      const actionlib::SimpleClientGoalState state = move_base_client_.getState();
      move_base_goal_active_ = false;
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
      std::string stop_message;
      if (!stop_move_base(stop_message))
      {
        message = "move_base segment timeout; " + stop_message;
        return false;
      }
      message = "move_base segment timeout";
      return false;
    }
  }

  move_base_client_.cancelGoal();
  move_base_goal_active_ = false;
  message = "ROS shutdown during route execution";
  return false;
}

bool RouteExecutor::stop_move_base(std::string& message)
{
  // 先撤销所有 move_base 目标，再等待本执行器发送的目标进入终态，避免楼梯控制抢占 cmd_vel。
  move_base_client_.cancelAllGoals();
  if (!move_base_goal_active_)
  {
    ros::WallDuration(0.1).sleep();
    message = "move_base has no active goal";
    return true;
  }

  const ros::WallTime started = ros::WallTime::now();
  while (ros::ok())
  {
    if (move_base_client_.waitForResult(ros::Duration(0.05)))
    {
      move_base_goal_active_ = false;
      ros::WallDuration(0.1).sleep();
      message = "move_base stopped";
      return true;
    }
    if ((ros::WallTime::now() - started).toSec() >= move_base_stop_timeout_)
    {
      message = "move_base did not acknowledge cancellation within " +
                std::to_string(move_base_stop_timeout_) + " seconds";
      return false;
    }
  }

  message = "ROS shutdown while stopping move_base";
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
      if (motion_manager_ != nullptr &&
          motion_manager_->getCurrentMode() != Go2WMotionManager::MotionMode::NORMAL)
      {
        if (feedback)
        {
          feedback(floor_msgs::MultiFloorNavigationFeedback::SWITCHING_MODE,
                   i,
                   map_switcher_.current_floor());
        }
        if (!motion_manager_->setNormalMode())
        {
          message = "failed to switch Go2W to flat mode 0";
          return false;
        }
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

    if (!stop_move_base(message))
    {
      return false;
    }
    if (motion_manager_ != nullptr)
    {
      if (feedback)
      {
        feedback(floor_msgs::MultiFloorNavigationFeedback::SWITCHING_MODE,
                 i,
                 map_switcher_.current_floor());
      }
      if (!motion_manager_->setTerrainMode())
      {
        message = "failed to switch Go2W to stair mode 1";
        return false;
      }
    }
    if (feedback)
    {
      feedback(floor_msgs::MultiFloorNavigationFeedback::STAIR,
               i,
               map_switcher_.current_floor());
    }
    if (!stair_executor_->execute(segment.from_node_id,
                                  segment.from_floor,
                                  segment.to_floor,
                                  *stair_edge,
                                  cancel_requested,
                                  message))
    {
      if (motion_manager_ != nullptr && !motion_manager_->stop())
      {
        message += "; failed to stop Go2W after stair execution failure";
      }
      return false;
    }

    if (feedback)
    {
      feedback(floor_msgs::MultiFloorNavigationFeedback::SWITCHING_MAP,
               i,
               segment.to_floor);
    }
    const bool map_switched = map_switcher_.switch_to(segment.to_floor, message);

    bool normal_mode = true;
    if (motion_manager_ != nullptr)
    {
      if (feedback)
      {
        feedback(floor_msgs::MultiFloorNavigationFeedback::SWITCHING_MODE,
                 i,
                 map_switcher_.current_floor());
      }
      normal_mode = motion_manager_->setNormalMode();
      if (!normal_mode)
      {
        motion_manager_->stop();
      }
    }

    if (!map_switched)
    {
      if (!normal_mode)
      {
        message += "; also failed to switch Go2W back to flat mode 0";
      }
      return false;
    }
    if (!normal_mode)
    {
      message = "floor map switched, but failed to switch Go2W back to flat mode 0";
      return false;
    }
  }

  message = "route execution completed";
  return true;
}

void RouteExecutor::cancel()
{
  move_base_client_.cancelAllGoals();
  move_base_goal_active_ = false;
  if (stair_executor_ != nullptr)
  {
    stair_executor_->cancel();
  }
  if (motion_manager_ != nullptr && motion_manager_->isInitialized())
  {
    motion_manager_->stop();
  }
}

}  // namespace multi_floor_navigation
