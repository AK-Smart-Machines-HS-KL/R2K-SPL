/**
 * @file PointingCard.cpp
 * @author Jonathan Brauch    
 * @brief This card's preconditions are always true. 
 *        Edit it for testing
 * @version 0.1
 * @date 2024-01-10
 * 
 * 
 */

// Skills - Must be included BEFORE Card Base
#include "Representations/BehaviorControl/Skills.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

// Representations
//#include "Representations/BehaviorControl/FieldBall.h"

#include "Representations/MotionControl/ArmMotionRequest.h"
//#include "../../../../../Tools/Debugging/Debugging.h"
//#include <filesystem>

// Modify this card but don't commit changes to keep it clean for other developers
// Also don't forget to put this card at the top of your Card Stack!
CARD(PointingCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookForward),
        CALLS(Stand),
        CALLS(PointAt),
        CALLS(PointAtWithArm),


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

class PointingCard : public PointingCardBase
{

  //always active
  bool preconditions() const override
  {
    return true;
  }

  bool postconditions() const override
  {
    return true;   // set to true, when used as default card, ie, lowest card on stack
  }

  void execute() override
  {
    OUTPUT_TEXT("Test Pointing Start");
    
    const Vector3f localPoint = Vector3f(100.f, 102.f, 1030.f);
    thePointAtSkill(localPoint);


    thePointAtWithArmSkill(Vector3f(1000.f, 0.f, 0.f), Arms::right);
    thePointAtWithArmSkill(Vector3f(1000.f, 0.f, 1000.f), Arms::left);


    theActivitySkill(BehaviorStatus::testingBehavior);
    // std::string s = "testingBehavior";
    // OUTPUT_TEXT(s);

    // Override these skills with the skills you wish to test
    theLookForwardSkill(); // Head Motion Request
    theStandSkill(); // Standard Motion Request

  }
};

MAKE_CARD(PointingCard);
