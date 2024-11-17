/**
 * @file PointingCard.cpp
 * @author Jonathan Brauch
 * @version 3.0
 * @date 2024-06-03
 *
 *
 */

 // Skills - Must be included BEFORE Card Base
#include "Representations/BehaviorControl/Skills.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Modules\Infrastructure\WhistleHandler\WhistleHandler.h"

// Representations
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"
#include "Representations/Modeling/RobotPose.h"
#include "Platform/SystemCall.h"


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
     REQUIRES(RobotInfo),
     REQUIRES(GameInfo),
     REQUIRES(TeammateRoles),
     REQUIRES(RobotPose),
     REQUIRES(Whistle),


     DEFINES_PARAMETERS(
          {,
       //Define Params here
       (int)(10000) cooldown,
       (unsigned)(0) startTime,
       (int)(10000) maxRuntime,
       (bool)(false) say,
       (int)(2000) whistleTimeout,
       (float)(0.5f) confidenceThreshold,
    }),

  });

class PointingCard : public PointingCardBase
{
private:
  bool isActive = true;

public:
  // always active
  bool preconditions() const override
  {
    // Überprüfen, ob eine Pfeife erkannt wurde
    bool whistleDetected = (theFrameInfo.getTimeSince(theWhistle.lastTimeWhistleDetected) < whistleTimeout) &&
      (theWhistle.confidenceOfLastWhistleDetection >= confidenceThreshold);

    int timeSinceLastStart = theFrameInfo.getTimeSince(startTime);

    return whistleDetected && isActive && (timeSinceLastStart < maxRuntime || timeSinceLastStart > maxRuntime + cooldown);
    /*
    //WhistleHandler whistleHandler;
     //return whistleHandler.isWhistleDetected();
    
    
    int timeSinceLastStart = theFrameInfo.getTimeSince(startTime);

    return isActive && (timeSinceLastStart < maxRuntime || timeSinceLastStart > maxRuntime + cooldown);
    //  && theGameInfo.setPlay == SET_PLAY_CORNER_KICK
    //  && theTeammateRoles.isTacticalOffense(theRobotInfo.number);
    */
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
        if (preconditions())
        {
          goto pointing;
        }
      }
    }

    state(pointing)
    {
      transition
      {
        if (preconditions())
        {
          if (isPointInFieldOfView())
          {
            goto pointAtPoint;
          }
          else
          {
            goto turningToPoint;
          }
        }
      }

      action
      {
        // Head Motion Request
        theLookForwardSkill();
        theStandSkill();
      }
    }

    state(pointAtPoint)
    {
      transition
      {
        if (!isPointInFieldOfView())
        {
          goto turningToPoint;
        }
      }

      action
      {
        theLookForwardSkill();
        theStandSkill();

        //Vector2f PointPosition(-4500.f, 0.f);
        Vector2f PointPosition = theFieldBall.recentBallPositionOnField();

        Vector2f relativePointPosition = theRobotPose.toRelative(PointPosition);
        Vector3f relativePointCoordinate3D(relativePointPosition.x(), relativePointPosition.y(), 0.f);
        OUTPUT_TEXT("Point Position (x, y): " << relativePointPosition.x() << ", " << relativePointPosition.y());
        thePointAtSkill(relativePointCoordinate3D);
      }
    }

    state(turningToPoint)
    {
      transition
      {
        if (isPointInFieldOfView())
        {
          goto pointAtPoint;
        }
      }

      action
      {
        theLookForwardSkill();
      if (!say) {
        SystemCall::say("ready");
        SystemCall::say("set");
        SystemCall::say("playing");
        say = true;
      }
      //Vector2f PointPosition(-4500.f, 0.f);
      Vector2f PointPosition = theFieldBall.recentBallPositionOnField();

      Vector2f relativePointPosition = theRobotPose.toRelative(PointPosition);
      //OUTPUT_TEXT("Turning to point (x, y):  " << relativePointPosition.x() << ", " << relativePointPosition.y());
      //OUTPUT_TEXT("Robot Position: " << -theRobotPose.translation.x() << ", " << -theRobotPose.translation.y() << ", " << 180 + theRobotPose.rotation.toDegrees());
      theTurnToPointSkill(relativePointPosition);
    }
  }
  }

    bool isPointInFieldOfView()
  {
    // Berechne die Differenz der x- und y-Koordinaten zwischen Ihrer Position und der des Punktes
    //double dx = -4500.f - theRobotPose.translation.x();
    //double dy = 0.f - theRobotPose.translation.y();

    // Berechne die Differenz der x- und y-Koordinaten zwischen Ihrer Position und der des Balls
    double dx = theFieldBall.recentBallPositionOnField().x() - theRobotPose.translation.x();
    double dy = theFieldBall.recentBallPositionOnField().y() - theRobotPose.translation.y();

    // Berechne den Winkel des Punktes relativ zur Position des Roboters
    double anglePoint = fmod((atan2(dy, dx) * (180.0 / M_PI) + 360.0), 360.0);
    double rotation = theRobotPose.rotation.toDegrees();

    // Berechne den minimalen und maximalen Winkel des zeigbaren Bereichs
    double minAngle = fmod((rotation - 90.0 + 360.0), 360.0); 
    double maxAngle = fmod((rotation + 90.0 + 360.0), 360.0);

    // Überprüfe, ob der Winkel des Balls innerhalb des zeigbaren Bereichs liegt
    if (minAngle < maxAngle)
      return minAngle <= anglePoint && anglePoint <= maxAngle;
    else
      return anglePoint >= minAngle || anglePoint <= maxAngle;
  }

};

MAKE_CARD(PointingCard);