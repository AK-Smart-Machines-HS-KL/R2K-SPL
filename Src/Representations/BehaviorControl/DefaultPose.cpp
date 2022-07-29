/**
 * @file DefaultPose.cpp
 * @author Jonas Lambing, Adrian Müller
 * @brief 1.1 adjusted to new walk-in positions (rules 2021)
 * @version 1.1
 * @date 2022-08-04
 * 
 */

#include "DefaultPose.h"

Pose2f DefaultPose::getDefaultPosition(const int robotNumber) const
{
  return teamDefaultPoses[robotNumber - 1];
}