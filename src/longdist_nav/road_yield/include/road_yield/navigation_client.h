#ifndef ROAD_YIELD_NAVIGATION_CLIENT_H_
#define ROAD_YIELD_NAVIGATION_CLIENT_H_

#include <road_yield/config_loader.h>

#include <actionlib/client/simple_action_client.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <ros/time.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

namespace road_yield
{

enum class GoalOutcome
{
  NONE,
  SUCCEEDED,
  FAILED,
  TIMED_OUT,
};

class NavigationClient
{
public:
  NavigationClient(std::string action_name, std::string planning_frame, double wait_server_timeout);
  ~NavigationClient();

  void sendGoal(const NavigationPose& pose, double timeout);
  void cancelGoal();
  bool active() const;
  GoalOutcome pollOutcome(std::string& detail);

private:
  using MoveBaseClient = actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction>;

  std::string action_name_;
  std::string planning_frame_;
  std::unique_ptr<MoveBaseClient> client_;

  mutable std::mutex mutex_;
  std::uint64_t sequence_{0};
  bool goal_active_{false};
  bool result_ready_{false};
  bool succeeded_{false};
  std::string state_text_;
  ros::Time sent_time_;
  double timeout_{0.0};
};

}  // namespace road_yield

#endif  // ROAD_YIELD_NAVIGATION_CLIENT_H_
