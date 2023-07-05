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
 * ToDo:
 *  we need to verify this approach is ok with EBC 
 */

// B-Human includes
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Modeling/RobotPose.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Tools/Math/BHMath.h"
#include "Representations/Communication/TeamData.h"

// this is the R2K specific stuff
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"
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
    REQUIRES(FieldBall),
    REQUIRES(FieldDimensions),
    REQUIRES(RobotPose),
    REQUIRES(TeamData),
    REQUIRES(TeamBehaviorStatus),  
    REQUIRES(TeammateRoles),       
    REQUIRES(PlayerRole),          
    REQUIRES(RobotInfo),           
    REQUIRES(TeamCommStatus),
    DEFINES_PARAMETERS(
                       {,
                           (float)(0.8f) walkSpeed,
                       }),
    
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
        !theTeammateRoles.playsTheBall(&theRobotInfo,theTeamCommStatus.isWifiCommActive) &&   // I am not the striker
        theTeammateRoles.isTacticalOffense(theRobotInfo.number) && // my recent role
        theTeamBehaviorStatus.teamActivity != TeamBehaviorStatus::R2K_SPARSE_GAME;
        
    }
    
    bool postconditions() const override
    {
        return !preconditions();
    }
    
    void execute() override
    {
        
        theActivitySkill(BehaviorStatus::offenseReceivePassCard);
        theLookActiveSkill();
        theStandSkill();
        
        
    }
    
    Angle calcAngleToOffense(float xPos, float yPos) const
    {
        return (theRobotPose.inversePose * Vector2f(xPos, yPos)).angle();
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
