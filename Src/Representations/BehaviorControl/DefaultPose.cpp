/**
 * @file DefaultPose.cpp
 * @author Jonas Lambing, Adrian Müller, Andy Hobelsberger
 * @brief 1.1 adjusted to new walk-in positions (rules 2021)
 * @version 1.1
 * @date 2022-08-04
 * 
 */

#include "DefaultPose.h"
#include "Tools/Debugging/DebugDrawings.h"

Pose2f DefaultPose::getDefaultPosition(const int robotNumber) const
{
  return teamDefaultPoses[robotNumber - 1];
}

void DefaultPose::draw() const
{
  DECLARE_DEBUG_DRAWING("representation:DefaultPose:own", "drawingOnField");
  DECLARE_DEBUG_DRAWING("representation:DefaultPose:all", "drawingOnField");

  int i = 1;
  for(auto pose : teamDefaultPoses)
  {
    CIRCLE("representation:DefaultPose:all", pose.translation.x(), pose.translation.y(), 75, 5, Drawings::solidPen, ColorRGBA::red, Drawings::solidPen, ColorRGBA::red);
    DRAW_TEXT("representation:DefaultPose:all", pose.translation.x(), pose.translation.y(), 100, ColorRGBA::black, i);
    i++;
  }

  CIRCLE("representation:DefaultPose:own", ownDefaultPose.translation.x(), ownDefaultPose.translation.y(), 75, 5, Drawings::solidPen, ColorRGBA::green, Drawings::solidPen, ColorRGBA::green);
}