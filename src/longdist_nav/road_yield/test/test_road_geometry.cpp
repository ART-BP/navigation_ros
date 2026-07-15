#include <gtest/gtest.h>

#include <road_yield/road_geometry.h>

namespace rym = road_yield;

TEST(RoadGeometry, DetectsOnlyInsideAndAhead)
{
  const rym::RoadGeometry road({
      {{0.0, 2.0}, {0.0, -2.0}},
      {{10.0, 2.0}, {10.0, -2.0}},
      {{20.0, 2.0}, {20.0, -2.0}},
  });

  const auto robot = road.projectToCenterline({8.0, 0.0});
  ASSERT_TRUE(robot.valid);
  EXPECT_TRUE(road.isAheadInside({12.0, 1.0}, robot.station, 0.5, 20.0));
  EXPECT_FALSE(road.isAheadInside({6.0, 1.0}, robot.station, 0.5, 20.0));
  EXPECT_FALSE(road.isAheadInside({12.0, 3.0}, robot.station, 0.5, 20.0));
  EXPECT_FALSE(road.isAheadInside({19.0, 0.0}, robot.station, 0.5, 5.0));
}

TEST(RoadGeometry, SupportsCurvedStrip)
{
  const rym::RoadGeometry road({
      {{0.0, 2.0}, {0.0, -2.0}},
      {{10.0, 2.0}, {10.0, -2.0}},
      {{12.0, 10.0}, {8.0, 10.0}},
  });

  rym::RoadProjection projection;
  EXPECT_TRUE(road.contains({10.0, 6.0}, &projection));
  EXPECT_TRUE(projection.valid);
  EXPECT_GT(projection.station, 10.0);
  EXPECT_FALSE(road.contains({14.0, 6.0}));
}
