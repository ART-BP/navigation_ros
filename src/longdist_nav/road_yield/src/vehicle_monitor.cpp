#include <road_yield/vehicle_monitor.h>

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
  , perception_timeout_(config.perception_timeout)
{
  subscriber_ = nh.subscribe(
      config.tracked_objects_topic, 10, &VehicleMonitor::callback, this);
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
      continue;
    }
    if (road_.isAheadTowardGoal(vehicle_position,
                                robot_position,
                                travel_goal,
                                ahead_margin_,
                                max_detection_ahead_))
    {
      oncoming_ahead = true;
      break;
    }
  }

  bool changed = false;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (travel_goal_version != travel_goal_version_)
    {
      return;
    }
    changed = snapshot_.oncoming_ahead != oncoming_ahead;
    snapshot_.received = true;
    snapshot_.oncoming_ahead = oncoming_ahead;
    snapshot_.reception_time = now;
  }
  if (changed)
  {
    ROS_INFO_STREAM("Vehicle perception: oncoming_in_region_ahead="
                    << std::boolalpha << oncoming_ahead);
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
  return snapshot.received && (now - snapshot.reception_time).toSec() <= perception_timeout_;
}

}  // namespace road_yield
