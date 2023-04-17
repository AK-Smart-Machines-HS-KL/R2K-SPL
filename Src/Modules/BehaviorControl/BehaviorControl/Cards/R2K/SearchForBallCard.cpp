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


CARD(SearchForBallCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookForward),
        CALLS(LookActive),
        CALLS(Stand),
        CALLS(WalkAtRelativeSpeed),

        REQUIRES(DefaultPose),
        REQUIRES(FieldBall),
        REQUIRES(RobotPose),
        REQUIRES(RobotInfo),

        DEFINES_PARAMETERS(
             {,
          (int)(4000) headSweepDuration,
          (int)(400) bodyTurnDuration,
          (int)(5000) ballNotSeenTimeout,
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
    return !theFieldBall.ballWasSeen(ballNotSeenTimeout);
  }

  bool postconditions() const override
  {
    return !preconditions(); 
  }
  option
  {
    theActivitySkill(BehaviorStatus::searchForBall);

    initial_state(start)
    {
      transition
      {
        if (state_time > headSweepDuration)
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
        if (state_time > headSweepDuration + (rand() % 10 + 0.5) * bodyTurnDuration)
        goto start;
      }

      action
      {
         theLookForwardSkill();
         theWalkAtRelativeSpeedSkill(Pose2f(0.8f, 0.f, 0.f));
      }
    }
  }
};

MAKE_CARD(SearchForBallCard);
