/**
 * @file GoalieDiveCard.cpp
 * @author Asfiya Aazim
 * @brief This card's preconditions are always true.
 *        Edit it for testing
 * @version 0.1
 * @date 2023-04-05
 *
 *
 */

 // Skills - Must be included BEFORE Card Base
#include "Representations/BehaviorControl/Skills.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

// Representations
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations\Communication\RobotInfo.h"
//#include <filesystem>

// Modify this card but don't commit changes to keep it clean for other developers
// Also don't forget to put this card at the top of your Card Stack!
CARD(GoalieDiveCard,
  {
     ,
     CALLS(Activity),
     CALLS(InterceptBall),
     REQUIRES(FieldBall),
     REQUIRES(RobotInfo),


     DEFINES_PARAMETERS(
          {,
       //Define Params here
    }),

  /*
  //Optionally, Load Config params here. DEFINES and LOADS can not be used together
  LOADS_PARAMETERS(
       {,
          //Load Params here
       }),

  */

  });

class GoalieDiveCard : public GoalieDiveCardBase
{

  //always active
  bool preconditions() const override
  {
    return theFieldBall.isRollingTowardsOwnGoal && theRobotInfo.number == 1;

  }

  bool postconditions() const override
  {
    return true;   // set to true, when used as default card, ie, lowest card on stack
  }

  void execute() override
  {

    theActivitySkill(BehaviorStatus::testingBehavior);
    // std::string s = "testingBehavior";
    // OUTPUT_TEXT(s);

    // Override these skills with the skills you wish to test

    theInterceptBallSkill((unsigned)(bit(Interception::jumpRight) | bit(Interception::jumpLeft) | bit(Interception::genuflectFromSitting) | bit(Interception::stand)));
  }
};

MAKE_CARD(GoalieDiveCard);