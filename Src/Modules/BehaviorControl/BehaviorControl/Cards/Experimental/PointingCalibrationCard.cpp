/**
 * @file PointingCalibrationCard.cpp
 * @author Jobr1005@stud.hs-kl.de
 * @date 2025-01-16
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
#include "Representations/Infrastructure/SensorData/KeyStates.h"

#include "Platform/SystemCall.h"



CARD(PointingCalibrationCard,
  {
     ,
     CALLS(Activity),
     CALLS(LookForward),
     CALLS(Stand),
     REQUIRES(EnhancedKeyStates),


     DEFINES_PARAMETERS(
          {,
       //Define Params here
       (int)(0) step,
    }),


  });

class PointingCalibrationCard : public PointingCalibrationCardBase
{
private:
  mutable bool isActive = false;  // mutable für Änderungen in const Methoden


  bool preconditions() const override
  {
    // Falls der Roboter am hinteren Kopfbereich berührt wurde, wird isActive auf true gesetzt
    if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] > 0)
    {
      isActive = true;
    }

    return false;
  }

  bool postconditions() const override
  {
    return true;
  }

  // Zustand und Logik für die Ausführung der PointingCalibrationCard
  void execute() override
  {
    theActivitySkill(BehaviorStatus::testingBehavior);

    SystemCall::say("Test funktioniert");

     
     OUTPUT_TEXT("geeeeht");

    // Override these skills with the skills you wish to test
    theLookForwardSkill(); // Head Motion Request
    theStandSkill(); // Standard Motion Request

  }
};

MAKE_CARD(PointingCalibrationCard);