/**
 * @file PointingCard.cpp
 * @author Jobr1005@stud.hs-kl.de
 * @date 2024-12-01
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
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/MotionControl/ArmMotionRequest.h"

#include "Modules/Infrastructure/WhistleHandler/WhistleHandler.h"
#include "Platform/SystemCall.h"



CARD(PointingCard,
  {,
     CALLS(Activity),
     CALLS(LookForward),
     CALLS(Stand),
     CALLS(PointAt),
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
        (int)(90) fieldOfViewAngle,
        (int)(3000) whistleTimeout,
        (float)(0.5f) confidenceThreshold,

        //Um auf festen Punkt zu zeigen
        //(Vector2f)(-4500.f, 0.f) pointPosition,
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
    
    //Triggern auf Pfeife
    bool whistleDetected = (theFrameInfo.getTimeSince(theWhistle.lastTimeWhistleDetected) < whistleTimeout)
      && (theWhistle.confidenceOfLastWhistleDetection >= confidenceThreshold);

    int timeSinceLastStart = theFrameInfo.getTimeSince(startTime);

    return whistleDetected && isActive && (timeSinceLastStart < maxRuntime || timeSinceLastStart > maxRuntime + cooldown);
    

    /*alternative
    //Karte triggert bei ownFreeKick
    int timeSinceLastStart = theFrameInfo.getTimeSince(startTime);

    return isActive && (timeSinceLastStart < maxRuntime || timeSinceLastStart > maxRuntime + cooldown)
      && theGameInfo.setPlay == SET_PLAY_CORNER_KICK
      && theTeammateRoles.isTacticalOffense(theRobotInfo.number);
    */


    /*alternative
    //Karte kann erst wieder aufgerufen werden, wenn cooldown abgelaufen ist
    int timeSinceLastStart = theFrameInfo.getTimeSince(startTime);

    return isActive && (timeSinceLastStart < maxRuntime || timeSinceLastStart > maxRuntime + cooldown);
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

        //Um auf Ball zu zeigen
        Vector2f pointPosition = theFieldBall.recentBallPositionOnField();

        Vector2f relativePointPosition = theRobotPose.toRelative(pointPosition);
        Vector3f relativePointCoordinate3D(relativePointPosition.x(), relativePointPosition.y(), 0.f);
        //OUTPUT_TEXT("Point Position (x, y): " << relativePointPosition.x() << ", " << relativePointPosition.y());
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

        //Um auf Ball zu zeigen
        Vector2f pointPosition = theFieldBall.recentBallPositionOnField();

        Vector2f relativePointPosition = theRobotPose.toRelative(pointPosition);
        //OUTPUT_TEXT("Turning to point (x, y):  " << relativePointPosition.x() << ", " << relativePointPosition.y());
        //OUTPUT_TEXT("Robot Position: " << -theRobotPose.translation.x() << ", " << -theRobotPose.translation.y() << ", " << 180 + theRobotPose.rotation.toDegrees());
        theTurnToPointSkill(relativePointPosition);
      }
    }
  }

  bool isPointInFieldOfView()
  {

    //Um auf Ball zu zeigen
    Vector2f pointPosition = theFieldBall.recentBallPositionOnField();

    // Berechne die Differenz der x- und y-Koordinaten zwischen Ihrer Position und der des Punktes
    double dx = pointPosition.x() - theRobotPose.translation.x();
    double dy = pointPosition.y() - theRobotPose.translation.y();

    // Berechne den Winkel des Punktes relativ zur Position des Roboters
    double anglePoint = fmod((atan2(dy, dx) * (180.0 / M_PI) + 360.0), 360.0);
    double rotation = theRobotPose.rotation.toDegrees();

    // Berechne den minimalen und maximalen Winkel des zeigbaren Bereichs
    double minAngle = fmod((rotation - fieldOfViewAngle + 360.0), 360.0);
    double maxAngle = fmod((rotation + fieldOfViewAngle + 360.0), 360.0);

    // Überprüfe, ob der Winkel des Balls innerhalb des zeigbaren Bereichs liegt
    if (minAngle < maxAngle)
      return minAngle <= anglePoint && anglePoint <= maxAngle;
    else
      return anglePoint >= minAngle || anglePoint <= maxAngle;
  }

};

MAKE_CARD(PointingCard);