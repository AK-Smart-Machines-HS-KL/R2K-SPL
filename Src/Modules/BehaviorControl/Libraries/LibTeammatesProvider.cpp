/**
 * @file LibTeamProvider.cpp
 * @author Lukas Malte Monerjahn
 */

#include "LibTeammatesProvider.h"

MAKE_MODULE(LibTeammatesProvider, behaviorControl);

void LibTeammatesProvider::update(LibTeammates& libTeammates)
{
  // Pre-calculate thresholds for own penalty area based on this robot's position
  float ownDistanceThreshold = outsideDistanceThreshold;
  bool ownIsNear = false;
  if(theLibPosition.isNearOwnPenaltyArea(theRobotPose.translation, -theRobotPose.getXAxisStandardDeviation(), -theRobotPose.getYAxisStandardDeviation()))
    ownDistanceThreshold = insideDistanceThreshold;
  else if(theLibPosition.isNearOwnPenaltyArea(theRobotPose.translation, outsideDistanceThreshold, outsideDistanceThreshold))
    ownIsNear = true;

  // Pre-calculate threshold for opponent penalty area based on this robot's position
  float oppDistanceThreshold = outsideDistanceThreshold;
  if(theLibPosition.isNearOpponentPenaltyArea(theRobotPose.translation, -theRobotPose.getXAxisStandardDeviation(), -theRobotPose.getYAxisStandardDeviation()))
    oppDistanceThreshold = insideDistanceThreshold;

  int nonKeeperInOwn = 0;
  int inOpponent = 0;

  // Single pass through all teammates instead of two separate loops
  for(const auto& teammate : theTeamData.teammates)
  {
    if(teammate.status == Teammate::PENALIZED)
      continue;

    // Check own penalty area for non-goalkeepers
    if(!teammate.isGoalkeeper
       && theLibPosition.isNearOwnPenaltyArea(teammate.theRobotPose.translation, ownDistanceThreshold, ownDistanceThreshold))
    {
      if(!ownIsNear || theLibPosition.isNearOwnPenaltyArea(teammate.theRobotPose.translation, insideDistanceThreshold, insideDistanceThreshold))
        ++nonKeeperInOwn;
    }

    // Check opponent penalty area
    if(theLibPosition.isNearOpponentPenaltyArea(teammate.theRobotPose.translation, oppDistanceThreshold, oppDistanceThreshold))
      ++inOpponent;
  }

  libTeammates.nonKeeperTeammatesInOwnPenaltyArea = nonKeeperInOwn;
  libTeammates.teammatesInOpponentPenaltyArea = inOpponent;
}
