/**
 * @file OffenseForwardPassCard.cpp
 * @author Niklas Schmidts, Adrian Müller
 * @version 1.1
 * @date 2023-001-006
 *
 *
 * Functions, values, side effects:
 *
 *
 * Details:
 * Purpose of this card is to walk to the ball and kick it to the front teammate.
 * Only the the second player from the front beginning can activate this card.
 *
 * v.1.1: increased the shoot strength from KickInfo::walkForwardsLeft to   KickInfo::walkForwardsLeftLong);
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
#include "Representations/Modeling/ObstacleModel.h"
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
#include "Representations/Infrastructure/ExtendedGameInfo.h"

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
    REQUIRES(ObstacleModel),
    REQUIRES(RobotPose),
    REQUIRES(TeamData),
    REQUIRES(TeamBehaviorStatus),   
    REQUIRES(TeammateRoles),        
    REQUIRES(PlayerRole),           
    REQUIRES(RobotInfo),           
    REQUIRES(TeamCommStatus),
    REQUIRES(ExtendedGameInfo),
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

class OffenseForwardPassCard : public OffenseForwardPassCardBase
{
    
    bool preconditions() const override
    {
        
      return
        !aBuddyIsClearingOrPassing() &&      
        theTeammateRoles.playsTheBall(&theRobotInfo,theTeamCommStatus.isWifiCommActive) &&   // I am the striker
        theTeammateRoles.isTacticalOffense(theRobotInfo.number) && // my recent role
        // either a substantial delta on x - or we are at kick-off
        ( thePlayerRole.supporterIndex() == thePlayerRole.numOfActiveSupporters - 1 ||
         theExtendedGameInfo.timeSincePlayingStarted < 10000) &&
        // theObstacleModel.opponentIsTooClose(theFieldBall.positionRelative) != KickInfo::LongShotType::noKick &&  
        theTeamBehaviorStatus.teamActivity != TeamBehaviorStatus::R2K_SPARSE_GAME;
        
    }
    
    bool postconditions() const override
    {
        return !preconditions();
    }
    
    void execute() override
    {
        
        theActivitySkill(BehaviorStatus::offenseForwardPassCard);
        
        float x = 0;
        float y = -700;

        for (const auto& buddy : theTeamData.teammates)
        {
            if (!buddy.isPenalized && buddy.isUpright)
            {
                
                if(buddy.theRobotPose.translation.x()>theRobotPose.translation.x())
                {
                    x = buddy.theRobotPose.translation.x()+x;
                    y = buddy.theRobotPose.translation.y()+y;
                    
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
    bool aBuddyIsClearingOrPassing() const
    {
      for (const auto& buddy : theTeamData.teammates) 
      {
        if (
          // buddy.theBehaviorStatus.activity == BehaviorStatus::clearOwnHalfCard ||
          // buddy.theBehaviorStatus.activity == BehaviorStatus::clearOwnHalfCardGoalie ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::defenseLongShotCard ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::goalieLongShotCard ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::goalShotCard ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::offenseForwardPassCard)
          // uddy.theBehaviorStatus.activity == BehaviorStatus::offenseReceivePassCard)
          return true;
      }
      return false;
    }
};

MAKE_CARD(OffenseForwardPassCard);
