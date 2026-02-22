#pragma once

#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Modeling/ObstacleModel.h"
#include "Representations/Modeling/RobotPose.h"

namespace R2KClearShotLogic
{
  Angle chooseClearAngleToGoal(const ObstacleModel& obstacleModel,
                               const RobotPose& robotPose,
                               const FieldDimensions& fieldDimensions,
                               float scanAngleDeg);
}

