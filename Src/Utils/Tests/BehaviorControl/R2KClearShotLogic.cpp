#include "Tools/BehaviorControl/R2KClearShotLogic.h"

#include "gtest/gtest.h"
#include <cmath>

GTEST_TEST(R2KClearShotLogic, FallsBackToCenterWithoutObstacles)
{
  ObstacleModel obstacleModel;
  RobotPose robotPose;
  FieldDimensions fieldDimensions;
  const Angle angle = R2KClearShotLogic::chooseClearAngleToGoal(obstacleModel, robotPose, fieldDimensions, 30.f);
  EXPECT_NEAR(angle, 0.f, 0.1f);
}

GTEST_TEST(R2KClearShotLogic, ChoosesSideWhenCenterBlocked)
{
  ObstacleModel obstacleModel;
  Obstacle obstacle;
  obstacle.center = Vector2f(1000.f, 0.f);
  obstacleModel.obstacles.push_back(obstacle);

  RobotPose robotPose;
  FieldDimensions fieldDimensions;
  const Angle angle = R2KClearShotLogic::chooseClearAngleToGoal(obstacleModel, robotPose, fieldDimensions, 30.f);
  EXPECT_GT(std::abs(angle), 0.05f);
}
