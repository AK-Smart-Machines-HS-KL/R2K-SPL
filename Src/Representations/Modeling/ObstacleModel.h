/**
 * @file ObstacleModel.h
 *
 * Declaration of struct ObstacleModel.
 *
 * @author Florian Maaß
 */
#pragma once

#include "Tools/Communication/BHumanTeamMessageParts/BHumanMessageParticle.h"
#include "Tools/Streams/Enum.h"
#include "Tools/Math/Eigen.h"
#include "Tools/Modeling/Obstacle.h"
#include "Representations/Configuration/KickInfo.h"

/**
 * @struct ObstacleModel
 *
 * A struct that represents all kind of obstacles seen by vision, detected by arm contact,
 * foot bumper contact.
 */

STREAMABLE(ObstacleModel, COMMA public BHumanMessageParticle<idObstacleModel>
{
  /** BHumanMessageParticle functions */
  void operator>>(BHumanMessage& m) const override;
  void operator<<(const BHumanMessage& m) override;
  bool opponentIsClose(float min = 1800.0f) const;
  KickInfo::LongShotType opponentIsTooClose(Vector2f ballPosRelative, float min = 1800.0f, float preciseMin = 2300.0f) const;

  ObstacleModel() = default;
  void draw() const;
  void verify() const,

  (std::vector<Obstacle>) obstacles, /**< List of obstacles (position relative to own pose) */
});
