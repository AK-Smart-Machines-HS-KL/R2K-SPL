/**
 * @file OwnKickoffCard.cpp
 * @author Andy Hobelsberger
 * @brief Covers Own Kickoff
 * @version 0.1
 * @date 2022-11-22
 * 
 * Behavior: During the Own Kickoff, Robot 5 attempts to kick the ball 20_deg to the left 
 * 
 * V1.1 Card migrated (Nicholas)
 */

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Communication/RobotInfo.h"

#include "Tools/Math/Geometry.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"



CARD(OwnKickoffCard,
{,
  CALLS(Stand),
  CALLS(Activity),
  CALLS(LookForward),
  CALLS(GoAndKickAtTarget),

  REQUIRES(OwnTeamInfo),
  REQUIRES(GameInfo),
  REQUIRES(RobotInfo),
	REQUIRES(TeamBehaviorStatus),

  DEFINES_PARAMETERS(
  {,
    (Vector2f)(Vector2f(1000.0f, -340.0f)) kickTarget, // Based on 20_deg setup angle in ready card; This is a 20 degree shot
  }),
});

class OwnKickoffCard : public OwnKickoffCardBase
{
  /**
   * @brief The condition that needs to be met to execute this card
   */
  bool preconditions() const override
  {  
    return theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber 
           && theGameInfo.secsRemaining >= 590 
           && theGameInfo.state == STATE_PLAYING 
           && ((4 == theRobotInfo.number 
               && !std::strcmp(theRobotInfo.getPenaltyAsString().c_str(), "None") 
               && theTeamBehaviorStatus.teamActivity == TeamBehaviorStatus::R2K_DEFENSIVE_GAME
               ) 
              || 
              (5 == theRobotInfo.number 
               && theTeamBehaviorStatus.teamActivity == TeamBehaviorStatus::R2K_NORMAL_GAME
              ));
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
    theActivitySkill(BehaviorStatus::ownKickoff);
    
    initial_state(init)
    {
      transition
      {
        goto kick;
      }

      action
      {
        theLookForwardSkill();
        theStandSkill();
      }
    }

    state(kick)
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
  }
};

MAKE_CARD(OwnKickoffCard);