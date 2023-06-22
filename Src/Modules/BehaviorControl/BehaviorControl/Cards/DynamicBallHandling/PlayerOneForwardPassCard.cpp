/**
 * @file PlayerOneForwardPassCard.cpp
 * @author Asrar
 * @version 1.1
 * @date 21-06-2023
 *
 *
 * Functions, values, side effects:
 *Passing from player one to player two
 *
 * Details:
 * Purpose of this card is to walk to the ball and kick it to the front teammate.
 * v.1.1: increased the shoot strength from KickInfo::walkForwardsLeft to   KickInfo::walkForwardsLeftLong);
 
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

CARD(PlayerOneForwardPassCard,
     {
    ,
    CALLS(Activity),
    CALLS(GoToBallAndKick),
    CALLS(LookForward),
    CALLS(Stand),
    CALLS(WalkAtRelativeSpeed),
    REQUIRES(FieldBall),
    REQUIRES(FieldDimensions),
    REQUIRES(RobotPose),
    REQUIRES(TeamData),
    REQUIRES(TeamBehaviorStatus),   // R2K
    REQUIRES(TeammateRoles),        // R2K
    REQUIRES(PlayerRole),           // R2K
    REQUIRES(RobotInfo),            // R2K
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

class PlayerOneForwardPassCard : public PlayerOneForwardPassCardBase
{
    
    bool preconditions() const override
    {
        
      return 
        // theTeammateRoles.playsTheBall(&theRobotInfo, true) &&   // I am the striker
        theRobotInfo.number == 1;
        // theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number);
        
    }
    
    bool postconditions() const override
    {
        return !preconditions();
    }
    
    void execute() override
    {
        
        theActivitySkill(BehaviorStatus::playerOneForwardPass);
        
        float x = 0.f;
        float y = 0.f;

        for (const auto& buddy : theTeamData.teammates)
        {
            if (!buddy.isPenalized && buddy.isUpright)
            {
                
                if(buddy.number==2)
                {
                    x = buddy.theRobotPose.translation.x();
                    y = buddy.theRobotPose.translation.y();
                    break;
                }
            }
        }

        // theGoToBallAndKickSkill(calcAngleToOffense(x,y), KickInfo::walkForwardsLeft);
        theGoToBallAndKickSkill(calcAngleToOffense(x, y), KickInfo::walkForwardsLeftLong);
    }
    
    Angle calcAngleToOffense(float xPos, float yPos) const
    {
        return (theRobotPose.inversePose * Vector2f(xPos, yPos)).angle();
    }
};

MAKE_CARD(PlayerOneForwardPassCard);
