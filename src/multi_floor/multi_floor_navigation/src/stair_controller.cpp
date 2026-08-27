#include "multi_floor_navigation/stair_controller.h"

#include <tf2/utils.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

#include <sensor_msgs/point_cloud2_iterator.h>

#include <boost/bind.hpp>

#include <algorithm>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

namespace multi_floor_navigation
{
namespace
{

double clamp(double value, double lower, double upper)
{
  return std::max(lower, std::min(value, upper));
}

double normalize_angle(double angle)
{
  return std::atan2(std::sin(angle), std::cos(angle));
}

double position_distance(double first_x, double first_y, double second_x, double second_y)
{
  return std::hypot(second_x - first_x, second_y - first_y);
}

template <typename T>
T parameter(ros::NodeHandle& node, const std::string& name, const T& default_value)
{
  T value = default_value;
  node.param(name, value, default_value);
  return value;
}

Pose3D pose_from_message(const geometry_msgs::Pose& message)
{
  Pose3D pose;
  pose.x = message.position.x;
  pose.y = message.position.y;
  pose.z = message.position.z;
  // 楼梯路径点的orientation不参与控制；每段yaw由相邻点的连线计算。
  pose.yaw = 0.0;
  return pose;
}

class StopOnExit
{
public:
  explicit StopOnExit(std::function<void()> stop) : stop_(std::move(stop))
  {
  }

