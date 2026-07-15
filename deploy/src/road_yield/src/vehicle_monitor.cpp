#include <road_yield/vehicle_monitor.h>

#include <cmath>
#include <utility>

namespace road_yield
{

VehicleMonitor::VehicleMonitor(ros::NodeHandle& nh,
                               const ManagerConfig& config,
                               const RoadGeometry& road,
                               const PoseProvider& pose_provider)
  : road_(road)
  , pose_provider_(pose_provider)
  , vehicle_labels_(config.vehicle_labels)
  , max_coasted_time_(config.max_coasted_time)
  , ahead_margin_(config.ahead_margin)
  , max_detection_ahead_(config.max_detection_ahead)
  , behind_margin_(config.behind_margin)
  , max_detection_behind_(config.max_detection_behind)
  , stationary_speed_threshold_(config.stationary_speed_threshold)
  , opposing_speed_threshold_(config.opposing_speed_threshold)
  , perception_timeout_(config.perception_timeout)
  , oncoming_window_(static_cast<std::size_t>(config.detection_window_size),
                     static_cast<std::size_t>(config.detection_required_count))
{
  if (config.road_yield_enabled)
  {
    subscriber_ = nh.subscribe(
        config.tracked_objects_topic, 10, &VehicleMonitor::callback, this);
  }
}

bool VehicleMonitor::isConfiguredVehicle(const dynamic_obstacles::TrackedObject& object) const
{
  if (!vehicle_labels_.count(object.classification.label))
  {
    return false;
  }
  if (object.track_state == dynamic_obstacles::TrackedObject::TRACK_COASTED &&
      object.time_since_update > max_coasted_time_)
  {
    return false;
  }
  return true;
}

void VehicleMonitor::callback(const dynamic_obstacles::TrackedObjectArray::ConstPtr& message)
{
  const ros::Time now = ros::Time::now();
  bool oncoming_ahead = false;
  bool vehicle_behind = false;

  Point2D travel_goal;
  std::uint64_t travel_goal_version = 0;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!has_travel_goal_)
    {
      return;
    }
    travel_goal = travel_goal_;
    travel_goal_version = travel_goal_version_;
  }

  Point2D robot_position;
  if (!pose_provider_.robotPosition(robot_position))
  {
    return;
  }

  const double direction_x = travel_goal.x - robot_position.x;
  const double direction_y = travel_goal.y - robot_position.y;
  const double direction_length = std::hypot(direction_x, direction_y);
  if (direction_length <= 1e-9)
  {
    return;
  }
  const double unit_x = direction_x / direction_length;
  const double unit_y = direction_y / direction_length;

  for (const dynamic_obstacles::TrackedObject& object : message->objects)
  {
    if (!isConfiguredVehicle(object))
    {
      continue;
    }

    Point2D vehicle_position;
    if (!pose_provider_.transformPoint(object.pose.pose.position,
                                       message->header.frame_id,
                                       message->header.stamp,
                                       vehicle_position))
    {
      return;
    }
    if (!road_.contains(vehicle_position))
    {
      continue;
    }

    const double longitudinal_distance =
        (vehicle_position.x - robot_position.x) * unit_x +
        (vehicle_position.y - robot_position.y) * unit_y;
    const bool inside_ahead_range =
        longitudinal_distance > ahead_margin_ &&
        (max_detection_ahead_ <= 0.0 || longitudinal_distance <= max_detection_ahead_);
    const bool inside_behind_range =
        longitudinal_distance < -behind_margin_ &&
        (max_detection_behind_ <= 0.0 || -longitudinal_distance <= max_detection_behind_);

    if (inside_behind_range)
    {
      vehicle_behind = true;
    }

    if (!inside_ahead_range)
    {
      continue;
    }

    const bool reported_stationary =
        object.motion_state == dynamic_obstacles::TrackedObject::MOTION_STATIONARY;
    Point2D velocity;
    if (object.has_linear_velocity)
    {
      if (!pose_provider_.transformVector(object.twist.twist.linear,
                                          message->header.frame_id,
                                          message->header.stamp,
                                          velocity))
      {
        return;
      }
    }

    if (isSingleMessageOncoming(reported_stationary,
                                object.has_linear_velocity,
                                velocity.x,
                                velocity.y,
                                unit_x,
                                unit_y,
                                stationary_speed_threshold_,
                                opposing_speed_threshold_))
    {
      oncoming_ahead = true;
    }
  }

  bool changed = false;
  bool oncoming_confirmed = false;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (travel_goal_version != travel_goal_version_)
    {
      return;
    }
    oncoming_confirmed = oncoming_window_.add(oncoming_ahead);
    changed = snapshot_.oncoming_ahead != oncoming_ahead ||
              snapshot_.oncoming_confirmed != oncoming_confirmed ||
              snapshot_.vehicle_behind != vehicle_behind;
    snapshot_.received = true;
    snapshot_.oncoming_ahead = oncoming_ahead;
    snapshot_.oncoming_confirmed = oncoming_confirmed;
    snapshot_.vehicle_behind = vehicle_behind;
    ++snapshot_.sequence;
    snapshot_.reception_time = now;
  }
  if (changed)
  {
    ROS_INFO_STREAM("Vehicle perception: oncoming_now=" << std::boolalpha << oncoming_ahead
                    << ", oncoming_window_confirmed=" << oncoming_confirmed
                    << ", vehicle_behind=" << vehicle_behind);
  }
}

void VehicleMonitor::setTravelGoal(const Point2D& goal)
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (has_travel_goal_ && distance(travel_goal_, goal) <= 1e-6)
  {
    return;
  }

  travel_goal_ = goal;
  has_travel_goal_ = true;
  ++travel_goal_version_;
  oncoming_window_.reset();
  snapshot_ = PerceptionSnapshot{};
  ROS_INFO_STREAM("Vehicle detection direction updated toward navigation goal ["
                  << goal.x << ", " << goal.y << "]");
}

PerceptionSnapshot VehicleMonitor::snapshot() const
{
  std::lock_guard<std::mutex> lock(mutex_);
  return snapshot_;
}

bool VehicleMonitor::isFresh(const PerceptionSnapshot& snapshot, const ros::Time& now) const
{
  const double age = (now - snapshot.reception_time).toSec();
  return snapshot.received && age >= 0.0 && age <= perception_timeout_;
}

}  // namespace road_yield
