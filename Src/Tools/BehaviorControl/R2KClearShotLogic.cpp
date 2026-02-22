#include "R2KClearShotLogic.h"

#include "Tools/Math/BHMath.h"

#include <cmath>

namespace R2KClearShotLogic
{
  Angle chooseClearAngleToGoal(const ObstacleModel& obstacleModel,
                               const RobotPose& robotPose,
                               const FieldDimensions& fieldDimensions,
                               const float scanAngleDeg)
  {
    const Vector2f goalCenter(fieldDimensions.xPosOpponentGroundLine, 0.f);
    const Angle centerAngle = (robotPose.inversePose * goalCenter).angle();

    if(obstacleModel.obstacles.empty() || scanAngleDeg <= 0.f)
      return centerAngle;

    const Angle scanHalfAngle = static_cast<Angle>(scanAngleDeg * pi / 180.f * 0.5f);
    const Angle leftCandidate = centerAngle + scanHalfAngle;
    const Angle rightCandidate = centerAngle - scanHalfAngle;

    auto score = [&](const Angle candidate)
    {
      int closeObstacles = 0;
      for(const auto& obstacle : obstacleModel.obstacles)
      {
        const Angle obstacleAngle = obstacle.center.angle();
        const Angle delta = (obstacleAngle - candidate).normalize();
        if(std::abs(delta) < 18_deg && obstacle.center.norm() < 2200.f)
          ++closeObstacles;
      }
      return closeObstacles;
    };

    const int centerScore = score(centerAngle);
    const int leftScore = score(leftCandidate);
    const int rightScore = score(rightCandidate);

    if(leftScore < centerScore && leftScore <= rightScore)
      return leftCandidate;
    if(rightScore < centerScore)
      return rightCandidate;

    return centerAngle;
  }
}
