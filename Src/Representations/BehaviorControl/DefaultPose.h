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
#include "Tools/Math/Pose2f.h"
#include "Tools/Streams/AutoStreamable.h"
#include "Tools/Settings.h"

STREAMABLE(DefaultPose,
{
  /**
   * @brief This function returns the default position based on the robotnumber in absolute coordinates 
   * 
   */
  Pose2f getDefaultPosition(const int robotNumber) const,

  (bool)(true)                isOnDefaultPosition,
  (bool)(true)                isOnDefaultPose,
  (Pose2f)(Vector2f::Zero())  ownDefaultPose,
  (std::vector<Pose2f>) (std::vector<Pose2f>(Settings::highestValidPlayerNumber)) teamDefaultPoses, // Default poses of the team
});