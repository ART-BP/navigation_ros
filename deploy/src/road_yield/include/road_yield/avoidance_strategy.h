#ifndef ROAD_YIELD_AVOIDANCE_STRATEGY_H_
#define ROAD_YIELD_AVOIDANCE_STRATEGY_H_

#include <road_yield/config_loader.h>
#include <road_yield/road_geometry.h>

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

namespace road_yield
{

// Detects a goal that remains active without making a configured amount of
// distance progress. Time is represented as seconds to keep the policy easy to
// unit test without a ROS clock.
class AvoidanceProgressWatchdog
{
public:
  AvoidanceProgressWatchdog(double timeout, double minimum_progress)
    : timeout_(timeout)
    , minimum_progress_(minimum_progress)
  {
  }

  void start(double distance_to_goal, double now)
  {
    active_ = true;
    reference_distance_ = distance_to_goal;
    last_progress_time_ = now;
  }

  bool update(double distance_to_goal, double now)
  {
    if (!active_ || now < last_progress_time_)
    {
      start(distance_to_goal, now);
      return false;
    }

    if (reference_distance_ - distance_to_goal >= minimum_progress_)
    {
      reference_distance_ = distance_to_goal;
      last_progress_time_ = now;
      return false;
    }
    return now - last_progress_time_ >= timeout_;
  }

  // Missing robot pose is not evidence that the goal is blocked. Defer the
  // watchdog until pose feedback is available again.
  void suspend(double now)
  {
    if (active_ && now >= last_progress_time_)
    {
      last_progress_time_ = now;
    }
  }

  void reset()
  {
    active_ = false;
    reference_distance_ = 0.0;
    last_progress_time_ = 0.0;
  }

  bool active() const
  {
    return active_;
  }

private:
  double timeout_;
  double minimum_progress_;
  bool active_{false};
  double reference_distance_{0.0};
  double last_progress_time_{0.0};
};

inline std::vector<std::size_t> nearestEligibleAvoidancePoints(
    const Point2D& robot_position,
    const std::vector<AvoidancePoint>& points,
    std::size_t route_index,
    std::size_t limit)
{
  std::vector<std::pair<double, std::size_t>> ranked;
  ranked.reserve(points.size());
  for (std::size_t i = 0; i < points.size(); ++i)
  {
    const AvoidancePoint& candidate = points[i];
    if (static_cast<int>(route_index) < candidate.route_begin ||
        static_cast<int>(route_index) > candidate.route_end)
    {
      continue;
    }
    ranked.emplace_back(distance(robot_position, candidate.shelter.position), i);
  }

  std::sort(ranked.begin(), ranked.end(),
            [](const std::pair<double, std::size_t>& lhs,
               const std::pair<double, std::size_t>& rhs) {
              if (lhs.first != rhs.first)
              {
                return lhs.first < rhs.first;
              }
              return lhs.second < rhs.second;
            });

  const std::size_t result_size = std::min(limit, ranked.size());
  std::vector<std::size_t> result;
  result.reserve(result_size);
  for (std::size_t i = 0; i < result_size; ++i)
  {
    result.push_back(ranked[i].second);
  }
  return result;
}

}  // namespace road_yield

#endif  // ROAD_YIELD_AVOIDANCE_STRATEGY_H_