  ~StopOnExit()
  {
    stop_();
  }

private:
  std::function<void()> stop_;
};

}  // namespace

PidController::PidController(double kp,
                             double ki,
                             double kd,
                             double integral_limit,
                             double output_limit)
  : kp_(kp)
  , ki_(ki)
  , kd_(kd)
  , integral_limit_(std::fabs(integral_limit))
  , output_limit_(std::fabs(output_limit))
  , integral_(0.0)
  , previous_error_(0.0)
  , initialized_(false)
{
}

void PidController::reset()
{
  integral_ = 0.0;
  previous_error_ = 0.0;
  initialized_ = false;
}

double PidController::update(double error, double dt)
{
  dt = clamp(dt, 1e-3, 0.2);
  integral_ = clamp(integral_ + error * dt, -integral_limit_, integral_limit_);
  const double derivative = initialized_ ? (error - previous_error_) / dt : 0.0;
  previous_error_ = error;
  initialized_ = true;
  return clamp(kp_ * error + ki_ * integral_ + kd_ * derivative,
               -output_limit_,
               output_limit_);
}

StairController::StairController(ros::NodeHandle& node, ros::NodeHandle& private_node)
  : node_(node)
  , private_node_(private_node)
  , action_server_(node_,
                   parameter<std::string>(private_node_, "action_name", "stair_navigation"),
                   boost::bind(&StairController::execute_goal, this, _1),
                   false)
  , tf_listener_(tf_buffer_)
  , map_frame_(parameter<std::string>(private_node_, "map_frame", "map"))
  , base_frame_(parameter<std::string>(private_node_, "base_frame", "base_link"))
  , control_rate_(parameter<double>(private_node_, "control_rate", 30.0))
  , tf_timeout_(parameter<double>(private_node_, "tf_timeout", 0.2))
  , start_position_tolerance_(parameter<double>(private_node_, "start_position_tolerance", 0.20))
  , start_yaw_tolerance_(parameter<double>(private_node_, "start_yaw_tolerance", 0.15))
  , waypoint_position_tolerance_(parameter<double>(private_node_, "waypoint_position_tolerance", 0.12))
  , lookahead_distance_(parameter<double>(private_node_, "lookahead_distance", 0.35))
  , centerline_slowdown_distance_(
        parameter<double>(private_node_, "centerline_slowdown_distance", 0.12))
  , max_centerline_deviation_(
        parameter<double>(private_node_, "max_centerline_deviation", 0.25))
  , min_linear_velocity_(parameter<double>(private_node_, "min_linear_velocity", 0.04))
  , max_linear_velocity_(parameter<double>(private_node_, "max_linear_velocity", 0.45))
  , max_angular_velocity_(parameter<double>(private_node_, "max_angular_velocity", 0.8))
  , heading_stop_threshold_(parameter<double>(private_node_, "heading_stop_threshold", 0.75))
  , segment_timeout_(parameter<double>(private_node_, "segment_timeout", 60.0))
  , obstacle_box_width_(parameter<double>(private_node_, "obstacle_box_width", 0.5))
  , obstacle_box_length_(parameter<double>(private_node_, "obstacle_box_length", 0.6))
  , uphill_obstacle_z_(parameter<double>(private_node_, "uphill_obstacle_z", 0.1))
  , downhill_obstacle_z_(parameter<double>(private_node_, "downhill_obstacle_z", -0.1))
  , stair_direction_(0)
  , front_obstacle_detected_(false)
  , linear_pid_(parameter<double>(private_node_, "linear_kp", 0.8),
                parameter<double>(private_node_, "linear_ki", 0.0),
                parameter<double>(private_node_, "linear_kd", 0.05),
                parameter<double>(private_node_, "linear_integral_limit", 0.5),
                max_linear_velocity_)
  , angular_pid_(parameter<double>(private_node_, "angular_kp", 1.8),
                 parameter<double>(private_node_, "angular_ki", 0.0),
                 parameter<double>(private_node_, "angular_kd", 0.08),
                 parameter<double>(private_node_, "angular_integral_limit", 0.5),
                 max_angular_velocity_)
  , last_feedback_state_(255)
  , last_feedback_progress_()
{
  if (control_rate_ <= 0.0 || tf_timeout_ < 0.0 || start_position_tolerance_ <= 0.0 ||
      start_yaw_tolerance_ <= 0.0 || waypoint_position_tolerance_ <= 0.0 ||
      lookahead_distance_ <= 0.0 || centerline_slowdown_distance_ < 0.0 ||
      start_position_tolerance_ >= max_centerline_deviation_ ||
      max_centerline_deviation_ <= centerline_slowdown_distance_ ||
      min_linear_velocity_ < 0.0 || max_linear_velocity_ <= 0.0 ||
      min_linear_velocity_ > max_linear_velocity_ || max_angular_velocity_ <= 0.0 ||
      obstacle_box_width_ <= 0.0 || obstacle_box_length_ <= 0.0)
  {
    throw std::invalid_argument("invalid stair controller parameter");
  }

  const std::string cmd_vel_topic =
      parameter<std::string>(private_node_, "cmd_vel_topic", "/cmd_vel");
  const std::string cloud_topic =
      parameter<std::string>(private_node_, "cloud_topic", "/pseudo_cloud_base");
  cmd_vel_publisher_ = node_.advertise<geometry_msgs::Twist>(cmd_vel_topic, 1);
  cloud_subscriber_ = node_.subscribe(cloud_topic, 1, &StairController::cloud_callback, this);
  action_server_.start();
  ROS_INFO_STREAM("stair executor uses TF " << map_frame_ << " -> " << base_frame_);
  ROS_INFO_STREAM("stair obstacle stop monitors " << cloud_topic << " in front box "
                                                   << obstacle_box_length_ << " x "
                                                   << obstacle_box_width_ << " m; z thresholds up="
                                                   << uphill_obstacle_z_ << ", down="
                                                   << downhill_obstacle_z_);
}

StairController::~StairController()
{
  publish_stop();
}

bool StairController::lookup_robot_pose(RobotPose& pose, std::string& message)
{
  try
  {
    const geometry_msgs::TransformStamped transform =
        tf_buffer_.lookupTransform(map_frame_, base_frame_, ros::Time(0), ros::Duration(tf_timeout_));
    pose.x = transform.transform.translation.x;
    pose.y = transform.transform.translation.y;
    pose.z = transform.transform.translation.z;
    pose.yaw = tf2::getYaw(transform.transform.rotation);
    if (!std::isfinite(pose.x) || !std::isfinite(pose.y) || !std::isfinite(pose.z) ||
        !std::isfinite(pose.yaw))
    {
      message = "TF contains a non-finite robot pose";
      return false;
    }
    return true;
  }
  catch (const tf2::TransformException& error)
  {
    message = "failed to lookup TF " + map_frame_ + " -> " + base_frame_ + ": " + error.what();
    return false;
  }
}

void StairController::publish_feedback(std::uint8_t state,
                                       std::size_t completed_points,
                                       std::size_t point_count)
{
  floor_msgs::StairNavigationFeedback feedback;
  feedback.state = state;
  feedback.progress = std::to_string(completed_points) + "/" + std::to_string(point_count);
  if (feedback.state == last_feedback_state_ && feedback.progress == last_feedback_progress_)
  {
    return;
  }
  last_feedback_state_ = feedback.state;
  last_feedback_progress_ = feedback.progress;
  action_server_.publishFeedback(feedback);
}

void StairController::cloud_callback(const sensor_msgs::PointCloud2ConstPtr& cloud)
{
  const int direction = stair_direction_.load();
  if (direction == 0)
  {
    front_obstacle_detected_.store(false);
    return;
  }

  bool obstacle_detected = false;
  const double half_width = obstacle_box_width_ * 0.5;
  const double z_threshold = direction > 0 ? uphill_obstacle_z_ : downhill_obstacle_z_;

  try
  {
    sensor_msgs::PointCloud2ConstIterator<float> x_iterator(*cloud, "x");
    sensor_msgs::PointCloud2ConstIterator<float> y_iterator(*cloud, "y");
    sensor_msgs::PointCloud2ConstIterator<float> z_iterator(*cloud, "z");
    for (; x_iterator != x_iterator.end(); ++x_iterator, ++y_iterator, ++z_iterator)
    {
      const double x = *x_iterator;
      const double y = *y_iterator;
      const double z = *z_iterator;
      if (std::isfinite(x) && std::isfinite(y) && std::isfinite(z) &&
          x > 0.0 && x <= obstacle_box_length_ && std::fabs(y) <= half_width &&
          z > z_threshold)
      {
        obstacle_detected = true;
        break;
      }
    }
  }
  catch (const std::runtime_error& error)
  {
    ROS_ERROR_THROTTLE(1.0, "Cannot inspect stair obstacle cloud: %s", error.what());
    return;
  }

  const bool previous = front_obstacle_detected_.exchange(obstacle_detected);
  if (obstacle_detected && !previous)
  {
    ROS_WARN("Stair obstacle detected in front 3D box while going %s; "
             "stopping until it disappears.",
             direction > 0 ? "up" : "down");
  }
  else if (!obstacle_detected && previous)
  {
    ROS_INFO("Stair front box is clear; resuming control.");
  }
}

void StairController::publish_motion_command(const geometry_msgs::Twist& command)
{
  if (front_obstacle_detected_.load())
  {
    publish_stop();
    return;
  }
  cmd_vel_publisher_.publish(command);
}

void StairController::publish_stop()
{
  cmd_vel_publisher_.publish(geometry_msgs::Twist());
}

bool StairController::preempt_requested()
{
  return action_server_.isPreemptRequested() || !ros::ok();
}

bool StairController::align_to_start(const Pose3D& start,
                                     double segment_yaw,
                                     bool align_position,
                                     std::size_t start_index,
                                     std::size_t point_count,
                                     std::string& message)
{
  linear_pid_.reset();
  angular_pid_.reset();
  bool aligning_yaw = false;
  const ros::WallTime alignment_started = ros::WallTime::now();
  ros::WallTime previous_update = alignment_started;
  ros::WallRate rate(control_rate_);

  while (ros::ok())
  {
    if (preempt_requested())
    {
      publish_stop();
      message = "stair execution preempted while aligning segment start";
      return false;
    }
    if (segment_timeout_ > 0.0 &&
        (ros::WallTime::now() - alignment_started).toSec() >= segment_timeout_)
    {
      publish_stop();
      message = "stair start alignment timeout at point " + std::to_string(start_index + 1);
      return false;
    }

    RobotPose robot;
    if (!lookup_robot_pose(robot, message))
    {
      publish_stop();
      return false;
    }

    const ros::WallTime now = ros::WallTime::now();
    const double dt = (now - previous_update).toSec();
    previous_update = now;
    const double position_error = position_distance(robot.x, robot.y, start.x, start.y);
    geometry_msgs::Twist command;

    publish_feedback(floor_msgs::StairNavigationFeedback::ALIGNING,
                     start_index + 1,
                     point_count);

    // 只有整条楼梯路径的第一个入口点需要重新对齐位置。中间点已经由
    // 上一段确认到达，下一段只对齐新方向，避免因轻微越点而掉头追回。
    if (align_position && position_error > start_position_tolerance_)
    {
      aligning_yaw = false;
      const double desired_yaw = std::atan2(start.y - robot.y, start.x - robot.x);
      const double yaw_error = normalize_angle(desired_yaw - robot.yaw);
      double linear_velocity = std::max(0.0, linear_pid_.update(position_error, dt));
      if (linear_velocity > 0.0 && linear_velocity < min_linear_velocity_)
      {
        linear_velocity = min_linear_velocity_;
      }
      linear_velocity *= std::max(0.0, std::cos(yaw_error));
      if (std::fabs(yaw_error) >= heading_stop_threshold_)
      {
        linear_velocity = 0.0;
      }
      command.linear.x = clamp(linear_velocity, 0.0, max_linear_velocity_);
      command.angular.z = angular_pid_.update(yaw_error, dt);
    }
    else
    {
      if (!aligning_yaw)
      {
        linear_pid_.reset();
        angular_pid_.reset();
        aligning_yaw = true;
      }
      const double yaw_error = normalize_angle(segment_yaw - robot.yaw);
      if (std::fabs(yaw_error) <= start_yaw_tolerance_)
      {
        publish_stop();
        message = "aligned stair segment start " + std::to_string(start_index + 1);
        return true;
      }
      command.angular.z = angular_pid_.update(yaw_error, dt);
    }

    publish_motion_command(command);
    rate.sleep();
  }

  publish_stop();
  message = "ROS shutdown during stair start alignment";
  return false;
}

bool StairController::track_segment(const Pose3D& start,
                                    const Pose3D& goal,
                                    std::size_t target_index,
                                    std::size_t point_count,
                                    std::string& message)
{
  const double segment_x = goal.x - start.x;
  const double segment_y = goal.y - start.y;
  const double segment_length = std::hypot(segment_x, segment_y);
  const double unit_x = segment_length > 1e-9 ? segment_x / segment_length : 0.0;
  const double unit_y = segment_length > 1e-9 ? segment_y / segment_length : 0.0;

  if (segment_length <= 1e-9)
  {
    publish_stop();
    publish_feedback(floor_msgs::StairNavigationFeedback::TRACKING,
                     target_index + 1,
                     point_count);
    message = "reached stair route point " + std::to_string(target_index + 1);
    return true;
  }

  linear_pid_.reset();
  angular_pid_.reset();
  const ros::WallTime segment_started = ros::WallTime::now();
  ros::WallTime previous_update = segment_started;
  ros::WallRate rate(control_rate_);

  while (ros::ok())
  {
    if (preempt_requested())
    {
      publish_stop();
      message = "stair execution preempted";
      return false;
    }
    if (segment_timeout_ > 0.0 &&
        (ros::WallTime::now() - segment_started).toSec() >= segment_timeout_)
    {
      publish_stop();
      message = "stair segment timeout at point " + std::to_string(target_index + 1);
      return false;
    }

    RobotPose robot;
    if (!lookup_robot_pose(robot, message))
    {
      publish_stop();
      return false;
    }

    const ros::WallTime now = ros::WallTime::now();
    const double dt = (now - previous_update).toSec();
    previous_update = now;
    const double along_track = (robot.x - start.x) * unit_x +
                               (robot.y - start.y) * unit_y;
    const double cross_track = -(robot.x - start.x) * unit_y +
                               (robot.y - start.y) * unit_x;
    const double absolute_cross_track = std::fabs(cross_track);
    if (absolute_cross_track > max_centerline_deviation_)
    {
      publish_stop();
      message = "centerline deviation " + std::to_string(absolute_cross_track) +
                " exceeds limit " + std::to_string(max_centerline_deviation_) +
                " on segment ending at point " + std::to_string(target_index + 1);
      return false;
    }

    const double goal_distance = position_distance(robot.x, robot.y, goal.x, goal.y);
    const double endpoint_overshoot = along_track - segment_length;
    const bool reached_by_distance = goal_distance <= waypoint_position_tolerance_;
    const bool crossed_endpoint = endpoint_overshoot >= 0.0 &&
                                  endpoint_overshoot <= max_centerline_deviation_;
    if (reached_by_distance || crossed_endpoint)
    {
      publish_stop();
      publish_feedback(floor_msgs::StairNavigationFeedback::TRACKING,
                       target_index + 1,
                       point_count);
      message = "reached stair route point " + std::to_string(target_index + 1);
      return true;
    }

    // 终点已经明显位于机器人身后时禁止继续用它计算航向，否则控制器会
    // 线速度置零并原地掉头。正常情况下会在刚越过终点时由上面的条件结束。
    if (endpoint_overshoot > max_centerline_deviation_)
    {
      publish_stop();
      message = "endpoint overshoot " + std::to_string(endpoint_overshoot) +
                " exceeds limit " + std::to_string(max_centerline_deviation_) +
                " on segment ending at point " + std::to_string(target_index + 1);
      return false;
    }

    geometry_msgs::Twist command;
    const double projection = clamp(along_track, 0.0, segment_length);
    const double lookahead = std::min(segment_length, projection + lookahead_distance_);
    const double target_x = start.x + lookahead * unit_x;
    const double target_y = start.y + lookahead * unit_y;
    const double desired_yaw = std::atan2(target_y - robot.y, target_x - robot.x);
    const double yaw_error = normalize_angle(desired_yaw - robot.yaw);

    double linear_velocity = std::max(0.0, linear_pid_.update(goal_distance, dt));
    if (linear_velocity > 0.0 && linear_velocity < min_linear_velocity_)
    {
      linear_velocity = min_linear_velocity_;
    }
    linear_velocity *= std::max(0.0, std::cos(yaw_error));
    if (absolute_cross_track > centerline_slowdown_distance_)
    {
      const double centerline_scale =
          (max_centerline_deviation_ - absolute_cross_track) /
          (max_centerline_deviation_ - centerline_slowdown_distance_);
      linear_velocity *= clamp(centerline_scale, 0.0, 1.0);
    }
    if (std::fabs(yaw_error) >= heading_stop_threshold_)
    {
      linear_velocity = 0.0;
    }

    command.linear.x = clamp(linear_velocity, 0.0, max_linear_velocity_);
    command.angular.z = angular_pid_.update(yaw_error, dt);
    publish_feedback(floor_msgs::StairNavigationFeedback::TRACKING,
                     target_index,
                     point_count);

    publish_motion_command(command);
    rate.sleep();
  }

  publish_stop();
  message = "ROS shutdown during stair execution";
  return false;
}

void StairController::execute_goal(const floor_msgs::StairNavigationGoalConstPtr& goal)
{
  floor_msgs::StairNavigationResult result;
  // 无论成功、取消、校验失败、TF失败还是异常退出，回调结束时都再次发送零速度。
  StopOnExit stop_on_exit([this]() {
    stair_direction_.store(0);
    front_obstacle_detected_.store(false);
    publish_stop();
  });
  stair_direction_.store(0);
  front_obstacle_detected_.store(false);
  publish_stop();
  last_feedback_state_ = 255;
  last_feedback_progress_.clear();
  if (goal->start_floor == goal->goal_floor)
  {
    result.success = false;
    result.message = "stair route must connect different floors";
    action_server_.setAborted(result, result.message);
    return;
  }
  stair_direction_.store(goal->goal_floor > goal->start_floor ? 1 : -1);
  if (goal->primitives.size() < 2)
  {
    result.success = false;
    result.message = "stair route requires at least two points";
    action_server_.setAborted(result, result.message);
    return;
  }

  std::vector<Pose3D> route;
  route.reserve(goal->primitives.size());
  for (const geometry_msgs::Pose& primitive : goal->primitives)
  {
    const Pose3D point = pose_from_message(primitive);
    if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z))
    {
      result.success = false;
      result.message = "stair route contains a non-finite point";
      action_server_.setAborted(result, result.message);
      return;
    }
    route.push_back(point);
  }

  for (std::size_t target = 1; target < route.size(); ++target)
  {
    if (position_distance(route[target - 1].x,
                          route[target - 1].y,
                          route[target].x,
                          route[target].y) <= 1e-9)
    {
      result.success = false;
      result.message = "consecutive stair route points must be different at point " +
                       std::to_string(target + 1);
      action_server_.setAborted(result, result.message);
      return;
    }
  }

  std::string message;
  for (std::size_t target = 1; target < route.size(); ++target)
  {
    const double segment_yaw = std::atan2(route[target].y - route[target - 1].y,
                                          route[target].x - route[target - 1].x);
    if (!align_to_start(route[target - 1],
                        segment_yaw,
                        target == 1,
                        target - 1,
                        route.size(),
                        message))
    {
      result.success = false;
      result.message = message;
      if (action_server_.isPreemptRequested())
      {
        action_server_.setPreempted(result, message);
      }
      else
      {
        action_server_.setAborted(result, message);
      }
      return;
    }
    if (!track_segment(route[target - 1], route[target], target, route.size(), message))
    {
      result.success = false;
      result.message = message;
      if (action_server_.isPreemptRequested())
      {
        action_server_.setPreempted(result, message);
      }
      else
      {
        action_server_.setAborted(result, message);
      }
      return;
    }
  }

  publish_stop();
  publish_feedback(floor_msgs::StairNavigationFeedback::TRACKING, route.size(), route.size());
  ros::WallDuration(1.0 / control_rate_).sleep();
  result.success = true;
  result.message = "stair route completed";
  action_server_.setSucceeded(result, result.message);
}

}  // namespace multi_floor_navigation
