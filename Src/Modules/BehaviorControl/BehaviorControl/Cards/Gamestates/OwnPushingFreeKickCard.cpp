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

#include "Tools/Math/Geometry.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"


CARD(OwnPushingFreeKickCard,
{,
  CALLS(Stand),
  CALLS(Activity),
  CALLS(LookForward),

  REQUIRES(OwnTeamInfo),
  REQUIRES(GameInfo),
});

class OwnPushingFreeKickCard : public OwnPushingFreeKickCardBase
{
  /**
   * @brief The condition that needs to be met to execute this card
   */
  bool preconditions() const override
  {
    return theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
        && theGameInfo.setPlay == SET_PLAY_PUSHING_FREE_KICK;
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
        theStandSkill();
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

MAKE_CARD(OwnPushingFreeKickCard);