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
#include "Representations/Infrastructure/FrameInfo.h"



#include "Platform/SystemCall.h"



CARD(PointingCalibrationCard,
  {
     ,
     CALLS(Activity),
     CALLS(LookForward),
     CALLS(Stand),
     REQUIRES(EnhancedKeyStates),
     REQUIRES(FrameInfo),


     DEFINES_PARAMETERS(
          {,
       //Define Params here
       (unsigned)(0) startTime,
       (bool)(false) hasRun1,
       (bool)(false) hasRun2,
       (bool)(false) hasRun3,
       (bool)(false) hasRun4,
       (bool)(false) hasRun5,
       (bool)(false) hasRun6,
       (bool)(false) hasRun7,

    }),


  });

class PointingCalibrationCard : public PointingCalibrationCardBase
{
private:
  mutable bool isActive = true;  // mutable für Änderungen in const Methoden


  bool preconditions() const override
  {
    // Falls der Roboter am hinteren Kopfbereich berührt wurde, wird isActive auf true gesetzt
    if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] > 0)
    {
      isActive = true;
    }

    return isActive;
  }

  bool postconditions() const override
  {
    return true;
  }

  // Zustand und Logik für die Ausführung der PointingCard
  option
  {
    theActivitySkill(BehaviorStatus::testingBehavior);

    initial_state(init)
    {
      startTime = theFrameInfo.time;
      transition
      {
             if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] == 1) goto state1;
        else if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] == 2) goto state2;
        else if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] == 3) goto state3;
        else if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] == 4) goto state4;
        else if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] == 5) goto state5;
        else if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] == 6) goto state6;
        else if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] == 7) goto state7;
        else isActive = false;
      }
    }

    state(state1)
    {
      action
      {
        if (!hasRun1) { 
          hasRun1 = true; 
          SystemCall::say("1"); 
          OUTPUT_TEXT("1"); 
        }

        // Head Motion Request
        theLookForwardSkill();
        theStandSkill();
      }
        transition
      {
        if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] > 1)
          goto state2;
      }
    }

    state(state2)
    {
      action
      {
        if (!hasRun2) {
          hasRun2 = true;
          SystemCall::say("2");
          OUTPUT_TEXT("2");
        }

        // Head Motion Request
        theLookForwardSkill();
        theStandSkill();
      }
        transition
      {
        if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] > 2)
          goto state3;
      }
    }

    state(state3)
    {
      action
      {
        if (!hasRun3) {
          hasRun3 = true;
          SystemCall::say("3");
          OUTPUT_TEXT("3");
        }

        // Head Motion Request
        theLookForwardSkill();
        theStandSkill();
      }
        transition
      {
        if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] > 3)
          goto state4;
      }
    }

    state(state4)
    {
      action
      {
        if (!hasRun4) {
          hasRun4 = true;
          SystemCall::say("4");
          OUTPUT_TEXT("4");
        }

        // Head Motion Request
        theLookForwardSkill();
        theStandSkill();
      }
        transition
      {
        if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] > 4)
          goto state5;
      }
    }

    state(state5)
    {
      action
      {
        if (!hasRun5) {
          hasRun5 = true;
          SystemCall::say("5");
          OUTPUT_TEXT("5");
        }

        // Head Motion Request
        theLookForwardSkill();
        theStandSkill();
      }
        transition
      {
        if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] > 5)
          goto state6;
      }
    }

    state(state6)
    {
      action
      {
        if (!hasRun6) {
          hasRun6 = true;
          SystemCall::say("6");
          OUTPUT_TEXT("6");
        }

        // Head Motion Request
        theLookForwardSkill();
        theStandSkill();
      }
        transition
      {
        if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] > 6)
          goto state7;
      }
    }

    state(state7)
    {
      action
      {
        if (!hasRun7) {
          hasRun7 = true;
          SystemCall::say("7");
          OUTPUT_TEXT("7");
        }

        // Head Motion Request
        theLookForwardSkill();
        theStandSkill();
      }
        transition
      {
        if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] > 7)
          isActive = false;
      }
    }

  }
};

MAKE_CARD(PointingCalibrationCard);