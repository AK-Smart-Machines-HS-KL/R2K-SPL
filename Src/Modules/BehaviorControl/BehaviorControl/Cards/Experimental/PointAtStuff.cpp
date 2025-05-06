/**
 * @file PointAtStuff.cpp
 * @author Wilhelm Simus, Vorlage von Jonathan Brauch
 * @brief Make the robot point at objects and locations
 * @version 0.1
 * @date 2025-05-06
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

CARD(TestCard,
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
             }),
     });

class PointAtStuffCard : public PointAtStuffCardBase
{
  private:
    mutable bool isActive = false;

  // Card becomes active, if robots head rear button is touched
  bool preconditions() const override
  {
    if (theEnhancedKeyStates.hitStreak[KeyStates::headRear] > 0)
    {
      if (isActive)
        isActive = false;
      else
        isActive = true;
    }

    return isActive;
  }

  bool postconditions() const override
  {
    return !isActive;   // pointing card should become inactive, after toggling head rear button again
  }

  void execute() override
  {

    theActivitySkill(BehaviorStatus::testingBehavior);

    theStandSkill();

    // Relative coordinates of ball
    Vector2f ball_relativeCoordinate = theRobotPose.toRelative(theFieldBall.recentBallPositionOnField());

    // set Z-coordinates to 0 for Vector3f
    Vector3f ball_relativeCoordinate3D(ball_relativeCoordinate.x(), ball_relativeCoordinate.y(), 0.f);

    // point at calculated ball position
    thePointAtSkill(ball_relativeCoordinate3D);

  }
};

MAKE_CARD(PointAtStuffCard);
