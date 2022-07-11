/**
 * @file DefaultPose.cpp
 * @author Jonas Lambing, Adrian Müller
 * @brief 1.1 adjusted to new walk-in positions (rules 2021)
 * @version 1.1
 * @date 2022-08-04
 * 
 */

#include "DefaultPose.h"

Vector2f DefaultPose::getDefaultPosition(const int robotNumber) const
{
  switch(robotNumber)
  {
    case 1:
    return goalieDefaultPosition;
    case 2:
    return leftDefensePosition;
    case 3:
    return rightDefensePosition;
    case 4:
    return rightOffensePosition;
    case 5:
    return leftOffensePosition;
    default:
    return Vector2f::Zero();
  }
}