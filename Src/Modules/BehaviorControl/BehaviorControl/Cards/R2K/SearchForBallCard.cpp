/**
 * @file SearchForBallCard.cpp
 * @author Adrian Müller 
 * @brief theFieldBall says: haven't seen the ball since ..
 * @version 1.0
 * @date 2023-04-16
 * 
 * Functions, values, side effects: this card qualifies for all players. 
      It should be high on the gameplay stack
 * Details:
 * 
 * pre: if !theFieldBall.ballWasSeen(ballNotSeenTimeout);
 * 1) sweep head for headSweepDuration
 * 2) if ball is still not found, turn body for random ~ [180o ... 360o] 
   3) repeat with 1)
 * 
 * Note: theLookActiveSkill(true,    true,      false,      false) 
 *                          withBall,ignoreBall,onlyOwnBall,fixTilt
 * 
 */

#include <stdlib.h>     /* srand, rand */

// Skills - Must be included BEFORE Card Base
#include "Representations/BehaviorControl/Skills.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

// Representations
#include "Representations/BehaviorControl/DefaultPose.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Infrastructure/ExtendedGameInfo.h"


CARD(SearchForBallCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookForward),
        CALLS(LookActive),
        CALLS(Stand),
        CALLS(LookAtBall),
        CALLS(WalkAtRelativeSpeed),

        REQUIRES(DefaultPose),
        REQUIRES(FieldBall),
        REQUIRES(RobotPose),
        REQUIRES(RobotInfo),
        REQUIRES(TeammateRoles),
        REQUIRES(FrameInfo),
        REQUIRES(ExtendedGameInfo),
        DEFINES_PARAMETERS(
             {,
          (int)(2500) headSweepDuration,
          (int)(3000) bodyTurnDuration,
          (int)(5000) ballNotSeenTimeout,
          (int)(9000) maxRuntime,
          (int)(10000) cooldown,
          (unsigned)(0) startTime,
             }),

        /*
        //Optionally, Load Config params here. DEFINES and LOADS can not be used together
        LOADS_PARAMETERS(
             {,
                //Load Params here
             }),
             
        */

     });

class SearchForBallCard : public SearchForBallCardBase
{

  //always active
  bool preconditions() const override
  {
    // return true;   // use for testing the head and body moves in a fast game
    int timeSinceLastStart = theFrameInfo.getTimeSince(startTime);
    return !theFieldBall.ballWasSeen(ballNotSeenTimeout)
      && (timeSinceLastStart < maxRuntime || timeSinceLastStart > maxRuntime + cooldown)
      && theExtendedGameInfo.timeSinceLastPenaltyEnded > 10000 
      && !theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number);
  }

  bool postconditions() const override
  {
    //int timeSinceLastStart = theFrameInfo.getTimeSince(startTime);
    return !preconditions();
  }

  option
  {
    theActivitySkill(BehaviorStatus::searchForBall);

    initial_state(init)
    {
      startTime = theFrameInfo.time;
      transition
      {
        if (std::abs(theFieldBall.positionRelative.angle()) < 10_deg) {
          	goto search;
        }
      }

      action{
        theLookAtBallSkill();
        theWalkAtRelativeSpeedSkill(Pose2f(std::clamp(theFieldBall.positionRelative.angle() * 3.0f, -1.0f, 1.0f), 0.f, 0.f));
      }
    }
  
    state(search)
    {
      transition
      {
        if (state_time > headSweepDuration) 
        // !theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number))
          goto turnBody;
      }

      action
      {
      theLookActiveSkill(true,true, false,false); // Head Motion Request
      theStandSkill();
      }
    }

    state(turnBody)
    {
      transition
      {
        srand(theRobotInfo.number);
        if (state_time > bodyTurnDuration)
        goto search;
      }

      action
      {
        theLookForwardSkill();
        if (theRobotPose.translation.y() > 0 ) {
          theWalkAtRelativeSpeedSkill(Pose2f(-2.5f, 0.f, 0.f));
        } else {
          theWalkAtRelativeSpeedSkill(Pose2f(2.2f, 0.f, 0.f));
        }
      }
    }
  }
};

MAKE_CARD(SearchForBallCard);
