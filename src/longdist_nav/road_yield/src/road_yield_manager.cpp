#include <road_yield/road_yield_manager.h>

#include <road_yield/config_loader.h>
#include <road_yield/navigation_client.h>
#include <road_yield/pose_provider.h>
#include <road_yield/road_geometry.h>
#include <road_yield/stair_controller.h>
#include <road_yield/vehicle_monitor.h>

#include <ros/ros.h>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

#include <algorithm>
#include <atomic>
#include <limits>
#include <string>
#include <utility>

namespace road_yield
{
namespace
{

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

}  // namespace

class RoadYieldManager::Impl
{
public:
  Impl(ros::NodeHandle nh, const std::string& config_path)
    : nh_(std::move(nh))
    , config_(loadManagerConfig(config_path))
    , road_(config_.road_cross_sections)
    , tf_listener_(tf_buffer_)
    , pose_provider_(tf_buffer_, config_.planning_frame, config_.robot_base_frame, config_.tf_timeout)
    , navigation_(config_.move_base_action, config_.planning_frame, config_.wait_server_timeout)
    , stair_controller_(nh_, config_)
    , vehicle_monitor_(nh_, config_, road_, pose_provider_)
  {
    vehicle_monitor_.setTravelGoal(config_.route.front().position);
    control_timer_ = nh_.createTimer(
        ros::Duration(1.0 / config_.control_rate), &Impl::controlTimerCallback, this);
    ROS_INFO_STREAM("Road detection region length: " << road_.length()
                    << " m; forward direction follows the current patrol goal");
  }

  ~Impl()
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
    navigation_.cancelGoal();
    stair_controller_.disable();
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

