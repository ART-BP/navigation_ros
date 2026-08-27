#pragma once

#include <actionlib/server/simple_action_server.h>
#include <floor_msgs/StairNavigationAction.h>
#include <geometry_msgs/Twist.h>
#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "multi_floor_navigation/topology_struct.h"

namespace multi_floor_navigation
{

class PidController
{
public:
  PidController(double kp, double ki, double kd, double integral_limit, double output_limit);

  void reset();
  double update(double error, double dt);

private:
  double kp_;
  double ki_;
  double kd_;
  double integral_limit_;
  double output_limit_;
  double integral_;
  double previous_error_;
  bool initialized_;
};

class StairController
{
public:
  StairController(ros::NodeHandle& node, ros::NodeHandle& private_node);
  ~StairController();

private:
  using ActionServer = actionlib::SimpleActionServer<floor_msgs::StairNavigationAction>;

  struct RobotPose
  {
    double x;
    double y;
    double z;
    double yaw;
  };

  void execute_goal(const floor_msgs::StairNavigationGoalConstPtr& goal);
  bool lookup_robot_pose(RobotPose& pose, std::string& message);
  bool align_to_start(const Pose3D& start,
                      double segment_yaw,
                      bool align_position,
                      std::size_t start_index,
                      std::size_t point_count,
                      std::string& message);
  bool track_segment(const Pose3D& start,
                     const Pose3D& goal,
                     std::size_t target_index,
                     std::size_t point_count,
                     std::string& message);
  void publish_feedback(std::uint8_t state, std::size_t completed_points, std::size_t point_count);
  void cloud_callback(const sensor_msgs::PointCloud2ConstPtr& cloud);
  void publish_motion_command(const geometry_msgs::Twist& command);
  void publish_stop();
  bool preempt_requested();

  ros::NodeHandle node_;
  ros::NodeHandle private_node_;
  ActionServer action_server_;
  ros::Publisher cmd_vel_publisher_;
  ros::Subscriber cloud_subscriber_;
  tf2_ros::Buffer tf_buffer_;
  tf2_ros::TransformListener tf_listener_;

  std::string map_frame_;
  std::string base_frame_;
  double control_rate_;
  double tf_timeout_;
  double start_position_tolerance_;
  double start_yaw_tolerance_;
  double waypoint_position_tolerance_;
  double lookahead_distance_;
  double centerline_slowdown_distance_;
  double max_centerline_deviation_;
  double min_linear_velocity_;
  double max_linear_velocity_;
  double max_angular_velocity_;
  double heading_stop_threshold_;
  double segment_timeout_;
  double obstacle_box_width_;
  double obstacle_box_length_;
  double uphill_obstacle_z_;
  double downhill_obstacle_z_;
  // 1表示上楼，-1表示下楼，0表示当前没有楼梯任务。
  std::atomic<int> stair_direction_;
  std::atomic<bool> front_obstacle_detected_;

  PidController linear_pid_;
  PidController angular_pid_;
  std::uint8_t last_feedback_state_;
  std::string last_feedback_progress_;
};

}  // namespace multi_floor_navigation
