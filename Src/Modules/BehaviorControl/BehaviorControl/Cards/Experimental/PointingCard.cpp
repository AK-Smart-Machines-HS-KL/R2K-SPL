/**
 * @file PointingCard.cpp
 * @author Jonathan Brauch    
 * @version 0.3
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


CARD(PointingCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookForward),
        CALLS(Stand),
        CALLS(PointAt),
        CALLS(PointAtWithArm),
        CALLS(TurnToPoint),


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
  private:
  bool isActive = true;

  public:
  //always active
  bool preconditions() const override
  {
    return isActive;
  }

  bool postconditions() const override
  {
    return true;   // set to true, when used as default card, ie, lowest card on stack
  }

  void execute() override
  {
    
    theActivitySkill(BehaviorStatus::testingBehavior);
    // std::string s = "testingBehavior";

    // Override these skills with the skills you wish to test
    theLookForwardSkill(); // Head Motion Request
   // theStandSkill(); // Standard Motion Request


    OUTPUT_TEXT("Test pointing");

    const Vector2f localPoint = Vector2f(-900.f, -500.f);
    theTurnToPointSkill(localPoint);


    const Vector3f localPoint1 = Vector3f(0.f, 3000.f, 355.f);
    thePointAtSkill(localPoint1);





    //thePointAtWithArmSkill(Vector3f(1000.f, 0.f, 0.f), Arms::right);
    //thePointAtWithArmSkill(Vector3f(1000.f, 0.f, 1000.f), Arms::left);

    isActive = false;

  }
};

MAKE_CARD(PointingCard);
