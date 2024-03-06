/**
 * @file PointingCard.cpp
 * @author Jonathan Brauch
 * @version 1.0
 * @date 2024-03-04
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
#include "Representations/Infrastructure/FrameInfo.h"


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
     REQUIRES(FieldBall),
     REQUIRES(FrameInfo),


     DEFINES_PARAMETERS(
          {,
       //Define Params here
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

class PointingCard : public PointingCardBase
{
private:
  bool isActive = true;

public:
  // always active
  bool preconditions() const override
  {
    int timeSinceLastStart = theFrameInfo.getTimeSince(startTime);

    return isActive && (timeSinceLastStart > cooldown); // Überprüfung der Ausführungscooldown-Zeit
  }

  bool postconditions() const override
  {
    return true; // set to true, when used as the default card, i.e., the lowest card on the stack
  }

  void execute() override
  {
    theActivitySkill(BehaviorStatus::testingBehavior);

    theLookForwardSkill(); // Head Motion Request
    

    OUTPUT_TEXT("Test pointing");

    // Die relative Ballposition holen
    Vector2f ballPositionRelative = theFieldBall.recentBallPositionRelative();

    // Die relative Ballposition im theTurnToPointSkill verwenden
    theTurnToPointSkill(ballPositionRelative);

    // Die Z-Koordinate auf 0 setzen, um einen Vector3f zu erstellen
    Vector3f ballPositionRelative3D(ballPositionRelative.x(), ballPositionRelative.y(), 0.0f);

    // Die relative Ballposition im thePointAtSkill verwenden
    thePointAtSkill(ballPositionRelative3D);





    //Mit festen Koordinaten
    // const Vector2f localPoint = Vector2f(-900.f, -500.f);
    // theTurnToPointSkill(localPoint);

    // const Vector3f localPoint1 = Vector3f(0.f, 3000.f, 355.f);
    // thePointAtSkill(localPoint1);

    // thePointAtWithArmSkill(Vector3f(1000.f, 0.f, 0.f), Arms::right);
    // thePointAtWithArmSkill(Vector3f(1000.f, 0.f, 1000.f), Arms::left);

    // Aktualisieren Sie den Zeitpunkt der letzten Ausführung



    startTime = theFrameInfo.time;
  }
};


MAKE_CARD(PointingCard);
