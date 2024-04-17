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
    
    Vector2f targetAbsolute = Vector2f::Zero();
    bool preconditions() const override
    {
        bool buddyValid = false;
        
        for (const auto& buddy : theTeamData.teammates)
        {
            if (!buddy.isPenalized && buddy.isUpright)
            {
                if(buddy.theRobotPose.translation.x() > theRobotPose.translation.x()) {
                    buddyValid = true;
                    break;
                }
            }
        }

        if (!buddyValid) {
            return false;  // no boot closer towards goal than me
        }
        if(
          !aBuddyIsClearingOrPassing() &&
          theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive) &&   // I am the striker
          theTeammateRoles.isTacticalOffense(theRobotInfo.number) && // my recent role
          // either a substantial delta on x - or we are at kick-off
          (thePlayerRole.supporterIndex() == thePlayerRole.numOfActiveSupporters - 1 ||
            theExtendedGameInfo.timeSincePlayingStarted < 10000)  // side pass at kickOff
        // theObstacleModel.opponentIsTooClose(theFieldBall.positionRelative) != KickInfo::LongShotType::noKick &&  
        // theTeamBehaviorStatus.teamActivity != TeamBehaviorStatus::R2K_SPARSE_GAME;
          ) return true;
        return false;
    }
    
    bool postconditions() const override
    {
        return !preconditions();
    }

    Vector2f getTarget() {
        Vector2f target = Vector2f::Zero();
        for (const auto& buddy : theTeamData.teammates)
        {
            if (!buddy.isPenalized && buddy.isUpright)
            {
                if(buddy.theRobotPose.translation.x() > theRobotPose.translation.x()) {
                    if(target.x() < buddy.theRobotPose.translation.x() || target == Vector2f::Zero()) {
                        target = buddy.theRobotPose.translation;
                        target.x() += 1500;
                    }
                }
            }
        }
        return target;
    }
    
    void execute() override
    {
        // If we just enetered the card, grab the best passing target
        if (targetAbsolute == Vector2f::Zero()) {
            targetAbsolute = getTarget();
        }
        
        theActivitySkill(BehaviorStatus::offenseForwardPassCard);
        theGoToBallAndKickSkill(theRobotPose.toRelative(targetAbsolute).angle(), KickInfo::forwardFastLeft);
    }

    void reset() override
    {
        targetAbsolute = Vector2f::Zero();
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
