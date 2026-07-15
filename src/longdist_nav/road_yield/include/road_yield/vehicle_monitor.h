#ifndef ROAD_YIELD_VEHICLE_MONITOR_H_
#define ROAD_YIELD_VEHICLE_MONITOR_H_

#include <road_yield/config_loader.h>
#include <road_yield/pose_provider.h>
#include <road_yield/road_geometry.h>

#include <dynamic_obstacles/TrackedObject.h>
#include <dynamic_obstacles/TrackedObjectArray.h>
#include <ros/ros.h>

#include <cstdint>
#include <mutex>
#include <set>

namespace road_yield
{

struct PerceptionSnapshot
{
  bool received{false};
  bool oncoming_ahead{false};
  ros::Time reception_time;
};

class VehicleMonitor
{
public:
  VehicleMonitor(ros::NodeHandle& nh,
                 const ManagerConfig& config,
                 const RoadGeometry& road,
                 const PoseProvider& pose_provider);

  void setTravelGoal(const Point2D& goal);
  PerceptionSnapshot snapshot() const;
  bool isFresh(const PerceptionSnapshot& snapshot, const ros::Time& now) const;

private:
  bool isConfiguredVehicle(const dynamic_obstacles::TrackedObject& object) const;
  void callback(const dynamic_obstacles::TrackedObjectArray::ConstPtr& message);

  const RoadGeometry& road_;
  const PoseProvider& pose_provider_;
  std::set<std::uint8_t> vehicle_labels_;
  double max_coasted_time_;
  double ahead_margin_;
  double max_detection_ahead_;
  double perception_timeout_;
  ros::Subscriber subscriber_;

  mutable std::mutex mutex_;
  PerceptionSnapshot snapshot_;
  bool has_travel_goal_{false};
  Point2D travel_goal_;
  std::uint64_t travel_goal_version_{0};
};

}  // namespace road_yield

#endif  // ROAD_YIELD_VEHICLE_MONITOR_H_
