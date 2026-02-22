/**
 * @file OffenseReceivePassCard.cpp
 * @author Adrian Müller
 * @version 1.0
 * @date 2023-31-01
 *
 *
 * Functions, values, side effects:
 * check for bot with supporterIndex -1, whether it is in the process of passing the ball to him
 *
 * Details:
 * uses buddy.theBehaviorStatus.activity == BehaviorStatus::offenseForwardPassCard)
 *

 *
 * Note:
 * maybe we should another card, so the potential receiver actively walks to a promising position on field.
 *
 *
 * OpenPoints:
 *  we need to verify this approach is ok with EBC 
 */

// B-Human includes
#include "Representations/BehaviorControl/Skills.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Representations/Communication/TeamData.h"
#include "Tools/BehaviorControl/R2KDecisionLog.h"
#include <string>

// this is the R2K specific stuff
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/TeamCommStatus.h"

CARD(OffenseReceivePassCard,
     {
    ,
    CALLS(Activity),
    CALLS(LookActive),
    CALLS(Stand),
    REQUIRES(TeamData),
    REQUIRES(TeammateRoles),       
    REQUIRES(PlayerRole),          
    REQUIRES(RobotInfo),           
    REQUIRES(TeamCommStatus),
    
    /*
     //Optionally, Load Config params here. DEFINES and LOADS can not be used together
     LOADS_PARAMETERS(
     {,
     //Load Params here
     }),
     
     */
    
});

class OffenseReceivePassCard : public OffenseReceivePassCardBase
{
    
    bool preconditions() const override
    {
               
      return
        thePlayerRole.supporterIndex() == thePlayerRole.numOfActiveSupporters &&
        aBuddyIsPassing() &&
        !theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive) &&   // I am not the striker
        theTeammateRoles.isTacticalOffense(theRobotInfo.number);
        /* && // my recent role
        theTeamBehaviorStatus.teamActivity != TeamBehaviorStatus::R2K_SPARSE_GAME;
        */
    }
    
    bool postconditions() const override
    {
        return !preconditions();
    }
    
    void execute() override
    {
        int passer = -1;
        for (const auto& buddy : theTeamData.teammates)
          if (buddy.theBehaviorStatus.activity == BehaviorStatus::offenseForwardPassCard)
          {
            passer = buddy.number;
            break;
          }

        R2KDecisionLog::annotation("pass_ack", {{"card", "OffenseReceivePass"},
                                          {"receiver", std::to_string(theRobotInfo.number)},
                                          {"passer", std::to_string(passer)}});

        theActivitySkill(BehaviorStatus::offenseReceivePassCard);
        theLookActiveSkill();
        theStandSkill();
    }
    
    bool aBuddyIsPassing() const
    {
      for (const auto& buddy : theTeamData.teammates)
      {
        if (buddy.theBehaviorStatus.activity == BehaviorStatus::offenseForwardPassCard)
          return true;
      }
      return false;
    }
};



MAKE_CARD(OffenseReceivePassCard);
