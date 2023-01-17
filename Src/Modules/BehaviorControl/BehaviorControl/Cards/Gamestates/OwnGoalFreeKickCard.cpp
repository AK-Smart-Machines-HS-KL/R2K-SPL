/**
 * @file OwnGoalFreeKickCard.cpp
 * @author Andy Hobelsberger
 * @brief Covers the Free Kick: Own Team has Goal Free (11m) Kick
 * @version 1.1
 * @date 2022-11-22
 * 
 * Notes: 
 *  - Currently Triggers for all Robots. Use this Card as a template for preconditions
 *  - Currently only calls stand

 * V1.1 Card migrated (Nicholas)
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


CARD(OwnGoalFreeKickCard,
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
	{
		,
		(Vector2f)(Vector2f(1000.0f, -340.0f))kickTarget,  // Based on 20_deg setup angle in ready card; This is a 20 degree shot
	}),
});

class OwnGoalFreeKickCard : public OwnGoalFreeKickCardBase
{

  KickInfo::KickType kickType;

  /**
   * @brief The condition that needs to be met to execute this card
   */

  bool preconditions() const override
  {
		return theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber 
      && theGameInfo.setPlay == SET_PLAY_GOAL_KICK 
      && theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number);
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
        theGoToBallAndKickSkill(calcAngleToTarget(), kickType);
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

  Angle calcAngleToTarget() const
  {
    return (theRobotPose.inversePose * kickTarget).angle();
  }
};

MAKE_CARD(OwnGoalFreeKickCard);