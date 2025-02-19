/**
 * @file PointingCalibrationCard.cpp
 * @author Jobr1005@stud.hs-kl.de
 * @date 2025-02-19
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
#include "Representations/MotionControl/ArmMotionRequest.h"
#include "Representations/BehaviorControl/DefaultPose.h"
#include "Representations/Modeling/RobotPose.h"


#include "Platform/SystemCall.h"



CARD(PointingCalibrationCard,
  {
     ,
     CALLS(Activity),
     CALLS(LookForward),
     CALLS(LookActive),
     CALLS(Stand),
     CALLS(PointAt),
     CALLS(TurnToPoint),
     CALLS(WalkToPoint),
     CALLS(WalkToPose),
     REQUIRES(DefaultPose),
     REQUIRES(RobotPose),
     REQUIRES(FieldBall),
     REQUIRES(EnhancedKeyStates),


     DEFINES_PARAMETERS(
          {,
       //Define Params here
       (bool)(false) hasReachedDefaultPose,
       (bool)(false) hasReachedCenterCircle,
       (bool)(false) hasReachedBall,

    }),


  });

class PointingCalibrationCard : public PointingCalibrationCardBase
{
private:
  mutable bool isActive = false;  


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

  // Zustand und Logik für die Ausführung der PointingCalibrationCard
  option
  {
    theActivitySkill(BehaviorStatus::testingBehavior);

    initial_state(init)
    {
      transition
      {
             // Mehrmals Tippen, um direkt in einen höheren state zu kommen
             if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] == 1){
               SystemCall::say("I go to the default pose and then I point at the center circle");
               OUTPUT_TEXT("I go to the default pose and then I point at the center circle");
               goto state1; // gehe direkt zum Startpunkt dann zeige auf Mittelkreis
             }
        else if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] == 2){
               SystemCall::say("I go to the center circle and then I point at the ball");
               OUTPUT_TEXT("I go to the center circle and then I point at the ball");
               goto state2; // gehe direkt zum Mittelkreis dann zeige auf Ball
             }
        else if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] == 3) {
               SystemCall::say("I go to the ball and then I point at the right goalpost");
               OUTPUT_TEXT("I go to the ball and and then I point at the right goalpost.");
               goto state3; // gehe direkt zum Ball dann zeige auf rechten Torpfosten
             }
        else isActive = false;
      }
    }

    state(state1)
    {
      action
      {

        // Relative Koordinate der Startposition
        Pose2f DefaultPose_relativeCoordinate = theRobotPose.toRelative(theDefaultPose.ownDefaultPose);

        if (!hasReachedDefaultPose && (std::abs(DefaultPose_relativeCoordinate.translation.x()) > 0.8f ||
                                       std::abs(DefaultPose_relativeCoordinate.translation.y()) > 0.8f))
        {
          // Laufe zur Startposition
          theWalkToPointSkill(DefaultPose_relativeCoordinate, 0.7f, false, false, false, true);
        }

        else
        {
          hasReachedDefaultPose = true;

          theStandSkill();

          // Relative Koordinate des Mittelkreises
          Vector2f centerCircle_relativeCoordinate = theRobotPose.toRelative(Vector2f(0.f, 0.f));

          // Die Z-Koordinate auf 0 setzen, um einen Vector3f zu erstellen
          Vector3f centerCircle_relativeCoordinate3D(centerCircle_relativeCoordinate.x(), centerCircle_relativeCoordinate.y(), 0.f);

          // Zeige auf den Mittelkreise
          thePointAtSkill(centerCircle_relativeCoordinate3D);
        }
         
        // Head Motion Request
        theLookActiveSkill();
        
      }
        transition
      {
        if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] > 0){
          SystemCall::say("I go to the center circle and then I point at the ball");
          OUTPUT_TEXT("I go to the center circle and then I point at the ball");
          goto state2;
        }
      }
    }

    state(state2)
    {
      action
      {

        // Relative Koordinate des Mittelkreises
        Pose2f centerCircle_relativeCoordinate = theRobotPose.toRelative(Pose2f(0_deg, 0.f, 0.f));

        if (!hasReachedCenterCircle && (std::abs(centerCircle_relativeCoordinate.translation.x()) > 0.8f ||
                                        std::abs(centerCircle_relativeCoordinate.translation.y()) > 0.8f))
        {
          // Laufe zum Mittelkreis
          theWalkToPointSkill(centerCircle_relativeCoordinate, 0.7f, false, false, false, true);
        }

        else
        {
          hasReachedCenterCircle = true;

          theStandSkill();

          // Relative Koordinate des Balls
          Vector2f ball_relativeCoordinate = theRobotPose.toRelative(theFieldBall.recentBallPositionOnField());

          // Die Z-Koordinate auf 0 setzen, um einen Vector3f zu erstellen
          Vector3f ball_relativeCoordinate3D(ball_relativeCoordinate.x(), ball_relativeCoordinate.y(), 0.f);

          // Zeige auf den Ball
          thePointAtSkill(ball_relativeCoordinate3D);
        }

        // Head Motion Request
        theLookActiveSkill();

      }
        transition
      {
        if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] > 0) {
          SystemCall::say("I go to the ball and then I point at the right goalpost");
          OUTPUT_TEXT("I go to the ball and and then I point at the right goalpost.");
          goto state3;
        }
      }
    }

    state(state3)
    {
      action
      {

        // Relative Koordinate des Balls
        Pose2f ball_relativeCoordinate = theRobotPose.toRelative(theFieldBall.recentBallEndPositionOnField());
        //OUTPUT_TEXT(ball_relativeCoordinate.translation.x() << ", " << ball_relativeCoordinate.translation.y() << ", " << ball_relativeCoordinate.rotation.toDegrees());

        // Nao muss Abstand zum Ball halten, da er sonst den Ball schießt und ihm hinterherläuft
        if (!hasReachedBall && (std::abs(ball_relativeCoordinate.translation.x()) > 250.f || 
                                std::abs(ball_relativeCoordinate.translation.y()) > 250.f))
        {
          // Laufe zum Ball
          theWalkToPointSkill(ball_relativeCoordinate, 0.7f, false, false, false, true);
        }

        else
        {
          hasReachedBall = true;
          
          theStandSkill();

          // Relative Koordinate des rechten Torpfostens
          Vector2f goalpost_relativeCoordinate = theRobotPose.toRelative(Vector2f(4500.f, -800.f));

          // Die Z-Koordinate auf 0 setzen, um einen Vector3f zu erstellen
          Vector3f goalpost_relativeCoordinate3D(goalpost_relativeCoordinate.x(), goalpost_relativeCoordinate.y(), 0.f);

          // Zeige auf rechten Torpfosten
          thePointAtSkill(goalpost_relativeCoordinate3D);
        }

        // Head Motion Request
        theLookForwardSkill();

      }
        transition
      {
        if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] > 0) {
          SystemCall::say("Calibration card completed");
          OUTPUT_TEXT("Calibration card completed");
          isActive = false;
        }
      }
    }

  }
};

MAKE_CARD(PointingCalibrationCard);