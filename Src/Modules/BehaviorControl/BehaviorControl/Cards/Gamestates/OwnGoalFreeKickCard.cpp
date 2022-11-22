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

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Communication/RobotInfo.h"

#include "Tools/Math/Geometry.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"


CARD(OwnGoalFreeKickCard,
{,
  CALLS(Stand),
  CALLS(Activity),
  CALLS(LookForward),
  CALLS(GoAndKickAtTarget),

  REQUIRES(OwnTeamInfo),
  REQUIRES(GameInfo),
	REQUIRES(RobotInfo), 

  DEFINES_PARAMETERS(
	{
		,
		(Vector2f)(Vector2f(1000.0f, -340.0f))kickTarget,  // Based on 20_deg setup angle in ready card; This is a 20 degree shot
	}),
});

class OwnGoalFreeKickCard : public OwnGoalFreeKickCardBase
{
  /**
   * @brief The condition that needs to be met to execute this card
   */
  bool preconditions() const override
  {
		return theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber 
      && theGameInfo.setPlay == SET_PLAY_GOAL_KICK 
      && (1 == theRobotInfo.number);
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
        
      }

      action
      {
        theLookForwardSkill();
				theGoAndKickAtTargetSkill(kickTarget);
      }
    }

    state(active)
    {
      transition
      {

      }

      action
      {

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
};

MAKE_CARD(OwnGoalFreeKickCard);