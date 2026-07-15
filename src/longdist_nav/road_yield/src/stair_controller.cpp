#include <road_yield/stair_controller.h>

#include <std_msgs/Bool.h>

namespace road_yield
{

StairController::StairController(ros::NodeHandle& nh, const ManagerConfig& config)
  : overrides_(config.stair_teb_params)
  , reconfigure_timeout_(config.dynamic_reconfigure_timeout)
  , teb_client_(new dynamic_reconfigure::Client<TebConfig>(config.teb_reconfigure_ns))
{
  mode_publisher_ = nh.advertise<std_msgs::Bool>(config.stair_mode_topic, 1, true);
  publish_timer_ = nh.createTimer(
      ros::Duration(1.0 / config.stair_publish_rate), &StairController::timerCallback, this);
  publish(false);
}

StairController::~StairController()
{
  disable();
}

void StairController::applyConfiguredValues(TebConfig& target,
                                             const TebConfig* restore_source) const
{
  const bool restore = restore_source != nullptr;
  if (overrides_.has_max_vel_x)
    target.max_vel_x = restore ? restore_source->max_vel_x : overrides_.max_vel_x;
  if (overrides_.has_max_vel_theta)
    target.max_vel_theta = restore ? restore_source->max_vel_theta : overrides_.max_vel_theta;
  if (overrides_.has_acc_lim_x)
    target.acc_lim_x = restore ? restore_source->acc_lim_x : overrides_.acc_lim_x;
  if (overrides_.has_acc_lim_theta)
    target.acc_lim_theta = restore ? restore_source->acc_lim_theta : overrides_.acc_lim_theta;
  if (overrides_.has_weight_kinematics_forward_drive)
    target.weight_kinematics_forward_drive =
        restore ? restore_source->weight_kinematics_forward_drive :
                  overrides_.weight_kinematics_forward_drive;
  if (overrides_.has_weight_optimaltime)
    target.weight_optimaltime =
        restore ? restore_source->weight_optimaltime : overrides_.weight_optimaltime;
  if (overrides_.has_weight_viapoint)
    target.weight_viapoint =
        restore ? restore_source->weight_viapoint : overrides_.weight_viapoint;
}

bool StairController::enable()
{
  if (enabled_.load())
  {
    return true;
  }

  TebConfig current;
  if (!overrides_.empty())
  {
    if (!teb_client_->getCurrentConfiguration(current, ros::Duration(reconfigure_timeout_)))
    {
      ROS_ERROR("Timed out while reading current TEB dynamic-reconfigure values");
      return false;
    }
    original_teb_config_ = current;
    applyConfiguredValues(current, nullptr);
    if (!teb_client_->setConfiguration(current))
    {
      ROS_ERROR("Failed to apply stair TEB dynamic-reconfigure values");
      return false;
    }
    teb_config_saved_ = true;
  }

  enabled_.store(true);
  publish(true);
  ROS_INFO("Stair mode enabled");
  return true;
}

void StairController::disable()
{
  enabled_.store(false);
  publish(false);

  if (!teb_config_saved_ || !teb_client_)
  {
    return;
  }

  TebConfig current;
  if (teb_client_->getCurrentConfiguration(current, ros::Duration(reconfigure_timeout_)))
  {
    applyConfiguredValues(current, &original_teb_config_);
    if (!teb_client_->setConfiguration(current))
    {
      ROS_ERROR("Failed to restore original TEB dynamic-reconfigure values");
    }
  }
  else
  {
    ROS_ERROR("Timed out while preparing to restore original TEB values");
  }
  teb_config_saved_ = false;
}

bool StairController::enabled() const
{
  return enabled_.load();
}

void StairController::publish(bool enabled)
{
  if (!mode_publisher_)
  {
    return;
  }
  std_msgs::Bool message;
  message.data = enabled;
  mode_publisher_.publish(message);
}

void StairController::timerCallback(const ros::TimerEvent&)
{
  publish(enabled_.load());
}

}  // namespace road_yield
