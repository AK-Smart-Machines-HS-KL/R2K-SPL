/**
 * @file DefaultPose.h
 * @author Jonas Lambing (R2K)
 * @brief Provides default positions on the field based on robotnumbers
 * @version 1.0
 * @date 2021-07-30
 * 
 * This representation is managed (loading cfg and updated) by the defaultPoseProvider.
 * Check DefaultPoseProvider.h for more information.
 * 
 */

#pragma once
#include "Tools/Math/Eigen.h"
#include "Tools/Streams/AutoStreamable.h"

STREAMABLE(DefaultPose,
{
  /**
   * @brief This function returns the default position based on the robotnumber in absolute coordinates 
   * 
   */
  Vector2f getDefaultPosition(const int robotNumber) const,

  (bool)(true)                isOnDefaultPosition,
  (bool)(true)                isOnDefaultPose,
  (Vector2f)(Vector2f::Zero()) defaultPosition,
  (Vector2f)(Vector2f::Zero()) goalieDefaultPosition,
  (Vector2f)(Vector2f::Zero()) leftDefensePosition,
  (Vector2f)(Vector2f::Zero()) rightDefensePosition,
  (Vector2f)(Vector2f::Zero()) leftOffensePosition,
  (Vector2f)(Vector2f::Zero()) rightOffensePosition,
});