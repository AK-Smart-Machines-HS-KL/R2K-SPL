/**
 * @file OwnKickInCard.cpp
 * @author Andy Hobelsberger, Adrian Müller (R2K)
 * @brief 
 * @version 1.0
 * @date 2022-11-22
 *   
 * Covers the Free Kick: Own Team has Kick in
 * added (Adrian): pre-condition: triggers for role.playBall, action: WalkToBallAndKickAtGoal
 * V1.1 Card migrated; uses goToBallAndKickAtGoal (Nicholas) 
 * v1.2. Gore hack: first DEFENSE and latest (acc. to roles[] OFFENSE qualify
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
  REQUIRES(TeammateRoles),

  DEFINES_PARAMETERS(
    { ,
      (bool)(false) footIsSelected,  // freeze the first decision
      (bool)(true) leftFoot,
      (Vector2f)(Vector2f(1000.0f, -340.0f)) kickTarget, // Based on 20_deg setup angle in ready card; This is a 20 degree shot
    }),

});

class OwnKickInCard : public OwnKickInCardBase
{

  /**
   * @brief The condition that needs to be met to execute this card
   */
  bool preconditions() const override
  {
    int theDefense = -1;
    for (int i = 0; i < 5; i++) {
      if (theTeammateRoles.isTacticalDefense(i + 1)) {
        theDefense = i + 1;
        break;
      }
    }
    int theOffense = -1;
    for (int j = 4; j >= 0; j--) {
      if (theTeammateRoles.isTacticalOffense(j + 1)) {
        theOffense = j + 1;
        break;
      }
    }
    return
      // theTeammateRoles.playsTheBall(theRobotInfo.number)
      theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
      && theGameInfo.setPlay == SET_PLAY_KICK_IN
      && (theRobotInfo.number == theDefense || theRobotInfo.number == theOffense);
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
    KickInfo::KickType kickType = leftFoot ? KickInfo::forwardFastLeft : KickInfo::forwardFastRight;
    theGoToBallAndKickSkill(calcAngleToGoal(), kickType, true);
  }
  Angle calcAngleToGoal() const
  {
  return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
  }
};

MAKE_CARD(OwnKickInCard);