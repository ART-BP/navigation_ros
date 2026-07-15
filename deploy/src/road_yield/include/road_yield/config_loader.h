#ifndef ROAD_YIELD_CONFIG_LOADER_H_
#define ROAD_YIELD_CONFIG_LOADER_H_

#include <road_yield/road_geometry.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <set>
#include <string>
#include <vector>

namespace road_yield
{

struct NavigationPose
{
  std::string name;
  Point2D position;
  double yaw{0.0};
  bool stair{false};
};

struct AvoidancePoint
{
  NavigationPose shelter;
  bool has_exit{false};
  NavigationPose exit;
  int route_begin{0};
  int route_end{std::numeric_limits<int>::max()};
};

struct TebOverrides
{
  bool has_max_vel_x{false};
  bool has_max_vel_theta{false};
  bool has_acc_lim_x{false};
  bool has_acc_lim_theta{false};
  bool has_weight_kinematics_forward_drive{false};
  bool has_weight_optimaltime{false};
  bool has_weight_viapoint{false};

  double max_vel_x{0.0};
  double max_vel_theta{0.0};
  double acc_lim_x{0.0};
  double acc_lim_theta{0.0};
  double weight_kinematics_forward_drive{0.0};
  double weight_optimaltime{0.0};
  double weight_viapoint{0.0};

  bool empty() const;
};

struct ManagerConfig
{
  std::string planning_frame{"map"};
  std::string robot_base_frame{"base_link"};
  std::string move_base_action{"/move_base"};
  std::string tracked_objects_topic{"/tracked_objects"};
  std::string stair_mode_topic{"/stair_mode"};
  std::string teb_reconfigure_ns{"/move_base/TebLocalPlannerROS"};

  double wait_server_timeout{10.0};
  double tf_timeout{0.1};
  double control_rate{10.0};
  double stair_publish_rate{10.0};
  double stair_warmup_time{1.0};
  double dynamic_reconfigure_timeout{5.0};
  double route_goal_timeout{120.0};
  double avoidance_goal_timeout{120.0};
  double avoidance_progress_timeout{8.0};
  double avoidance_min_progress{0.25};
  int avoidance_candidate_limit{5};
  double near_goal_advance_distance{3.0};
  double failure_retry_distance{3.0};
  int failure_retry_count{2};
  bool continue_on_route_failure{false};

  double perception_timeout{0.8};
  double perception_warning_interval{30.0};
  double avoidance_wait_time{3.0};
  double max_coasted_time{0.3};
  double ahead_margin{0.3};
  double max_detection_ahead{0.0};
  double behind_margin{0.3};
  double max_detection_behind{0.0};
  double stationary_speed_threshold{0.1};
  double opposing_speed_threshold{0.1};
  int detection_window_size{5};
  int detection_required_count{3};
  int clear_required_count{5};
  bool require_perception_before_navigation{true};
  bool road_yield_enabled{false};

  std::set<uint8_t> vehicle_labels;
  std::vector<NavigationPose> route;
  std::vector<RoadCrossSection> road_cross_sections;
  std::vector<AvoidancePoint> avoidance_points;

  // Zero-based route indexes derived from boundary CSV waypoint_line values.
  std::size_t detection_route_begin{0};
  std::size_t detection_route_end{std::numeric_limits<std::size_t>::max()};

  TebOverrides stair_teb_params;
};

// Loads scalar behavior settings and the mandatory patrol CSV. Road-yield
// behavior is enabled only when both optional avoidance and boundary CSVs are
// available.
ManagerConfig loadManagerConfig(const std::string& yaml_path);

}  // namespace road_yield

#endif  // ROAD_YIELD_CONFIG_LOADER_H_
