#include <gtest/gtest.h>

#include <road_yield/config_loader.h>

namespace rym = road_yield;

TEST(ConfigLoader, ReadsCurrentThreeCsvFormat)
{
  const rym::ManagerConfig config = rym::loadManagerConfig(TEST_CONFIG_PATH);

  ASSERT_EQ(50u, config.route.size());
  ASSERT_EQ(19u, config.avoidance_points.size());
  ASSERT_EQ(17u, config.road_cross_sections.size());
  EXPECT_TRUE(config.road_yield_enabled);

  // Boundary waypoint_line 35..51 are physical patrol CSV line numbers.
  EXPECT_EQ(33u, config.detection_route_begin);
  EXPECT_EQ(49u, config.detection_route_end);

  EXPECT_NEAR(-38.741147, config.route.front().position.x, 1e-6);
  EXPECT_NEAR(164.657, config.avoidance_points.front().shelter.position.x, 1e-6);
  EXPECT_NEAR(162.306, config.road_cross_sections.front().left.x, 1e-6);
  EXPECT_NEAR(160.175, config.road_cross_sections.front().right.x, 1e-6);
  EXPECT_DOUBLE_EQ(3.0, config.avoidance_wait_time);
  EXPECT_EQ(5, config.avoidance_candidate_limit);
  EXPECT_DOUBLE_EQ(8.0, config.avoidance_progress_timeout);
  EXPECT_DOUBLE_EQ(0.25, config.avoidance_min_progress);
  EXPECT_DOUBLE_EQ(30.0, config.perception_warning_interval);
  EXPECT_EQ(5, config.detection_window_size);
  EXPECT_EQ(3, config.detection_required_count);
  EXPECT_EQ(5, config.clear_required_count);
  EXPECT_DOUBLE_EQ(0.1, config.stationary_speed_threshold);
  EXPECT_DOUBLE_EQ(0.1, config.opposing_speed_threshold);
  for (const rym::NavigationPose& waypoint : config.route)
  {
    EXPECT_FALSE(waypoint.stair);
  }
}

TEST(ConfigLoader, MarksPatrolStairsFromCsvCommandAndYamlIndex)
{
  const rym::ManagerConfig config = rym::loadManagerConfig(TEST_STAIR_CONFIG_PATH);

  ASSERT_EQ(3u, config.route.size());
  EXPECT_FALSE(config.route[0].stair);
  EXPECT_TRUE(config.route[1].stair);
  EXPECT_TRUE(config.route[2].stair);
}

TEST(ConfigLoader, PatrolOnlyConfigurationKeepsStairWaypoints)
{
  const rym::ManagerConfig config = rym::loadManagerConfig(TEST_PATROL_ONLY_CONFIG_PATH);

  EXPECT_FALSE(config.road_yield_enabled);
  EXPECT_TRUE(config.avoidance_points.empty());
  EXPECT_TRUE(config.road_cross_sections.empty());
  ASSERT_EQ(3u, config.route.size());
  EXPECT_FALSE(config.route[0].stair);
  EXPECT_TRUE(config.route[1].stair);
  EXPECT_TRUE(config.route[2].stair);
}

TEST(ConfigLoader, MissingBothRoadYieldFilesSelectsPatrolOnlyMode)
{
  const rym::ManagerConfig config = rym::loadManagerConfig(TEST_MISSING_YIELD_CONFIG_PATH);

  EXPECT_FALSE(config.road_yield_enabled);
  EXPECT_TRUE(config.avoidance_points.empty());
  EXPECT_TRUE(config.road_cross_sections.empty());
}

TEST(ConfigLoader, RejectsOnlyOneAvailableRoadYieldFile)
{
  EXPECT_THROW(rym::loadManagerConfig(TEST_PARTIAL_YIELD_CONFIG_PATH), std::runtime_error);
}
