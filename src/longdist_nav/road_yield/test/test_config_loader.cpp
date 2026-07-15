#include <gtest/gtest.h>

#include <road_yield/config_loader.h>

namespace rym = road_yield;

TEST(ConfigLoader, ReadsCurrentThreeCsvFormat)
{
  const rym::ManagerConfig config = rym::loadManagerConfig(TEST_CONFIG_PATH);

  ASSERT_EQ(50u, config.route.size());
  ASSERT_EQ(19u, config.avoidance_points.size());
  ASSERT_EQ(17u, config.road_cross_sections.size());

  // Boundary waypoint_line 35..51 are physical patrol CSV line numbers.
  EXPECT_EQ(33u, config.detection_route_begin);
  EXPECT_EQ(49u, config.detection_route_end);

  EXPECT_NEAR(-38.741147, config.route.front().position.x, 1e-6);
  EXPECT_NEAR(164.657, config.avoidance_points.front().shelter.position.x, 1e-6);
  EXPECT_NEAR(162.306, config.road_cross_sections.front().left.x, 1e-6);
  EXPECT_NEAR(160.175, config.road_cross_sections.front().right.x, 1e-6);
  EXPECT_DOUBLE_EQ(3.0, config.avoidance_wait_time);
}
