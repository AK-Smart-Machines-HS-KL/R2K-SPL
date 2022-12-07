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
});

class OwnKickInCard : public OwnKickInCardBase
{
  KickInfo::KickType kickType;


  /**
   * @brief The condition that needs to be met to execute this card
   */
  bool preconditions() const override
  {
    return
        theTeammateRoles.playsTheBall(theRobotInfo.number)
        && theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
        && theGameInfo.setPlay == SET_PLAY_KICK_IN;
  }

  /**
   * @brief The condition that needs to be met to exit the this card
   */
  bool postconditions() const override
  {
    return !preconditions();
  }

  option
  {
    theActivitySkill(BehaviorStatus::ownFreeKick);
    initial_state(init)
    {

      transition
      {
        bool leftFoot = theFieldBall.positionRelative.y() < 0;
        kickType = leftFoot ? KickInfo::forwardFastLeft : KickInfo::forwardFastRight;
        goto active;
      }

      action
      {
        
      }
    }

    state(active)
    {
      transition
      {

      }

      action
      {
        theGoToBallAndKickSkill(calcAngleToGoal(), kickType);
      }
    }

    state(standby)
    {
      transition
      {

      }

      action
      {
        
      }
    }
  }

  Angle calcAngleToGoal() const
  {
    return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
  }
};

MAKE_CARD(OwnKickInCard);