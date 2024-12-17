#pragma once

#include "Tools/Module/Module.h"
#include "Representations/BehaviorControl/Shots.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Modeling/ObstacleModel.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Modeling/RobotPose.h"
#include "Tools/Debugging/DebugDrawings.h"
#include "Tools/Math/Sector.h"

struct ObstaclePrediction {
  Vector2f pos;
  float range;
};

MODULE(ShotPredictor,
{,
  REQUIRES(FieldBall),
  REQUIRES(ObstacleModel),
  REQUIRES(FieldDimensions),
  REQUIRES(RobotPose),
  PROVIDES(Shots),
  LOADS_PARAMETERS(
  {,
    (float) ballRadius,
    (KickTypeData) defaultKick,
    (std::vector<KickTypeData>) kicks,
    (Pose2f) maxSpeed, // Copied from WalkingEngine. Temporary until Someone finds out how to read the configs of other Modules
    (float) opponentSpeedModifier,
  }),
});

class ShotPredictor: public ShotPredictorBase
{
private:
  Vector2f goalLeft;
  Vector2f goalRight;

public:

  ShotPredictor();
  /**
   * The update method to generate the head joint angles from desired head motion.
   */
  void update(Shots& shotData) override;

  float estimateTimeToKick(Shot& shot);
  float estimateTimeToAlign(Angle& direction, Shot& shot);

  std::vector<ObstaclePrediction> predictObstacles(float time);

  SectorList getInsterceptSectors(Shot& shot);
  SectorList getMissSectors(Shot& shot, Vector2f& targetLineStartRelative, Vector2f& targetLineEndRelative);
  void calcShotFailProbability(Shot& shot, Vector2f& targetLineStartRelative, Vector2f& targetLineEndRelative);

  Shot getBestThruShot(const Vector2f& lineStart, const Vector2f& lineEnd, const size_t scanFidelity = 25U);

  void draw();
};