  // Check if the oncoming vehicle is confirmed to be ahead of the robot toward the navigation goal.
  // The vehicle must be detected within the configured road region and remain detected for at least the configured confirmation time.
  bool oncomingConfirmed(const ros::Time& now)
  {
    if (route_index_ < config_.detection_route_begin ||
        route_index_ > config_.detection_route_end)
    {
      oncoming_since_ = ros::Time(0);
      return false;
    }

    const PerceptionSnapshot snapshot = vehicle_monitor_.snapshot();
    if (!vehicle_monitor_.isFresh(snapshot, now) || !snapshot.oncoming_ahead)
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
    if (!pose_provider_.robotPosition(robot_position))
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

  bool robotWithinDistance(const NavigationPose& pose, double threshold) const
  {
    if (threshold <= 0.0)
    {
      return false;
    }
    Point2D robot_position;
    return pose_provider_.robotPosition(robot_position) &&
           distance(robot_position, pose.position) <= threshold;
  }

  void enterSafeStop(const std::string& reason)
  {
    ROS_ERROR_STREAM("Entering SAFE_STOP: " << reason);
    navigation_.cancelGoal();
    // If the failure happened on a stair, keep terrain mode active while the
    // robot is stopped. Normal shutdown still restores the original settings.
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

    navigation_.cancelGoal();
    selected_avoidance_index_ = selected;
    route_attempts_ = 0;
    avoidance_attempts_ = 0;
    exit_attempts_ = 0;
    oncoming_since_ = ros::Time(0);
    avoidance_arrival_time_ = ros::Time(0);

    if (!stair_controller_.enable())
    {
      enterSafeStop("could not enable stair mode");
      return;
    }
    stair_ready_time_ = now + ros::Duration(config_.stair_warmup_time);
    setState(ManagerState::PREPARE_YIELD,
             "vehicle inside the road region toward the current patrol goal");
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
      navigation_.sendGoal(current, config_.route_goal_timeout);
      return;
    }

    if (close_enough || config_.continue_on_route_failure)
    {
      ROS_WARN_STREAM("Route goal failed (" << detail
                      << ") but route will advance from " << current.name);
      ++route_index_;
      route_attempts_ = 0;
      if (route_index_ >= config_.route.size())
      {
        stair_controller_.disable();
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
      navigation_.sendGoal(pose, config_.avoidance_goal_timeout);
      return false;
    }
    enterSafeStop("avoidance goal failed after retry policy: " + detail);
    return false;
  }

  void handleRouteNavigation(const ros::Time& now)
  {
    if (route_index_ >= config_.route.size())
    {
      stair_controller_.disable();
      setState(ManagerState::FINISHED, "all route waypoints completed");
      return;
    }

    std::string detail;
    const GoalOutcome outcome = navigation_.pollOutcome(detail);
    if (outcome == GoalOutcome::SUCCEEDED)
    {
      ROS_INFO_STREAM("Route goal reached: " << config_.route[route_index_].name);
      ++route_index_;
      route_attempts_ = 0;
      if (route_index_ >= config_.route.size())
      {
        stair_controller_.disable();
        setState(ManagerState::FINISHED, "all route waypoints completed");
        return;
      }
    }
    else if (outcome == GoalOutcome::FAILED || outcome == GoalOutcome::TIMED_OUT)
    {
      handleRouteFailure(detail);
      return;
    }

    const NavigationPose& current = config_.route[route_index_];
    const bool is_last = route_index_ + 1 == config_.route.size();
    if (navigation_.active() && !is_last &&
        robotWithinDistance(current, config_.near_goal_advance_distance))
    {
      ROS_INFO_STREAM("Passing route goal within near-goal threshold: " << current.name);
      navigation_.cancelGoal();
      ++route_index_;
      route_attempts_ = 0;
    }

    vehicle_monitor_.setTravelGoal(config_.route[route_index_].position);
    if (oncomingConfirmed(now))
    {
      enterYield(now);
      return;
    }

    if (route_index_ < config_.route.size() && !navigation_.active())
    {
      route_attempts_ = std::max(1, route_attempts_);
      navigation_.sendGoal(config_.route[route_index_], config_.route_goal_timeout);
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
        const PerceptionSnapshot snapshot = vehicle_monitor_.snapshot();
        if (!config_.require_perception_before_navigation ||
            vehicle_monitor_.isFresh(snapshot, now))
        {
          setState(ManagerState::ROUTE_NAV, "dependencies are ready");
        }
        else
        {
          ROS_WARN_THROTTLE(3.0,
                            "Waiting for fresh tracked-object perception before starting route");
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
          navigation_.sendGoal(selected.shelter, config_.avoidance_goal_timeout);
          setState(ManagerState::GO_TO_YIELD, "stair mode is ready");
        }
        break;

      case ManagerState::GO_TO_YIELD:
      {
        std::string detail;
        const GoalOutcome outcome = navigation_.pollOutcome(detail);
        const AvoidancePoint& selected =
            config_.avoidance_points[static_cast<std::size_t>(selected_avoidance_index_)];
        if (handleAvoidanceGoalResult(
                outcome, detail, selected.shelter, avoidance_attempts_))
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
        const PerceptionSnapshot snapshot = vehicle_monitor_.snapshot();
        if (!vehicle_monitor_.isFresh(snapshot, now))
        {
          ROS_WARN_THROTTLE(
              2.0,
              "Perception is stale while waiting; cannot confirm the forward road region is clear");
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
          navigation_.sendGoal(selected.exit, config_.avoidance_goal_timeout);
          setState(ManagerState::EXIT_YIELD,
                   "shelter wait elapsed and forward road region is clear");
        }
        else
        {
          ROS_WARN_STREAM("Avoidance point " << selected.shelter.name
                          << " has no exit_pose; disabling stair mode at the shelter as new_path.py does");
          stair_controller_.disable();
          setState(ManagerState::RESUME_ROUTE,
                   "shelter wait elapsed, forward region is clear and no exit pose is configured");
        }
        break;
      }

      case ManagerState::EXIT_YIELD:
      {
        std::string detail;
        const GoalOutcome outcome = navigation_.pollOutcome(detail);
        const AvoidancePoint& selected =
            config_.avoidance_points[static_cast<std::size_t>(selected_avoidance_index_)];
        if (handleAvoidanceGoalResult(outcome, detail, selected.exit, exit_attempts_))
        {
          stair_controller_.disable();
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
  ManagerConfig config_;
  RoadGeometry road_;

  tf2_ros::Buffer tf_buffer_;
  tf2_ros::TransformListener tf_listener_;
  PoseProvider pose_provider_;
  NavigationClient navigation_;
  StairController stair_controller_;
  VehicleMonitor vehicle_monitor_;
  ros::Timer control_timer_;

  ManagerState state_{ManagerState::INIT};
  std::size_t route_index_{0};
  int selected_avoidance_index_{-1};
  int route_attempts_{0};
  int avoidance_attempts_{0};
  int exit_attempts_{0};
  ros::Time stair_ready_time_;
  ros::Time oncoming_since_;
  ros::Time avoidance_arrival_time_;
  std::atomic<bool> shutdown_started_{false};
};

RoadYieldManager::RoadYieldManager(ros::NodeHandle nh, const std::string& config_path)
  : impl_(new Impl(std::move(nh), config_path))
{
}

RoadYieldManager::~RoadYieldManager() = default;

void RoadYieldManager::shutdown()
{
  impl_->shutdown();
}

}  // namespace road_yield
