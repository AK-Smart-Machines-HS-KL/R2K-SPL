/**
 * @file OwnKickInCard.cpp
 * @author Andy Hobelsberger, Adrian Müller (R2K)
 * @brief 
 * @version 1.3
 * @date 2023-05-23
 *   
 * Covers the Free Kick: Own Team has Kick in
 * added (Adrian): pre-condition: triggers for role.playBall, action: WalkToBallAndKickAtGoal
 * V1.1 Card migrated; uses goToBallAndKickAtGoal (Nicholas) 
 * v1.2. (Adrian) Gore hack: first DEFENSE and latest (acc. to roles[] OFFENSE qualify
 * v.3.  (Adrian) wo is doing the kick in differs from online vs offline  (see precondition for details)
 */


#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"

#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamData.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/TeamCommStatus.h"

#include "Representations/Modeling/RobotPose.h"

#include "Tools/Math/Geometry.h"


CARD(OwnKickInCard,
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
  REQUIRES(TeamData),
  REQUIRES(TeammateRoles),
  REQUIRES(TeamCommStatus),  // wifi on off?

  DEFINES_PARAMETERS(
    { ,
      (bool)(false) footIsSelected,  // freeze the first decision
      (bool)(true) leftFoot,
      (Vector2f)(Vector2f(1000.0f, -340.0f)) kickTarget, // Based on 20_deg setup angle in ready card; This is a 20 degree shot
      (int)(5000) ballWasSeenStickyPeriod,  // freeze the first decision
    }),

});

class OwnKickInCard : public OwnKickInCardBase
{

  /**
   * @brief SET_PLAY_KICK_IN: wo is doing the kick in, online/offline differ
   */
  bool preconditions() const override
  {
    
    return
      theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive) // I am the striker
      && theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
      && theGameInfo.setPlay == SET_PLAY_KICK_IN 
      && !aBuddyIsDoingPenaltyKick()
      //  && theTeammateRoles.isTacticalDefense(theRobotInfo.number) // my recent role
      //  && theTeammateRoles.isTacticalOffense(theRobotInfo.number)

      ;

    
 
  }

  /**
   * @brief The condition that needs to be met to exit the this card
   */
  bool postconditions() const override
  {
    /*
    return 
       !theFieldBall.ballWasSeen(ballWasSeenStickyPeriod)
        || theGameInfo.kickingTeam != theOwnTeamInfo.teamNumber
        || theGameInfo.setPlay != SET_PLAY_KICK_IN;
        */
    // added AM
    return !preconditions();
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
  return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
  }
  bool aBuddyIsDoingPenaltyKick() const
    {
      for (const auto& buddy : theTeamData.teammates) 
      {
        if (buddy.theBehaviorStatus.activity == BehaviorStatus::ownFreeKick)
          return true;
      }
      return false;
    }
};

MAKE_CARD(OwnKickInCard);