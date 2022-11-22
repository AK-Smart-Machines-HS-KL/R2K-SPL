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
 */

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"

#include "Tools/Math/Geometry.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Representations/Communication/RobotInfo.h"


CARD(OwnCornerKickCard,
{,
  CALLS(Stand),
  CALLS(Activity),
  CALLS(LookForward),
  CALLS(WalkToBallAndKickAtGoal),

  REQUIRES(OwnTeamInfo),
  REQUIRES(GameInfo),
  REQUIRES(TeamBehaviorStatus),
  REQUIRES(RobotInfo),
});

class OwnCornerKickCard : public OwnCornerKickCardBase
{
  /**
   * @brief The condition that needs to be met to execute this card
   */
  bool preconditions() const override
  {
		return theTeamBehaviorStatus.role.playBall
        && theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
        && theGameInfo.setPlay == SET_PLAY_CORNER_KICK;
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
        // theStandSkill();
				theWalkToBallAndKickAtGoalSkill();
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

MAKE_CARD(OwnCornerKickCard);