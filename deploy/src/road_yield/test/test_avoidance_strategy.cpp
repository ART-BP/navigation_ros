#include <gtest/gtest.h>

#include <road_yield/avoidance_strategy.h>

namespace rym = road_yield;

TEST(AvoidanceProgressWatchdog, TimesOutWithoutMinimumProgress)
{
  rym::AvoidanceProgressWatchdog watchdog(8.0, 0.25);
  watchdog.start(5.0, 10.0);

  EXPECT_FALSE(watchdog.update(4.8, 17.9));
  EXPECT_TRUE(watchdog.update(4.8, 18.0));
}

TEST(AvoidanceProgressWatchdog, MinimumProgressRestartsTimeout)
{
  rym::AvoidanceProgressWatchdog watchdog(8.0, 0.25);
  watchdog.start(5.0, 10.0);

  EXPECT_FALSE(watchdog.update(4.74, 17.0));
  EXPECT_FALSE(watchdog.update(4.7, 24.9));
  EXPECT_TRUE(watchdog.update(4.7, 25.0));
}

TEST(AvoidanceProgressWatchdog, MissingPoseSuspendsTimeout)
{
  rym::AvoidanceProgressWatchdog watchdog(8.0, 0.25);
  watchdog.start(5.0, 10.0);
  watchdog.suspend(17.0);

  EXPECT_FALSE(watchdog.update(5.0, 24.9));
  EXPECT_TRUE(watchdog.update(5.0, 25.0));
}

TEST(AvoidanceCandidateSelection, LimitsAndSortsEligiblePoints)
{
  std::vector<rym::AvoidancePoint> points(4);
  points[0].shelter.position = {4.0, 0.0};
  points[1].shelter.position = {1.0, 0.0};
  points[2].shelter.position = {2.0, 0.0};
  points[3].shelter.position = {3.0, 0.0};
  points[1].route_begin = 6;

  const std::vector<std::size_t> selected =
      rym::nearestEligibleAvoidancePoints({0.0, 0.0}, points, 5, 2);

  ASSERT_EQ(2u, selected.size());
  EXPECT_EQ(2u, selected[0]);
  EXPECT_EQ(3u, selected[1]);
}
