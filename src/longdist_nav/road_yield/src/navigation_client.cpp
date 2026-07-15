#include <road_yield/navigation_client.h>

#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

#include <stdexcept>
#include <utility>

namespace road_yield
{
namespace
{

move_base_msgs::MoveBaseGoal makeGoal(const NavigationPose& pose, const std::string& frame_id)
{
  move_base_msgs::MoveBaseGoal goal;
  goal.target_pose.header.frame_id = frame_id;
  goal.target_pose.header.stamp = ros::Time::now();
  goal.target_pose.pose.position.x = pose.position.x;
  goal.target_pose.pose.position.y = pose.position.y;
  goal.target_pose.pose.position.z = 0.0;
  tf2::Quaternion orientation;
  orientation.setRPY(0.0, 0.0, pose.yaw);
  goal.target_pose.pose.orientation = tf2::toMsg(orientation);
  return goal;
}

}  // namespace

NavigationClient::NavigationClient(std::string action_name,
                                   std::string planning_frame,
                                   double wait_server_timeout)
  : action_name_(std::move(action_name))
  , planning_frame_(std::move(planning_frame))
  , client_(new MoveBaseClient(action_name_, true))
{
  ROS_INFO_STREAM("Waiting for move_base action server: " << action_name_);
  if (!client_->waitForServer(ros::Duration(wait_server_timeout)))
  {
    throw std::runtime_error("move_base action server is unavailable: " + action_name_);
  }
}

NavigationClient::~NavigationClient()
{
  cancelGoal();
}

void NavigationClient::sendGoal(const NavigationPose& pose, double timeout)
{
  cancelGoal();
  const move_base_msgs::MoveBaseGoal goal = makeGoal(pose, planning_frame_);

  std::uint64_t sequence = 0;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    sequence = ++sequence_;
    goal_active_ = true;
    result_ready_ = false;
    succeeded_ = false;
    state_text_.clear();
    sent_time_ = ros::Time::now();
    timeout_ = timeout;
  }

  ROS_INFO_STREAM("Sending move_base goal: " << pose.name << " [" << pose.position.x << ", "
                                              << pose.position.y << "]");
  client_->sendGoal(
      goal,
      [this, sequence](const actionlib::SimpleClientGoalState& state,
                       const move_base_msgs::MoveBaseResultConstPtr&) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (sequence != sequence_)
        {
          return;
        }
        goal_active_ = false;
        result_ready_ = true;
        succeeded_ = state == actionlib::SimpleClientGoalState::SUCCEEDED;
        state_text_ = state.toString();
      });
}

void NavigationClient::cancelGoal()
{
  bool should_cancel = false;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    should_cancel = goal_active_;
    ++sequence_;
    goal_active_ = false;
    result_ready_ = false;
    succeeded_ = false;
  }
  if (should_cancel && client_)
  {
    client_->cancelGoal();
  }
}

bool NavigationClient::active() const
{
  std::lock_guard<std::mutex> lock(mutex_);
  return goal_active_;
}

GoalOutcome NavigationClient::pollOutcome(std::string& detail)
{
  bool timed_out = false;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (result_ready_)
    {
      result_ready_ = false;
      detail = state_text_;
      return succeeded_ ? GoalOutcome::SUCCEEDED : GoalOutcome::FAILED;
    }
    timed_out = goal_active_ && timeout_ > 0.0 &&
                (ros::Time::now() - sent_time_).toSec() >= timeout_;
  }
  if (timed_out)
  {
    detail = "goal timeout";
    cancelGoal();
    return GoalOutcome::TIMED_OUT;
  }
  return GoalOutcome::NONE;
}

}  // namespace road_yield
