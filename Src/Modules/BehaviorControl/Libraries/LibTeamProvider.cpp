/**
 * @file LibTeamProvider.cpp
 */

#include <cmath>
#include <limits>
#include "LibTeamProvider.h"

MAKE_MODULE(LibTeamProvider, behaviorControl);

void LibTeamProvider::update(LibTeam& libTeam)
{
  // Merged pass 1: compute keeper and striker data in a single loop
  int keeperNum = theTeamBehaviorStatus.role.isGoalkeeper() ? theRobotInfo.number : -1;
  Pose2f keeperPose = (keeperNum != -1) ? static_cast<const Pose2f&>(theRobotPose) : Pose2f(0.f, 1000000.f, 1000000.f);
  int strikerNum = theTeamBehaviorStatus.role.playsTheBall() ? theRobotInfo.number : -1;
  Pose2f strikerPose = (strikerNum != -1) ? static_cast<const Pose2f&>(theRobotPose) : Pose2f(0.f, 1000000.f, 1000000.f);

  if(keeperNum == -1 || strikerNum == -1)
  {
    for(const auto& teammate : theTeamData.teammates)
    {
      if(teammate.status != Teammate::PENALIZED)
      {
        if(keeperNum == -1)
        {
          keeperNum = teammate.number;
          keeperPose = teammate.theRobotPose;
        }
        if(strikerNum == -1)
        {
          strikerNum = teammate.number;
          strikerPose = teammate.theRobotPose;
        }
        if(keeperNum != -1 && strikerNum != -1)
          break;
      }
    }
  }

  libTeam.keeperPlayerNumber = keeperNum;
  libTeam.strikerPlayerNumber = strikerNum;
  libTeam.keeperPose = keeperPose;
  libTeam.strikerPose = strikerPose;
  libTeam.numberOfBallPlayingTeammate = strikerNum;

  // Merged pass 2: compute iAmClosestToBall and minTeammateDistanceToBall together
  const Vector2f ballPositionOnField(theFieldBall.recentBallPositionOnField(3000));
  const float selfDistanceSq = (theRobotPose.translation - ballPositionOnField).squaredNorm();
  bool iAmClosest = true;
  float minTeammateDistSq = std::numeric_limits<float>::max();

  for(const Teammate& teammate : theTeamData.teammates)
  {
    if(teammate.status != Teammate::PENALIZED)
    {
      const float distSq = (teammate.theRobotPose.translation - ballPositionOnField).squaredNorm();
      if(distSq < selfDistanceSq)
        iAmClosest = false;
      if(distSq < minTeammateDistSq)
        minTeammateDistSq = distSq;
    }
  }

  libTeam.iAmClosestToBall = iAmClosest;
  libTeam.minTeammateDistanceToBall = std::sqrt(minTeammateDistSq);

  libTeam.numberOfNonKeeperTeammateInOwnGoalArea = numberOfNonKeeperTeammateInOwnGoalArea();

  libTeam.getTeammatePose = [this](int player)->Pose2f
  {
    return getTeammatePose(player);
  };
  libTeam.getActivity = [this](int player)->BehaviorStatus::Activity
  {
    return getActivity(player);
  };
  libTeam.getStatus = [this](int player)->Teammate::Status
  {
    return getStatus(player);
  };
  libTeam.getBallPosition = [this](int player)->Vector2f
  {
    return getBallPosition(player);
  };
  libTeam.getTimeToReachBall = [this](int player)->const TimeToReachBall*
  {
    return getTimeToReachBall(player);
  };
}

Pose2f LibTeamProvider::getTeammatePose(int player) const
{
  if(player == theRobotInfo.number)
    return theRobotPose;
  for(auto const& teammate : theTeamData.teammates)
  {
    if(teammate.number == player)
    {
      if(teammate.status != Teammate::PENALIZED)
        return teammate.theRobotPose;
      else
        break;
    }
  }
  return Pose2f(0.f, 1000000.f, 1000000.f);
}

BehaviorStatus::Activity LibTeamProvider::getActivity(int player) const
{
  if(player == theRobotInfo.number)
    return theBehaviorStatus.activity;
  for(auto const& teammate : theTeamData.teammates)
  {
    if(teammate.number == player)
    {
      if(teammate.status != Teammate::PENALIZED)
        return teammate.theBehaviorStatus.activity;
      else
        break;
    }
  }
  return BehaviorStatus::unknown;
}

Teammate::Status LibTeamProvider::getStatus(int player) const
{
  if(player == theRobotInfo.number)
    return Teammate::PLAYING;
  for(auto const& teammate : theTeamData.teammates)
  {
    if(teammate.number == player)
    {
      return teammate.status;
    }
  }
  return Teammate::PENALIZED;
}

Vector2f LibTeamProvider::getBallPosition(int player) const
{
  if(player == theRobotInfo.number)
    return theBallModel.estimate.position;
  for(auto const& teammate : theTeamData.teammates)
  {
    if(teammate.number == player)
    {
      return teammate.theBallModel.estimate.position;
    }
  }
  return Vector2f::Zero();
}

const TimeToReachBall* LibTeamProvider::getTimeToReachBall(int player) const
{
  if(player == theRobotInfo.number)
    return &(theTeamBehaviorStatus.timeToReachBall);

  return nullptr;
}

int LibTeamProvider::numberOfNonKeeperTeammateInOwnGoalArea(const float distanceThreshold) const
{
  for(auto const& teammate : theTeamData.teammates)
  {
    if(teammate.status != Teammate::PENALIZED
       && theLibPosition.isNearOwnGoalArea(teammate.theRobotPose.translation, distanceThreshold, distanceThreshold))
    {
      return teammate.number;
    }
  }
  return -1;
}
