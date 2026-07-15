#ifndef ROAD_YIELD_DETECTION_WINDOW_H_
#define ROAD_YIELD_DETECTION_WINDOW_H_

#include <cmath>
#include <cstddef>
#include <deque>

namespace road_yield
{

inline bool isSingleMessageOncoming(bool reported_stationary,
                                    bool has_linear_velocity,
                                    double velocity_x,
                                    double velocity_y,
                                    double travel_unit_x,
                                    double travel_unit_y,
                                    double stationary_speed_threshold,
                                    double opposing_speed_threshold)
{
  if (reported_stationary)
  {
    return true;
  }
  if (!has_linear_velocity)
  {
    return true;
  }

  const double speed = std::hypot(velocity_x, velocity_y);
  const double longitudinal_speed =
      velocity_x * travel_unit_x + velocity_y * travel_unit_y;
  return speed <= stationary_speed_threshold ||
         longitudinal_speed <= -opposing_speed_threshold;
}

class DetectionWindow
{
public:
  DetectionWindow(std::size_t window_size, std::size_t required_positive)
    : window_size_(window_size)
    , required_positive_(required_positive)
  {
  }

  bool add(bool positive)
  {
    samples_.push_back(positive);
    if (positive)
    {
      ++positive_count_;
    }
    if (samples_.size() > window_size_)
    {
      if (samples_.front())
      {
        --positive_count_;
      }
      samples_.pop_front();
    }
    return confirmed();
  }

  bool confirmed() const
  {
    return samples_.size() == window_size_ && positive_count_ >= required_positive_;
  }

  void reset()
  {
    samples_.clear();
    positive_count_ = 0;
  }

  std::size_t size() const
  {
    return samples_.size();
  }

  std::size_t positiveCount() const
  {
    return positive_count_;
  }

private:
  std::size_t window_size_;
  std::size_t required_positive_;
  std::deque<bool> samples_;
  std::size_t positive_count_{0};
};

}  // namespace road_yield

#endif  // ROAD_YIELD_DETECTION_WINDOW_H_
