#include <gtest/gtest.h>

#include <road_yield/detection_window.h>

namespace rym = road_yield;

TEST(DetectionWindow, RequiresThreePositiveSamplesAmongFive)
{
  rym::DetectionWindow window(5, 3);
  EXPECT_FALSE(window.add(true));
  EXPECT_FALSE(window.add(false));
  EXPECT_FALSE(window.add(true));
  EXPECT_FALSE(window.add(false));
  EXPECT_TRUE(window.add(true));
}

TEST(DetectionWindow, UsesOnlyLatestFiveSamples)
{
  rym::DetectionWindow window(5, 3);
  EXPECT_FALSE(window.add(true));
  EXPECT_FALSE(window.add(true));
  EXPECT_FALSE(window.add(true));
  EXPECT_FALSE(window.add(false));
  EXPECT_TRUE(window.add(false));
  EXPECT_FALSE(window.add(false));
  EXPECT_EQ(2u, window.positiveCount());
}

TEST(DetectionWindow, ResetRequiresANewCompleteWindow)
{
  rym::DetectionWindow window(5, 3);
  window.add(true);
  window.add(true);
  window.add(true);
  window.add(false);
  EXPECT_TRUE(window.add(false));
  window.reset();
  EXPECT_FALSE(window.confirmed());
  EXPECT_EQ(0u, window.size());
}

TEST(SingleMessageOncoming, AcceptsOpposingAndStationaryVehicles)
{
  EXPECT_TRUE(rym::isSingleMessageOncoming(
      false, true, -0.2, 0.0, 1.0, 0.0, 0.1, 0.1));
  EXPECT_TRUE(rym::isSingleMessageOncoming(
      false, true, 0.05, 0.02, 1.0, 0.0, 0.1, 0.1));
  EXPECT_TRUE(rym::isSingleMessageOncoming(
      true, false, 0.0, 0.0, 1.0, 0.0, 0.1, 0.1));
}

TEST(SingleMessageOncoming, RejectsSameDirectionAndCrossTraffic)
{
  EXPECT_FALSE(rym::isSingleMessageOncoming(
      false, true, 0.2, 0.0, 1.0, 0.0, 0.1, 0.1));
  EXPECT_FALSE(rym::isSingleMessageOncoming(
      false, true, 0.0, 0.2, 1.0, 0.0, 0.1, 0.1));
}

TEST(SingleMessageOncoming, AcceptsVehicleWithoutVelocity)
{
  EXPECT_TRUE(rym::isSingleMessageOncoming(
      false, false, 0.0, 0.0, 1.0, 0.0, 0.1, 0.1));
}
