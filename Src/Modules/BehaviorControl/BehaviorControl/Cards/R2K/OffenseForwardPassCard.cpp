/**
 * @file OffenseForwardPassCard.cpp
 * @author Niklas Schmidts
 * @version 1.0
 * @date 2022-09-04
 *
 *
 * Functions, values, side effects:
 *
 *
 * Details:
 * Purpose of this card is to walk to the ball and kick it to the front teammate.
 * Only the the second player from the front beginning can activate this card.
 *

 *
 * Note:
 *
 *
 *
 * ToDo:
 * - Precondition needs to be fixed
 * - If everything works -> Card clean
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

CARD(OffenseForwardPassCard,
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
                           (int)(1000) initialWaitTime,
                           (int)(7000) ballNotSeenTimeout,
                       }),
    
    /*
     //Optionally, Load Config params here. DEFINES and LOADS can not be used together
     LOADS_PARAMETERS(
     {,
     //Load Params here
     }),
     
     */
    
});

class OffenseForwardPassCard : public OffenseForwardPassCardBase
{
    
    bool preconditions() const override
    {
        //OUTPUT_TEXT(thePlayerRole.playsTheBall());
        OUTPUT_TEXT(theRobotInfo.number);
        OUTPUT_TEXT(thePlayerRole.supporterIndex());
        OUTPUT_TEXT(thePlayerRole.numOfActiveSupporters - 1);
        OUTPUT_TEXT("-------------------------------");
        
        return true;
        //thePlayerRole.playsTheBall() &&  // I am the striker
        //thePlayerRole.supporterIndex() == thePlayerRole.numOfActiveSupporters - 1;
        
        //theTeamBehaviorStatus.teamActivity == TeamBehaviorStatus::R2K_NORMAL_GAME;
        
    }
    
    bool postconditions() const override
    {
        return !preconditions();
    }
    
    void execute() override
    {
        
        theActivitySkill(BehaviorStatus::offenseForwardPassCard);
        
        float x;
        float y;

        for (const auto& buddy : theTeamData.teammates)
        {
            if (!buddy.isPenalized && buddy.isUpright)
            {
                
                if(buddy.theRobotPose.translation.x()>theRobotPose.translation.x())
                {
                    x = buddy.theRobotPose.translation.x();
                    y = buddy.theRobotPose.translation.y();
                    
                }
            }
        }

        theGoToBallAndKickSkill(calcAngleToOffense(x,y), KickInfo::walkForwardsLeft);
        
    }
    
    Angle calcAngleToOffense(float xPos, float yPos) const
    {
        return (theRobotPose.inversePose * Vector2f(xPos, yPos)).angle();
    }
};

MAKE_CARD(OffenseForwardPassCard);
