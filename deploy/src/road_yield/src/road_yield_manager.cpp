#include <road_yield/road_yield_manager.h>

#include <road_yield/avoidance_strategy.h>
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
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace road_yield
{
namespace
{

enum class ManagerState
{
  INIT,
  ROUTE_NAV,
  PREPARE_ROUTE_STAIR,
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
    case ManagerState::PREPARE_ROUTE_STAIR: return "PREPARE_ROUTE_STAIR";
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
    , avoidance_progress_watchdog_(config_.avoidance_progress_timeout,
                                   config_.avoidance_min_progress)
  {
    vehicle_monitor_.setTravelGoal(config_.route.front().position);
    control_timer_ = nh_.createTimer(
        ros::Duration(1.0 / config_.control_rate), &Impl::controlTimerCallback, this);
    if (config_.road_yield_enabled)
    {
      ROS_INFO_STREAM("Road detection region length: " << road_.length()
                      << " m; forward direction follows the current patrol goal");
    }
    else
    {
      ROS_INFO("Road-yield behavior is disabled; running normal patrol with configured stair waypoints");
    }
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

  // Check the per-message detection window maintained by VehicleMonitor.
  bool oncomingConfirmed(const ros::Time& now)
  {
    if (!config_.road_yield_enabled ||
        route_index_ < config_.detection_route_begin ||
        route_index_ > config_.detection_route_end)
    {
      return false;
    }

    const PerceptionSnapshot snapshot = vehicle_monitor_.snapshot();
    if (!vehicle_monitor_.isFresh(snapshot, now))
    {
      ROS_WARN_THROTTLE(
          config_.perception_warning_interval,
          "Tracked-object perception is unavailable or stale; continuing patrol without road-yield detection");
      return false;
    }
    return snapshot.oncoming_confirmed;
  }

  bool prepareAvoidanceCandidates()
  {
    Point2D robot_position;
    if (!pose_provider_.robotPosition(robot_position))
    {
      return false;
    }

    avoidance_candidate_indexes_ = nearestEligibleAvoidancePoints(
        robot_position,
        config_.avoidance_points,
        route_index_,
        static_cast<std::size_t>(config_.avoidance_candidate_limit));
    failed_avoidance_indexes_.clear();
    if (avoidance_candidate_indexes_.empty())
    {
      return false;
    }
    ROS_INFO_STREAM("Prepared " << avoidance_candidate_indexes_.size()
                    << " nearest avoidance candidates for this yield event");
    return true;
  }

  int selectNextAvoidancePoint() const
  {
    for (const std::size_t index : avoidance_candidate_indexes_)
    {
      if (!failed_avoidance_indexes_.count(index))
      {
        return static_cast<int>(index);
      }
    }
    return -1;
  }

  void sendSelectedAvoidanceGoal(const ros::Time& now)
  {
    const AvoidancePoint& selected =
        config_.avoidance_points[static_cast<std::size_t>(selected_avoidance_index_)];
    navigation_.sendGoal(selected.shelter, config_.avoidance_goal_timeout);

    Point2D robot_position;
    if (pose_provider_.robotPosition(robot_position))
    {
      avoidance_progress_watchdog_.start(
          distance(robot_position, selected.shelter.position), now.toSec());
    }
    else
    {
      avoidance_progress_watchdog_.reset();
      ROS_WARN("Could not initialize avoidance progress watchdog without robot pose");
    }
  }

  bool avoidanceProgressTimedOut(const ros::Time& now, const NavigationPose& shelter)
  {
    Point2D robot_position;
    if (!pose_provider_.robotPosition(robot_position))
    {
      avoidance_progress_watchdog_.suspend(now.toSec());
      ROS_WARN_THROTTLE(5.0, "Cannot monitor avoidance progress without robot pose");
      return false;
    }

    const double current_distance = distance(robot_position, shelter.position);
    if (!avoidance_progress_watchdog_.active())
    {
      avoidance_progress_watchdog_.start(current_distance, now.toSec());
      return false;
    }
    return avoidance_progress_watchdog_.update(current_distance, now.toSec());
  }

  bool switchToNextAvoidancePoint(const ros::Time& now, const std::string& reason)
  {
    const std::size_t failed_index = static_cast<std::size_t>(selected_avoidance_index_);
    const std::string failed_name = config_.avoidance_points[failed_index].shelter.name;
    navigation_.cancelGoal();
    avoidance_progress_watchdog_.reset();
    failed_avoidance_indexes_.insert(failed_index);

    const int next = selectNextAvoidancePoint();
    if (next < 0)
    {
      enterSafeStop("all avoidance candidates failed; last failure at " + failed_name +
                    ": " + reason);
      return false;
    }

    selected_avoidance_index_ = next;
    const std::string& next_name =
        config_.avoidance_points[static_cast<std::size_t>(next)].shelter.name;
    ROS_WARN_STREAM("Avoidance point " << failed_name << " is blacklisted for this yield event ("
                    << reason << "); switching to " << next_name);
    sendSelectedAvoidanceGoal(now);
    return true;
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
    avoidance_progress_watchdog_.reset();
    // Avoidance failures keep terrain mode active while stopped. A patrol-stair
    // failure explicitly restores normal mode before entering SAFE_STOP.
    setState(ManagerState::SAFE_STOP, reason);
  }

  void enterYield(const ros::Time& now)
  {
    avoidance_candidate_indexes_.clear();
    failed_avoidance_indexes_.clear();
    avoidance_progress_watchdog_.reset();
    if (!prepareAvoidanceCandidates())
    {
      enterSafeStop("no eligible avoidance point is available");
      return;
    }
    const int selected = selectNextAvoidancePoint();

    navigation_.cancelGoal();
    selected_avoidance_index_ = selected;
    route_attempts_ = 0;
    exit_attempts_ = 0;
    avoidance_arrival_time_ = ros::Time(0);
    clear_sample_count_ = 0;
    last_clear_perception_sequence_ = vehicle_monitor_.snapshot().sequence;

    const bool stair_mode_was_enabled = stair_controller_.enabled();
    if (!stair_controller_.enable())
    {
      enterSafeStop("could not enable stair mode");
      return;
    }
    stair_ready_time_ = stair_mode_was_enabled ? now :
        now + ros::Duration(config_.stair_warmup_time);
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
      if (current.stair)
      {
        stair_controller_.disable();
      }
      ++route_index_;
      route_attempts_ = 0;
      if (route_index_ >= config_.route.size())
      {
        stair_controller_.disable();
        setState(ManagerState::FINISHED, "all route waypoints processed");
      }
      return;
    }
    if (current.stair)
    {
      stair_controller_.disable();
    }
    enterSafeStop("route goal failed after retry policy: " + detail);
  }

  bool handleExitGoalResult(GoalOutcome outcome,
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
      ROS_WARN_STREAM("Avoidance exit goal failed (" << detail << "); retrying " << pose.name);
      ++attempts;
      navigation_.sendGoal(pose, config_.avoidance_goal_timeout);
      return false;
    }
    enterSafeStop("avoidance exit goal failed after retry policy: " + detail);
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
      const NavigationPose& completed = config_.route[route_index_];
      ROS_INFO_STREAM("Route goal reached: " << completed.name);
      if (completed.stair)
      {
        stair_controller_.disable();
      }
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
    if (navigation_.active() && !is_last && !current.stair &&
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
      if (config_.route[route_index_].stair && !stair_controller_.enabled())
      {
        if (!stair_controller_.enable())
        {
          enterSafeStop("could not enable stair mode for patrol waypoint");
          return;
        }
        stair_ready_time_ = now + ros::Duration(config_.stair_warmup_time);
        setState(ManagerState::PREPARE_ROUTE_STAIR,
                 "preparing configured patrol stair waypoint");
        return;
      }
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
        if (!config_.road_yield_enabled ||
            !config_.require_perception_before_navigation ||
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

      case ManagerState::PREPARE_ROUTE_STAIR:
        if (route_index_ >= config_.route.size())
        {
          stair_controller_.disable();
          enterSafeStop("patrol route ended while preparing stair waypoint");
          break;
        }
        vehicle_monitor_.setTravelGoal(config_.route[route_index_].position);
        if (oncomingConfirmed(now))
        {
          enterYield(now);
          break;
        }
        if (now >= stair_ready_time_)
        {
          navigation_.sendGoal(config_.route[route_index_], config_.route_goal_timeout);
          setState(ManagerState::ROUTE_NAV, "patrol stair mode is ready");
        }
        break;

      case ManagerState::PREPARE_YIELD:
        if (now >= stair_ready_time_)
        {
          sendSelectedAvoidanceGoal(now);
          setState(ManagerState::GO_TO_YIELD, "stair mode is ready");
        }
        break;

      case ManagerState::GO_TO_YIELD:
      {
        std::string detail;
        const GoalOutcome outcome = navigation_.pollOutcome(detail);
        const AvoidancePoint& selected =
            config_.avoidance_points[static_cast<std::size_t>(selected_avoidance_index_)];
        if (outcome == GoalOutcome::SUCCEEDED)
        {
          avoidance_progress_watchdog_.reset();
          avoidance_arrival_time_ = now;
          clear_sample_count_ = 0;
          last_clear_perception_sequence_ = vehicle_monitor_.snapshot().sequence;
          ROS_INFO_STREAM("Avoidance wait timer started at shelter arrival; minimum wait="
                          << config_.avoidance_wait_time << " s");
          setState(ManagerState::WAIT_CLEAR, "avoidance point reached");
        }
        else if (outcome == GoalOutcome::FAILED || outcome == GoalOutcome::TIMED_OUT)
        {
          switchToNextAvoidancePoint(now, "move_base reported " + detail);
        }
        else if (avoidanceProgressTimedOut(now, selected.shelter))
        {
          switchToNextAvoidancePoint(
              now,
              "distance improved by less than " +
                  std::to_string(config_.avoidance_min_progress) + " m in " +
                  std::to_string(config_.avoidance_progress_timeout) + " s");
        }
        break;
      }

      case ManagerState::WAIT_CLEAR:
      {
        const PerceptionSnapshot snapshot = vehicle_monitor_.snapshot();
        if (!vehicle_monitor_.isFresh(snapshot, now))
        {
          clear_sample_count_ = 0;
          ROS_WARN_THROTTLE(
              config_.perception_warning_interval,
              "Perception is stale while waiting; consecutive clear-frame count was reset");
          break;
        }
        if (snapshot.sequence != last_clear_perception_sequence_)
        {
          last_clear_perception_sequence_ = snapshot.sequence;
          if (!snapshot.oncoming_ahead && !snapshot.vehicle_behind)
          {
            ++clear_sample_count_;
          }
          else
          {
            clear_sample_count_ = 0;
          }
        }
        if (avoidance_arrival_time_.isZero() ||
            (now - avoidance_arrival_time_).toSec() < config_.avoidance_wait_time)
        {
          break;
        }
        if (clear_sample_count_ < config_.clear_required_count)
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
        if (handleExitGoalResult(outcome, detail, selected.exit, exit_attempts_))
        {
          stair_controller_.disable();
          setState(ManagerState::RESUME_ROUTE, "normal-road exit pose reached");
        }
        break;
      }

      case ManagerState::RESUME_ROUTE:
        selected_avoidance_index_ = -1;
        avoidance_candidate_indexes_.clear();
        failed_avoidance_indexes_.clear();
        avoidance_progress_watchdog_.reset();
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
  AvoidanceProgressWatchdog avoidance_progress_watchdog_;
  ros::Timer control_timer_;

  ManagerState state_{ManagerState::INIT};
  std::size_t route_index_{0};
  int selected_avoidance_index_{-1};
  int route_attempts_{0};
  int exit_attempts_{0};
  std::vector<std::size_t> avoidance_candidate_indexes_;
  std::set<std::size_t> failed_avoidance_indexes_;
  ros::Time stair_ready_time_;
  ros::Time avoidance_arrival_time_;
  int clear_sample_count_{0};
  std::uint64_t last_clear_perception_sequence_{0};
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
