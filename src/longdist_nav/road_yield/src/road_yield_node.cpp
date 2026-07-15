#include <actionlib/client/simple_action_client.h>
#include <dynamic_obstacles/TrackedObject.h>
#include <dynamic_obstacles/TrackedObjectArray.h>
#include <dynamic_reconfigure/client.h>
#include <geometry_msgs/PointStamped.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <ros/ros.h>
#include <std_msgs/Bool.h>
#include <teb_local_planner/TebLocalPlannerReconfigureConfig.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <tf2_ros/transform_listener.h>
#include <road_yield/config_loader.h>
#include <road_yield/road_geometry.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace road_yield
{
namespace
{

using MoveBaseClient = actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction>;
using TebConfig = teb_local_planner::TebLocalPlannerReconfigureConfig;


std::string normalizeFrame(const std::string& frame)
{
  if (!frame.empty() && frame.front() == '/')
  {
    return frame.substr(1);
  }
  return frame;
}

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

enum class ManagerState
{
  INIT,
  ROUTE_NAV,
  PREPARE_YIELD,
  GO_TO_YIELD,
  WAIT_CLEAR,
  EXIT_YIELD,
  RESUME_ROUTE,
  FINISHED,
  SAFE_STOP,
};

const char* stateName(ManagerState state)
{
  switch (state)
  {
    case ManagerState::INIT: return "INIT";
    case ManagerState::ROUTE_NAV: return "ROUTE_NAV";
    case ManagerState::PREPARE_YIELD: return "PREPARE_YIELD";
    case ManagerState::GO_TO_YIELD: return "GO_TO_YIELD";
    case ManagerState::WAIT_CLEAR: return "WAIT_CLEAR";
    case ManagerState::EXIT_YIELD: return "EXIT_YIELD";
    case ManagerState::RESUME_ROUTE: return "RESUME_ROUTE";
    case ManagerState::FINISHED: return "FINISHED";
    case ManagerState::SAFE_STOP: return "SAFE_STOP";
  }
  return "UNKNOWN";
}

enum class GoalOutcome
{
  NONE,
  SUCCEEDED,
  FAILED,
  TIMED_OUT,
};

struct PerceptionSnapshot
{
  bool received{false};
  bool oncoming_ahead{false};
  ros::Time reception_time;
};

}  // namespace

class RoadYieldManager
{
public:
  RoadYieldManager(ros::NodeHandle nh, ros::NodeHandle private_nh, const std::string& config_path)
    : nh_(std::move(nh))
    , private_nh_(std::move(private_nh))
    , config_(loadManagerConfig(config_path))
    , road_(new RoadGeometry(config_.road_cross_sections))
    , tf_listener_(tf_buffer_)
  {
    move_base_.reset(new MoveBaseClient(config_.move_base_action, true));
    ROS_INFO_STREAM("Waiting for move_base action server: " << config_.move_base_action);
    if (!move_base_->waitForServer(ros::Duration(config_.wait_server_timeout)))
    {
      throw std::runtime_error("move_base action server is unavailable: " + config_.move_base_action);
    }

    teb_client_.reset(new dynamic_reconfigure::Client<TebConfig>(config_.teb_reconfigure_ns));
    stair_mode_pub_ = nh_.advertise<std_msgs::Bool>(config_.stair_mode_topic, 1, true);
    tracked_objects_sub_ = nh_.subscribe(
        config_.tracked_objects_topic, 10, &RoadYieldManager::trackedObjectsCallback, this);
    control_timer_ = nh_.createTimer(
        ros::Duration(1.0 / config_.control_rate), &RoadYieldManager::controlTimerCallback, this);
    stair_timer_ = nh_.createTimer(
        ros::Duration(1.0 / config_.stair_publish_rate), &RoadYieldManager::stairTimerCallback, this);

    publishStairMode(false);
    ROS_INFO_STREAM("Road detection region length: " << road_->length()
                    << " m; cross sections are interpreted in robot travel direction");
  }

  ~RoadYieldManager()
  {
    shutdown();
  }

  void shutdown()
  {
    bool expected = false;
    if (!shutdown_started_.compare_exchange_strong(expected, true))
    {
      return;
    }
    cancelActiveGoal();
    disableStairMode();
  }

private:
  void setState(ManagerState next, const std::string& reason)
  {
    if (state_ == next)
    {
      return;
    }
    ROS_INFO_STREAM("Road yield state: " << stateName(state_) << " -> " << stateName(next)
                                         << " (" << reason << ")");
    state_ = next;
  }

  bool lookupRobotPosition(Point2D& position) const
  {
    try
    {
      const geometry_msgs::TransformStamped transform = tf_buffer_.lookupTransform(
          config_.planning_frame, config_.robot_base_frame, ros::Time(0), ros::Duration(config_.tf_timeout));
      position.x = transform.transform.translation.x;
      position.y = transform.transform.translation.y;
      return std::isfinite(position.x) && std::isfinite(position.y);
    }
    catch (const tf2::TransformException& exception)
    {
      ROS_WARN_THROTTLE(2.0, "Cannot get robot pose %s -> %s: %s",
                        config_.planning_frame.c_str(), config_.robot_base_frame.c_str(), exception.what());
      return false;
    }
  }

  bool transformObjectPosition(const geometry_msgs::Point& input,
                               const std::string& source_frame,
                               const ros::Time& stamp,
                               Point2D& output) const
  {
    if (!std::isfinite(input.x) || !std::isfinite(input.y))
    {
      return false;
    }
    if (normalizeFrame(source_frame) == config_.planning_frame)
    {
      output.x = input.x;
      output.y = input.y;
      return true;
    }

    geometry_msgs::PointStamped source;
    source.header.frame_id = source_frame;
    source.header.stamp = stamp;
    source.point = input;
    geometry_msgs::PointStamped transformed;
    try
    {
      tf_buffer_.transform(source, transformed, config_.planning_frame, ros::Duration(config_.tf_timeout));
      output.x = transformed.point.x;
      output.y = transformed.point.y;
      return std::isfinite(output.x) && std::isfinite(output.y);
    }
    catch (const tf2::TransformException& exception)
    {
      ROS_WARN_THROTTLE(2.0, "Cannot transform tracked object from %s to %s: %s",
                        source_frame.c_str(), config_.planning_frame.c_str(), exception.what());
      return false;
    }
  }

  bool isConfiguredVehicle(const dynamic_obstacles::TrackedObject& object) const
  {
    if (!config_.vehicle_labels.count(object.classification.label))
    {
      return false;
    }
    if (object.track_state == dynamic_obstacles::TrackedObject::TRACK_COASTED &&
        object.time_since_update > config_.max_coasted_time)
    {
      return false;
    }
    return true;
  }

  void trackedObjectsCallback(const dynamic_obstacles::TrackedObjectArray::ConstPtr& message)
  {
    const ros::Time now = ros::Time::now();
    bool oncoming_ahead = false;

    Point2D robot_position;
    const bool robot_position_valid = lookupRobotPosition(robot_position);
    const RoadProjection robot_projection =
        robot_position_valid ? road_->projectToCenterline(robot_position) : RoadProjection{};

    for (const dynamic_obstacles::TrackedObject& object : message->objects)
    {
      if (!isConfiguredVehicle(object))
      {
        continue;
      }
      if (!robot_projection.valid)
      {
        continue;
      }
      Point2D vehicle_position;
      if (!transformObjectPosition(object.pose.pose.position, message->header.frame_id,
                                   message->header.stamp, vehicle_position))
      {
        continue;
      }
      if (road_->isAheadInside(vehicle_position, robot_projection.station,
                               config_.ahead_margin, config_.max_detection_ahead))
      {
        oncoming_ahead = true;
      }
    }

    bool changed = false;
    {
      std::lock_guard<std::mutex> lock(perception_mutex_);
      changed = perception_.oncoming_ahead != oncoming_ahead;
      perception_.received = true;
      perception_.oncoming_ahead = oncoming_ahead;
      perception_.reception_time = now;
    }
    if (changed)
    {
      ROS_INFO_STREAM("Vehicle perception: oncoming_in_region_ahead=" << std::boolalpha << oncoming_ahead);
    }
  }

  PerceptionSnapshot perceptionSnapshot() const
  {
    std::lock_guard<std::mutex> lock(perception_mutex_);
    return perception_;
  }

  bool perceptionFresh(const PerceptionSnapshot& snapshot, const ros::Time& now) const
  {
    return snapshot.received && (now - snapshot.reception_time).toSec() <= config_.perception_timeout;
  }

  bool oncomingConfirmed(const ros::Time& now)
  {
    if (route_index_ < config_.detection_route_begin || route_index_ > config_.detection_route_end)
    {
      oncoming_since_ = ros::Time(0);
      return false;
    }
    const PerceptionSnapshot snapshot = perceptionSnapshot();
    if (!perceptionFresh(snapshot, now) || !snapshot.oncoming_ahead)
    {
      oncoming_since_ = ros::Time(0);
      return false;
    }
    if (oncoming_since_.isZero())
    {
      oncoming_since_ = now;
    }
    return (now - oncoming_since_).toSec() >= config_.oncoming_confirm_time;
  }

  int selectNearestAvoidancePoint() const
  {
    Point2D robot_position;
    if (!lookupRobotPosition(robot_position))
    {
      return -1;
    }

    int selected = -1;
    double selected_distance = std::numeric_limits<double>::infinity();
    for (std::size_t i = 0; i < config_.avoidance_points.size(); ++i)
    {
      const AvoidancePoint& candidate = config_.avoidance_points[i];
      if (static_cast<int>(route_index_) < candidate.route_begin ||
          static_cast<int>(route_index_) > candidate.route_end)
      {
        continue;
      }
      const double candidate_distance = distance(robot_position, candidate.shelter.position);
      if (candidate_distance < selected_distance)
      {
        selected = static_cast<int>(i);
        selected_distance = candidate_distance;
      }
    }

    if (selected >= 0)
    {
      ROS_INFO_STREAM("Selected nearest avoidance point "
                      << config_.avoidance_points[static_cast<std::size_t>(selected)].shelter.name
                      << " at Euclidean distance " << selected_distance << " m");
    }
    return selected;
  }

  void sendGoal(const NavigationPose& pose, double timeout)
  {
    cancelActiveGoal();
    const move_base_msgs::MoveBaseGoal goal = makeGoal(pose, config_.planning_frame);

    uint64_t sequence = 0;
    {
      std::lock_guard<std::mutex> lock(goal_mutex_);
      sequence = ++goal_sequence_;
      goal_active_ = true;
      goal_result_ready_ = false;
      goal_succeeded_ = false;
      goal_state_text_.clear();
      goal_sent_time_ = ros::Time::now();
      goal_timeout_ = timeout;
    }

    ROS_INFO_STREAM("Sending move_base goal: " << pose.name << " [" << pose.position.x << ", "
                                               << pose.position.y << "]");
    move_base_->sendGoal(
        goal,
        [this, sequence](const actionlib::SimpleClientGoalState& state,
                         const move_base_msgs::MoveBaseResultConstPtr&) {
          std::lock_guard<std::mutex> lock(goal_mutex_);
          if (sequence != goal_sequence_)
          {
            return;
          }
          goal_active_ = false;
          goal_result_ready_ = true;
          goal_succeeded_ = state == actionlib::SimpleClientGoalState::SUCCEEDED;
          goal_state_text_ = state.toString();
        });
  }

  void cancelActiveGoal()
  {
    bool should_cancel = false;
    {
      std::lock_guard<std::mutex> lock(goal_mutex_);
      should_cancel = goal_active_;
      ++goal_sequence_;
      goal_active_ = false;
      goal_result_ready_ = false;
      goal_succeeded_ = false;
    }
    if (should_cancel && move_base_)
    {
      move_base_->cancelGoal();
    }
  }

  bool goalActive() const
  {
    std::lock_guard<std::mutex> lock(goal_mutex_);
    return goal_active_;
  }

  GoalOutcome pollGoalOutcome(std::string& detail)
  {
    bool timed_out = false;
    {
      std::lock_guard<std::mutex> lock(goal_mutex_);
      if (goal_result_ready_)
      {
        goal_result_ready_ = false;
        detail = goal_state_text_;
        return goal_succeeded_ ? GoalOutcome::SUCCEEDED : GoalOutcome::FAILED;
      }
      timed_out = goal_active_ && goal_timeout_ > 0.0 &&
                  (ros::Time::now() - goal_sent_time_).toSec() >= goal_timeout_;
    }
    if (timed_out)
    {
      detail = "goal timeout";
      cancelActiveGoal();
      return GoalOutcome::TIMED_OUT;
    }
    return GoalOutcome::NONE;
  }

  bool robotWithinDistance(const NavigationPose& pose, double threshold) const
  {
    if (threshold <= 0.0)
    {
      return false;
    }
    Point2D robot_position;
    return lookupRobotPosition(robot_position) && distance(robot_position, pose.position) <= threshold;
  }

  void applyConfiguredTebValues(TebConfig& target, const TebConfig* restore_source) const
  {
    const TebOverrides& values = config_.stair_teb_params;
    const bool restore = restore_source != nullptr;
    if (values.has_max_vel_x)
      target.max_vel_x = restore ? restore_source->max_vel_x : values.max_vel_x;
    if (values.has_max_vel_theta)
      target.max_vel_theta = restore ? restore_source->max_vel_theta : values.max_vel_theta;
    if (values.has_acc_lim_x)
      target.acc_lim_x = restore ? restore_source->acc_lim_x : values.acc_lim_x;
    if (values.has_acc_lim_theta)
      target.acc_lim_theta = restore ? restore_source->acc_lim_theta : values.acc_lim_theta;
    if (values.has_weight_kinematics_forward_drive)
      target.weight_kinematics_forward_drive = restore ? restore_source->weight_kinematics_forward_drive :
                                                        values.weight_kinematics_forward_drive;
    if (values.has_weight_optimaltime)
      target.weight_optimaltime = restore ? restore_source->weight_optimaltime : values.weight_optimaltime;
    if (values.has_weight_viapoint)
      target.weight_viapoint = restore ? restore_source->weight_viapoint : values.weight_viapoint;
  }

  bool enableStairMode()
  {
    if (stair_mode_enabled_.load())
    {
      return true;
    }

    TebConfig current;
    if (!config_.stair_teb_params.empty())
    {
      if (!teb_client_->getCurrentConfiguration(current, ros::Duration(config_.dynamic_reconfigure_timeout)))
      {
        ROS_ERROR("Timed out while reading current TEB dynamic-reconfigure values");
        return false;
      }
      original_teb_config_ = current;
      applyConfiguredTebValues(current, nullptr);
      if (!teb_client_->setConfiguration(current))
      {
        ROS_ERROR("Failed to apply stair TEB dynamic-reconfigure values");
        return false;
      }
      teb_config_saved_ = true;
    }

    stair_mode_enabled_.store(true);
    publishStairMode(true);
    ROS_INFO("Stair mode enabled");
    return true;
  }

  void disableStairMode()
  {
    stair_mode_enabled_.store(false);
    publishStairMode(false);

    if (teb_config_saved_ && teb_client_)
    {
      TebConfig current;
      if (teb_client_->getCurrentConfiguration(current, ros::Duration(config_.dynamic_reconfigure_timeout)))
      {
        applyConfiguredTebValues(current, &original_teb_config_);
        if (!teb_client_->setConfiguration(current))
        {
          ROS_ERROR("Failed to restore original TEB dynamic-reconfigure values");
        }
      }
      else
      {
        ROS_ERROR("Timed out while preparing to restore original TEB values");
      }
      teb_config_saved_ = false;
    }
  }

  void publishStairMode(bool enabled)
  {
    if (stair_mode_pub_)
    {
      std_msgs::Bool message;
      message.data = enabled;
      stair_mode_pub_.publish(message);
    }
  }

  void stairTimerCallback(const ros::TimerEvent&)
  {
    publishStairMode(stair_mode_enabled_.load());
  }

  void enterSafeStop(const std::string& reason)
  {
    ROS_ERROR_STREAM("Entering SAFE_STOP: " << reason);
    cancelActiveGoal();
    // Keep stair mode active when a failure occurs on the stair. This avoids
    // changing terrain filtering underneath a stopped robot.
    setState(ManagerState::SAFE_STOP, reason);
  }

  void enterYield(const ros::Time& now)
  {
    const int selected = selectNearestAvoidancePoint();
    if (selected < 0)
    {
      enterSafeStop("no eligible avoidance point is available");
      return;
    }

    cancelActiveGoal();
    selected_avoidance_index_ = selected;
    route_attempts_ = 0;
    avoidance_attempts_ = 0;
    exit_attempts_ = 0;
    oncoming_since_ = ros::Time(0);
    avoidance_arrival_time_ = ros::Time(0);

    if (!enableStairMode())
    {
      enterSafeStop("could not enable stair mode");
      return;
    }
    stair_ready_time_ = now + ros::Duration(config_.stair_warmup_time);
    setState(ManagerState::PREPARE_YIELD, "oncoming vehicle inside road region ahead");
  }

  void handleRouteFailure(const std::string& detail)
  {
    const int max_attempts = 1 + std::max(0, config_.failure_retry_count);
    const NavigationPose& current = config_.route[route_index_];
    const bool close_enough = robotWithinDistance(current, config_.failure_retry_distance);
    if (!close_enough && route_attempts_ < max_attempts)
    {
      ROS_WARN_STREAM("Route goal failed (" << detail << "); retrying " << current.name);
      ++route_attempts_;
      sendGoal(current, config_.route_goal_timeout);
      return;
    }

    if (close_enough || config_.continue_on_route_failure)
    {
      ROS_WARN_STREAM("Route goal failed (" << detail << ") but route will advance from " << current.name);
      ++route_index_;
      route_attempts_ = 0;
      if (route_index_ >= config_.route.size())
      {
        disableStairMode();
        setState(ManagerState::FINISHED, "all route waypoints processed");
      }
      return;
    }
    enterSafeStop("route goal failed after retry policy: " + detail);
  }

  bool handleAvoidanceGoalResult(GoalOutcome outcome,
                                 const std::string& detail,
                                 const NavigationPose& pose,
                                 int& attempts)
  {
    if (outcome == GoalOutcome::SUCCEEDED)
    {
      return true;
    }
    if (outcome == GoalOutcome::NONE)
    {
      return false;
    }

    const int max_attempts = 1 + std::max(0, config_.failure_retry_count);
    if (attempts < max_attempts)
    {
      ROS_WARN_STREAM("Avoidance goal failed (" << detail << "); retrying " << pose.name);
      ++attempts;
      sendGoal(pose, config_.avoidance_goal_timeout);
      return false;
    }
    enterSafeStop("avoidance goal failed after retry policy: " + detail);
    return false;
  }

  void handleRouteNavigation(const ros::Time& now)
  {
    if (route_index_ >= config_.route.size())
    {
      disableStairMode();
      setState(ManagerState::FINISHED, "all route waypoints completed");
      return;
    }

    std::string detail;
    const GoalOutcome outcome = pollGoalOutcome(detail);
    if (outcome == GoalOutcome::SUCCEEDED)
    {
      ROS_INFO_STREAM("Route goal reached: " << config_.route[route_index_].name);
      ++route_index_;
      route_attempts_ = 0;
      if (route_index_ >= config_.route.size())
      {
        disableStairMode();
        setState(ManagerState::FINISHED, "all route waypoints completed");
        return;
      }
    }
    else if (outcome == GoalOutcome::FAILED || outcome == GoalOutcome::TIMED_OUT)
    {
      handleRouteFailure(detail);
      return;
    }

    if (oncomingConfirmed(now))
    {
      enterYield(now);
      return;
    }

    const NavigationPose& current = config_.route[route_index_];
    const bool is_last = route_index_ + 1 == config_.route.size();
    if (goalActive() && !is_last && robotWithinDistance(current, config_.near_goal_advance_distance))
    {
      ROS_INFO_STREAM("Passing route goal within near-goal threshold: " << current.name);
      cancelActiveGoal();
      ++route_index_;
      route_attempts_ = 0;
    }

    if (route_index_ < config_.route.size() && !goalActive())
    {
      route_attempts_ = std::max(1, route_attempts_);
      sendGoal(config_.route[route_index_], config_.route_goal_timeout);
    }
  }

  void controlTimerCallback(const ros::TimerEvent&)
  {
    if (shutdown_started_.load())
    {
      return;
    }
    const ros::Time now = ros::Time::now();

    switch (state_)
    {
      case ManagerState::INIT:
      {
        const PerceptionSnapshot snapshot = perceptionSnapshot();
        if (!config_.require_perception_before_navigation || perceptionFresh(snapshot, now))
        {
          setState(ManagerState::ROUTE_NAV, "dependencies are ready");
        }
        else
        {
          ROS_WARN_THROTTLE(3.0, "Waiting for fresh tracked-object perception before starting route");
        }
        break;
      }

      case ManagerState::ROUTE_NAV:
        handleRouteNavigation(now);
        break;

      case ManagerState::PREPARE_YIELD:
        if (now >= stair_ready_time_)
        {
          const AvoidancePoint& selected =
              config_.avoidance_points[static_cast<std::size_t>(selected_avoidance_index_)];
          avoidance_attempts_ = 1;
          sendGoal(selected.shelter, config_.avoidance_goal_timeout);
          setState(ManagerState::GO_TO_YIELD, "stair mode is ready");
        }
        break;

      case ManagerState::GO_TO_YIELD:
      {
        std::string detail;
        const GoalOutcome outcome = pollGoalOutcome(detail);
        const AvoidancePoint& selected =
            config_.avoidance_points[static_cast<std::size_t>(selected_avoidance_index_)];
        if (handleAvoidanceGoalResult(outcome, detail, selected.shelter, avoidance_attempts_))
        {
          avoidance_arrival_time_ = now;
          ROS_INFO_STREAM("Avoidance wait timer started at shelter arrival; minimum wait="
                          << config_.avoidance_wait_time << " s");
          setState(ManagerState::WAIT_CLEAR, "avoidance point reached");
        }
        break;
      }

      case ManagerState::WAIT_CLEAR:
      {
        const PerceptionSnapshot snapshot = perceptionSnapshot();
        if (!perceptionFresh(snapshot, now))
        {
          ROS_WARN_THROTTLE(2.0, "Perception is stale while waiting; cannot confirm the forward road region is clear");
          break;
        }
        if (avoidance_arrival_time_.isZero() ||
            (now - avoidance_arrival_time_).toSec() < config_.avoidance_wait_time)
        {
          break;
        }
        if (snapshot.oncoming_ahead)
        {
          break;
        }

        const AvoidancePoint& selected =
            config_.avoidance_points[static_cast<std::size_t>(selected_avoidance_index_)];
        if (selected.has_exit)
        {
          exit_attempts_ = 1;
          sendGoal(selected.exit, config_.avoidance_goal_timeout);
          setState(ManagerState::EXIT_YIELD, "shelter wait elapsed and forward road region is clear");
        }
        else
        {
          ROS_WARN_STREAM("Avoidance point " << selected.shelter.name
                          << " has no exit_pose; disabling stair mode at the shelter as new_path.py does");
          disableStairMode();
          setState(ManagerState::RESUME_ROUTE,
                   "shelter wait elapsed, forward region is clear and no exit pose is configured");
        }
        break;
      }

      case ManagerState::EXIT_YIELD:
      {
        std::string detail;
        const GoalOutcome outcome = pollGoalOutcome(detail);
        const AvoidancePoint& selected =
            config_.avoidance_points[static_cast<std::size_t>(selected_avoidance_index_)];
        if (handleAvoidanceGoalResult(outcome, detail, selected.exit, exit_attempts_))
        {
          disableStairMode();
          setState(ManagerState::RESUME_ROUTE, "normal-road exit pose reached");
        }
        break;
      }

      case ManagerState::RESUME_ROUTE:
        selected_avoidance_index_ = -1;
        route_attempts_ = 0;
        avoidance_arrival_time_ = ros::Time(0);
        setState(ManagerState::ROUTE_NAV, "resuming interrupted route waypoint");
        break;

      case ManagerState::FINISHED:
      case ManagerState::SAFE_STOP:
        break;
    }
  }

  ros::NodeHandle nh_;
  ros::NodeHandle private_nh_;
  ManagerConfig config_;
  std::unique_ptr<RoadGeometry> road_;

  tf2_ros::Buffer tf_buffer_;
  tf2_ros::TransformListener tf_listener_;
  std::unique_ptr<MoveBaseClient> move_base_;
  std::unique_ptr<dynamic_reconfigure::Client<TebConfig>> teb_client_;

  ros::Publisher stair_mode_pub_;
  ros::Subscriber tracked_objects_sub_;
  ros::Timer control_timer_;
  ros::Timer stair_timer_;

  ManagerState state_{ManagerState::INIT};
  std::size_t route_index_{0};
  int selected_avoidance_index_{-1};
  int route_attempts_{0};
  int avoidance_attempts_{0};
  int exit_attempts_{0};
  ros::Time stair_ready_time_;
  ros::Time oncoming_since_;
  ros::Time avoidance_arrival_time_;

  mutable std::mutex perception_mutex_;
  PerceptionSnapshot perception_;

  mutable std::mutex goal_mutex_;
  uint64_t goal_sequence_{0};
  bool goal_active_{false};
  bool goal_result_ready_{false};
  bool goal_succeeded_{false};
  std::string goal_state_text_;
  ros::Time goal_sent_time_;
  double goal_timeout_{0.0};

  std::atomic<bool> stair_mode_enabled_{false};
  bool teb_config_saved_{false};
  TebConfig original_teb_config_;
  std::atomic<bool> shutdown_started_{false};
};

}  // namespace road_yield

int main(int argc, char** argv)
{
  ros::init(argc, argv, "road_yield");
  ros::NodeHandle nh;
  ros::NodeHandle private_nh("~");

  std::string config_path;
  private_nh.param<std::string>("config", config_path, std::string());
  if (config_path.empty() && argc >= 2)
  {
    config_path = argv[1];
  }
  if (config_path.empty())
  {
    ROS_FATAL("Set ~config or pass a road-yield YAML path as the first argument");
    return 2;
  }

  try
  {
    road_yield::RoadYieldManager manager(nh, private_nh, config_path);
    ros::AsyncSpinner spinner(3);
    spinner.start();
    ros::waitForShutdown();
    manager.shutdown();
  }
  catch (const std::exception& exception)
  {
    ROS_FATAL_STREAM("road_yield failed: " << exception.what());
    return 1;
  }
  return 0;
}
