/**
 * @file OwnCornerKickCard.cpp
 * @author Andy Hobelsberger, Adrian Müller (R2K)
 * @brief Covers the Free Kick: Own Team has Corner Kick
 * @version 1.1
 * @date 2022-11-22
 * 
 * Notes: 
 *  added (Adrian): pre-condition: triggers for role.playBall, action: WalkToBallAndKickAtGoal
 * 
 * V1.1 Card migrated (Nicholas)
 * v1.2.added functionality to OwnCornerKick: OFFENSE goes to ball and kick to goal" (Adrian)
 * v1.3 Added online and offline role assignment(Asrar)
 * v1.4 (Asrar) card is  for  ballWasSeenStickyPeriod (5000msec), i.e., bot assumes ball to be at the last-seen position
 *                 Applied this parameter by changing the postcondition().
 */

#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"

#include "Representations/Configuration/FieldDimensions.h"

#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Communication/RobotInfo.h"

#include "Representations/Modeling/RobotPose.h"
#include "Representations/Communication/TeamCommStatus.h"

#include "Tools/Math/Geometry.h"


CARD(OwnCornerKickCard,
{,
  CALLS(Stand),
  CALLS(Activity),
  CALLS(LookForward),
  CALLS(GoToBallAndKick),

  REQUIRES(FieldBall),
  REQUIRES(RobotPose),
  REQUIRES(RobotInfo),
  REQUIRES(FieldDimensions),
  REQUIRES(OwnTeamInfo),
  REQUIRES(GameInfo),
  REQUIRES(TeamBehaviorStatus),
  REQUIRES(TeammateRoles),
  REQUIRES(TeamCommStatus),  // wifi on off?

  DEFINES_PARAMETERS(
    {,
      (bool)(false) footIsSelected,  // freeze the first decision
      (bool)(true) leftFoot,
      (Vector2f)(Vector2f(1000.0f, -340.0f)) kickTarget, // Based on 20_deg setup angle in ready card; This is a 20 degree shot
      (int)(5000) ballWasSeenStickyPeriod,  // freeze the first decision
    }),
});

class OwnCornerKickCard : public OwnCornerKickCardBase
{

  /**
   * @brief The condition that needs to be met to execute this card
   */
  bool preconditions() const override
  {
   
    
    return  theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive)  // I am the striker
      && theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
      && theGameInfo.setPlay == SET_PLAY_CORNER_KICK
      && theTeammateRoles.isTacticalOffense(theRobotInfo.number); // My recent role
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
      || theGameInfo.setPlay != SET_PLAY_CORNER_KICK;
  }

  void execute() override
  {
    theActivitySkill(BehaviorStatus::ownFreeKick);
    if (!footIsSelected) {  // select only once
      footIsSelected = true;
      leftFoot = theFieldBall.positionRelative.y() < 0;
    }
    KickInfo::KickType kickType = leftFoot ? KickInfo::forwardFastLeft : KickInfo::forwardFastRight;
    theGoToBallAndKickSkill(calcAngleToGoal(), kickType, true);
  }
  Angle calcAngleToGoal() const
  {
    return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGoalArea, 0.f)).angle();
  }
};

MAKE_CARD(OwnCornerKickCard);