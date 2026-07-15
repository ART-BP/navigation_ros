#ifndef ROAD_YIELD_STAIR_CONTROLLER_H_
#define ROAD_YIELD_STAIR_CONTROLLER_H_

#include <road_yield/config_loader.h>

#include <dynamic_reconfigure/client.h>
#include <ros/ros.h>
#include <teb_local_planner/TebLocalPlannerReconfigureConfig.h>

#include <atomic>
#include <memory>

namespace road_yield
{

class StairController
{
public:
  StairController(ros::NodeHandle& nh, const ManagerConfig& config);
  ~StairController();

  bool enable();
  void disable();
  bool enabled() const;

private:
  using TebConfig = teb_local_planner::TebLocalPlannerReconfigureConfig;

  void applyConfiguredValues(TebConfig& target, const TebConfig* restore_source) const;
  void publish(bool enabled);
  void timerCallback(const ros::TimerEvent&);

  TebOverrides overrides_;
  double reconfigure_timeout_;
  std::unique_ptr<dynamic_reconfigure::Client<TebConfig>> teb_client_;
  ros::Publisher mode_publisher_;
  ros::Timer publish_timer_;

  std::atomic<bool> enabled_{false};
  bool teb_config_saved_{false};
  TebConfig original_teb_config_;
};

}  // namespace road_yield

#endif  // ROAD_YIELD_STAIR_CONTROLLER_H_
