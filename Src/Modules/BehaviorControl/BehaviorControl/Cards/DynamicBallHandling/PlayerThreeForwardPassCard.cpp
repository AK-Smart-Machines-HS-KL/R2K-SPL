/**
 * @file PlayerThreeForwardPassCard.cpp
 * @author Asrar
 * @version 1.1
 * @date 22-06-2023
 *
 *
 * Functions, values, side effects:
 *Passing from player three to player two
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

CARD(PlayerThreeForwardPassCard,
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

class PlayerThreeForwardPassCard : public PlayerThreeForwardPassCardBase
{
    
    bool preconditions() const override
    {
        
      return //theTeammateRoles.playsTheBall(theRobotInfo.number) &&   // I am the striker
        theRobotInfo.number == 3
        && theFieldBall.positionRelative.norm() < 1500;
       // theTeammateRoles.isTacticalDefense(theRobotInfo.number);
        
    }
    
    bool postconditions() const override
    {
        return !preconditions();
    }
    
    void execute() override
    {
        
        theActivitySkill(BehaviorStatus::PlayerThreeForwardPass);
        
        float x = -500.f;
        float y = -2500.f;

        for (const auto& buddy : theTeamData.teammates)
        {
            if (!buddy.isPenalized && buddy.isUpright)
            {
                
                if(buddy.number==2)
                {
                    OUTPUT_TEXT("actual target " << x << "  " << y);
                    x = buddy.theRobotPose.translation.x()+500;
                    y = buddy.theRobotPose.translation.y()-500;
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

MAKE_CARD(PlayerThreeForwardPassCard);
