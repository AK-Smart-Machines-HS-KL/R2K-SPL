/**
 * @file OwnPushingFreeKickCard.cpp
 * @author Andy Hobelsberger
 * @brief Covers the Free Kick: Own Team has Pushing Free Kick
 * @version 1.1
 * @date 2022-11-22
 *   
 * Notes: 
 *  
 * 
 * V1.1 Card migrated (Nicholas)
 * V1.2 Added the online offline role assignment (Asrar)
 * v1.3 (Asrar) card is  for  ballWasSeenStickyPeriod (5000msec), i.e., bot assumes ball to be at the last-seen position
 *                 Applied this parameter by changing the postcondition().
 */

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Communication/RobotInfo.h"

#include "Tools/Math/Geometry.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/Communication/TeamCommStatus.h"

CARD(OwnPushingFreeKickCard,
  { ,
    CALLS(Stand),
    CALLS(Activity),
    CALLS(LookForward),
    CALLS(GoToBallAndKick),
    REQUIRES(OwnTeamInfo),
    REQUIRES(GameInfo),
    REQUIRES(FieldBall),
    REQUIRES(RobotPose),
    REQUIRES(RobotInfo),
    REQUIRES(TeammateRoles),
    REQUIRES(FieldDimensions),
    REQUIRES(TeamCommStatus),  // wifi on off?

    DEFINES_PARAMETERS(
    {,
      (bool)(false) footIsSelected,  // freeze the first decision
      (bool)(true) leftFoot,
      (Vector2f)(Vector2f(1000.0f, -340.0f)) kickTarget, // Based on 20_deg setup angle in ready card; This is a 20 degree shot
      (int)(5000) ballWasSeenStickyPeriod,  // freeze the first decision
    }),
});

class OwnPushingFreeKickCard : public OwnPushingFreeKickCardBase
{
  /**
   * @brief The condition that needs to be met to execute this card
   */
  bool preconditions() const override
  {
    

    
    return 
      theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
      && theGameInfo.setPlay == SET_PLAY_PUSHING_FREE_KICK
      && theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive);  // I am the striker


  }

  /**
   * @brief The condition that needs to be met to exit the this card
   */
  bool postconditions() const override
  {
    return 
      !theFieldBall.ballWasSeen(ballWasSeenStickyPeriod)
      ||
      theGameInfo.kickingTeam != theOwnTeamInfo.teamNumber
      || theGameInfo.setPlay != SET_PLAY_PUSHING_FREE_KICK;
  }

  void execute() override
  {
    theActivitySkill(BehaviorStatus::ownFreeKick);
    if (!footIsSelected) {  // select only once
      footIsSelected = true;
      leftFoot = theFieldBall.positionRelative.y() < 0;
    }
    KickInfo::KickType kickType = leftFoot ? KickInfo::forwardFastLeftLong : KickInfo::forwardFastRightLong;
    theGoToBallAndKickSkill(calcAngleToGoal(), kickType, true);
  }
  Angle calcAngleToGoal() const
  {
  return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
  }
};

MAKE_CARD(OwnPushingFreeKickCard);
