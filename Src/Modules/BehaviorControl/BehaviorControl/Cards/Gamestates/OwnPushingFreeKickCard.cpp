/**
 * @file OwnPushingFreeKickCard.cpp
 * @author Andy Hobelsberger
 * @brief Covers the Free Kick: Own Team has Pushing Free Kick
 * @version 1.1
 * @date 2022-11-22
 *   
 * Notes: 
 *  - Currently Triggers for all Robots. Use this Card as a template for preconditions
 *  - Currently only calls stand  
 * 
 * V1.1 Card migrated (Nicholas)
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

    DEFINES_PARAMETERS(
    {,
      (bool)(false) footIsSelected,  // freeze the first decision
      (bool)(true) leftFoot,
      (Vector2f)(Vector2f(1000.0f, -340.0f)) kickTarget, // Based on 20_deg setup angle in ready card; This is a 20 degree shot
    }),
});

class OwnPushingFreeKickCard : public OwnPushingFreeKickCardBase
{
  /**
   * @brief The condition that needs to be met to execute this card
   */
  bool preconditions() const override
  {
    int i = 0;
    for (i = 0; i < 5; i++ ) {
      if (theTeammateRoles.roles[i] != TeammateRoles::GOALKEEPER_NORMAL &&
        theTeammateRoles.roles[i] != TeammateRoles::GOALKEEPER_ACTIVE &&
        theTeammateRoles.roles[i] != TeammateRoles::UNDEFINED)
        break;
    }
    return theRobotInfo.number == (i+1)
      && theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
      && theGameInfo.setPlay == SET_PLAY_PUSHING_FREE_KICK
      ;
  }

  /**
   * @brief The condition that needs to be met to exit the this card
   */
  bool postconditions() const override
  {
    return !preconditions();
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
