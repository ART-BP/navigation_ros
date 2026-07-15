#ifndef ROAD_YIELD_POSE_PROVIDER_H_
#define ROAD_YIELD_POSE_PROVIDER_H_

#include <road_yield/road_geometry.h>

#include <geometry_msgs/Point.h>
#include <geometry_msgs/Vector3.h>
#include <ros/time.h>
#include <tf2_ros/buffer.h>

#include <string>

namespace road_yield
{

class PoseProvider
{
public:
  PoseProvider(tf2_ros::Buffer& tf_buffer,
               std::string planning_frame,
               std::string robot_base_frame,
               double timeout);

  bool robotPosition(Point2D& position) const;
  bool transformPoint(const geometry_msgs::Point& input,
                      const std::string& source_frame,
                      const ros::Time& stamp,
                      Point2D& output) const;
  bool transformVector(const geometry_msgs::Vector3& input,
                       const std::string& source_frame,
                       const ros::Time& stamp,
                       Point2D& output) const;

private:
  tf2_ros::Buffer& tf_buffer_;
  std::string planning_frame_;
  std::string robot_base_frame_;
  double timeout_;
};

}  // namespace road_yield

#endif  // ROAD_YIELD_POSE_PROVIDER_H_
