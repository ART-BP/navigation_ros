#include "multi_floor_navigation/stair_action_client.h"

#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

#include <utility>

namespace multi_floor_navigation
{
namespace
{

geometry_msgs::Pose pose_message(const Pose3D& source)
{
  geometry_msgs::Pose pose;
  pose.position.x = source.x;
  pose.position.y = source.y;
  pose.position.z = source.z;
  tf2::Quaternion orientation;
  orientation.setRPY(0.0, 0.0, source.yaw);
  pose.orientation = tf2::toMsg(orientation);
  return pose;
}

}  // namespace

StairActionClient::StairActionClient(std::string action_name, double server_timeout)
  : client_(std::move(action_name), true), server_timeout_(server_timeout)
{
}

bool StairActionClient::execute(int entry_node_id,
                                int from_floor,
                                int to_floor,
                                const TopologyEdge& edge,
                                const std::function<bool()>& cancel_requested,
                                std::string& message)
{
  if (!client_.isServerConnected() &&
      !client_.waitForServer(ros::Duration(server_timeout_)))
  {
    message = "stair action server is unavailable";
    return false;
  }

  floor_msgs::StairNavigationGoal goal;
  goal.entry_node_id = entry_node_id;
  goal.start_floor = from_floor;
  goal.goal_floor = to_floor;
  goal.primitives.reserve(edge.primitives.size());
  for (const Pose3D& primitive : edge.primitives)
  {
    goal.primitives.push_back(pose_message(primitive));
  }
  client_.sendGoal(goal);

  while (ros::ok())
  {
    if (cancel_requested && cancel_requested())
    {
      client_.cancelGoal();
      message = "stair execution canceled";
      return false;
    }
    if (!client_.waitForResult(ros::Duration(0.1)))
    {
      continue;
    }

    const floor_msgs::StairNavigationResultConstPtr result = client_.getResult();
    if (client_.getState() == actionlib::SimpleClientGoalState::SUCCEEDED && result && result->success)
    {
      message = result->message;
      return true;
    }
    message = result ? result->message : "stair action returned no result";
    return false;
  }

  client_.cancelGoal();
  message = "ROS shutdown while waiting for stair action";
  return false;
}

void StairActionClient::cancel()
{
  client_.cancelAllGoals();
}

}  // namespace multi_floor_navigation
