#include <road_yield/pose_provider.h>

#include <geometry_msgs/PointStamped.h>
#include <geometry_msgs/Vector3Stamped.h>
#include <ros/ros.h>
#include <tf2/exceptions.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

#include <cmath>
#include <utility>

namespace road_yield
{
namespace
{

std::string normalizeFrame(const std::string& frame)
{
  return !frame.empty() && frame.front() == '/' ? frame.substr(1) : frame;
}

}  // namespace

PoseProvider::PoseProvider(tf2_ros::Buffer& tf_buffer,
                           std::string planning_frame,
                           std::string robot_base_frame,
                           double timeout)
  : tf_buffer_(tf_buffer)
  , planning_frame_(normalizeFrame(planning_frame))
  , robot_base_frame_(normalizeFrame(robot_base_frame))
  , timeout_(timeout)
{
}

bool PoseProvider::robotPosition(Point2D& position) const
{
  try
  {
    const geometry_msgs::TransformStamped transform = tf_buffer_.lookupTransform(
        planning_frame_, robot_base_frame_, ros::Time(0), ros::Duration(timeout_));
    position.x = transform.transform.translation.x;
    position.y = transform.transform.translation.y;
    return std::isfinite(position.x) && std::isfinite(position.y);
  }
  catch (const tf2::TransformException& exception)
  {
    ROS_WARN_THROTTLE(2.0, "Cannot get robot pose %s -> %s: %s",
                      planning_frame_.c_str(), robot_base_frame_.c_str(), exception.what());
    return false;
  }
}

bool PoseProvider::transformPoint(const geometry_msgs::Point& input,
                                  const std::string& source_frame,
                                  const ros::Time& stamp,
                                  Point2D& output) const
{
  if (!std::isfinite(input.x) || !std::isfinite(input.y))
  {
    return false;
  }
  if (normalizeFrame(source_frame) == planning_frame_)
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
    tf_buffer_.transform(source, transformed, planning_frame_, ros::Duration(timeout_));
    output.x = transformed.point.x;
    output.y = transformed.point.y;
    return std::isfinite(output.x) && std::isfinite(output.y);
  }
  catch (const tf2::TransformException& exception)
  {
    ROS_WARN_THROTTLE(2.0, "Cannot transform tracked object from %s to %s: %s",
                      source_frame.c_str(), planning_frame_.c_str(), exception.what());
    return false;
  }
}

bool PoseProvider::transformVector(const geometry_msgs::Vector3& input,
                                   const std::string& source_frame,
                                   const ros::Time& stamp,
                                   Point2D& output) const
{
  if (!std::isfinite(input.x) || !std::isfinite(input.y))
  {
    return false;
  }
  if (normalizeFrame(source_frame) == planning_frame_)
  {
    output.x = input.x;
    output.y = input.y;
    return true;
  }

  geometry_msgs::Vector3Stamped source;
  source.header.frame_id = source_frame;
  source.header.stamp = stamp;
  source.vector = input;
  geometry_msgs::Vector3Stamped transformed;
  try
  {
    tf_buffer_.transform(source, transformed, planning_frame_, ros::Duration(timeout_));
    output.x = transformed.vector.x;
    output.y = transformed.vector.y;
    return std::isfinite(output.x) && std::isfinite(output.y);
  }
  catch (const tf2::TransformException& exception)
  {
    ROS_WARN_THROTTLE(2.0, "Cannot transform tracked-object velocity from %s to %s: %s",
                      source_frame.c_str(), planning_frame_.c_str(), exception.what());
    return false;
  }
}

}  // namespace road_yield
