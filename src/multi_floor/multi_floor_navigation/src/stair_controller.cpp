#include "multi_floor_navigation/stair_controller.h"

#include <tf2/utils.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

#include <boost/bind.hpp>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

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
      max_centerline_deviation_ <= centerline_slowdown_distance_ ||
      min_linear_velocity_ < 0.0 || max_linear_velocity_ <= 0.0 ||
      min_linear_velocity_ > max_linear_velocity_ || max_angular_velocity_ <= 0.0)
  {
    throw std::invalid_argument("invalid stair controller parameter");
  }

  const std::string cmd_vel_topic =
      parameter<std::string>(private_node_, "cmd_vel_topic", "/cmd_vel");
  cmd_vel_publisher_ = node_.advertise<geometry_msgs::Twist>(cmd_vel_topic, 1);
  action_server_.start();
  ROS_INFO_STREAM("stair executor uses TF " << map_frame_ << " -> " << base_frame_);
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

    if (position_error > start_position_tolerance_)
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

    cmd_vel_publisher_.publish(command);
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
    const double goal_distance = position_distance(robot.x, robot.y, goal.x, goal.y);
    geometry_msgs::Twist command;

    if (goal_distance <= waypoint_position_tolerance_)
    {
      publish_stop();
      publish_feedback(floor_msgs::StairNavigationFeedback::TRACKING,
                       target_index + 1,
                       point_count);
      message = "reached stair route point " + std::to_string(target_index + 1);
      return true;
    }

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

    cmd_vel_publisher_.publish(command);
    rate.sleep();
  }

  publish_stop();
  message = "ROS shutdown during stair execution";
  return false;
}

void StairController::execute_goal(const floor_msgs::StairNavigationGoalConstPtr& goal)
{
  floor_msgs::StairNavigationResult result;
  publish_stop();
  last_feedback_state_ = 255;
  last_feedback_progress_.clear();
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
    route.push_back(pose_from_message(primitive));
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
