/**
 * @file PointingCard.cpp
 * @author Jonathan Brauch
 * @version 2.0
 * @date 2024-05-13
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


     DEFINES_PARAMETERS(
          {,
       //Define Params here
       (int)(10000) cooldown,
       (unsigned)(0) startTime,
       (int)(10000) maxRuntime,
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

    return isActive && (timeSinceLastStart < maxRuntime || timeSinceLastStart > maxRuntime + cooldown);
    && theGameInfo.setPlay == SET_PLAY_CORNER_KICK
    && theTeammateRoles.isTacticalOffense(theRobotInfo.number);
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
          if (isBallInFieldOfView())
          {
            // Ball ist im zeigbaren Bereich
            goto pointAtBall;
          }
          else
          {
            // Ball ist nicht im zeigbaren Bereich
            goto turnToPointAndPointAtBall;
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

    state(pointAtBall)
    {
      transition
      {
        if (!isBallInFieldOfView())
        {
          // Ball ist nicht mehr im zeigbaren Bereich
          goto turningToPoint;
        }
      }

      action
      {
        theLookForwardSkill();
        theStandSkill();

        // Point at the ball
        Vector2f ballPosition = theFieldBall.recentBallPositionOnField();
        Vector2f relativeballPosition = theRobotPose.toRelative(ballPosition);
        Vector3f relativeBallCoordinate3D(relativeballPosition.x(), relativeballPosition.y(), 0.f);
        OUTPUT_TEXT("Ball Position (x, y): " << relativeballPosition.x() << ", " << relativeballPosition.y());
        thePointAtSkill(relativeBallCoordinate3D);
      }
    }

    state(turningToPoint)
    {
      transition
      {
        if (isBallInFieldOfView())
        {
          // Ball ist wieder im zeigbaren Bereich
          goto pointAtBall;
        }
      }

      action
      {
        theLookForwardSkill();

      // Turn to point at the ball
      Vector2f ballPosition = theFieldBall.recentBallPositionOnField();
      Vector2f relativeBallPosition = theRobotPose.toRelative(ballPosition);
      OUTPUT_TEXT("Turning to point at the ball. Relative Ball Position (x, y): " << relativeBallPosition.x() << ", " << relativeBallPosition.y());
      theTurnToPointSkill(relativeBallPosition);
    }
  }

  state(turnToPointAndPointAtBall)
  {
    transition
    {
      if (isBallInFieldOfView())
      {
        // Ball ist wieder im zeigbaren Bereich
        goto pointAtBall;
      }
    }

    action
    {
      // Turn to point at the ball
      Vector2f ballPosition = theFieldBall.recentBallPositionOnField();
      Vector2f relativeBallPosition = theRobotPose.toRelative(ballPosition);
      OUTPUT_TEXT("Turning to point at the ball. Relative Ball Position (x, y): " << relativeBallPosition.x() << ", " << relativeBallPosition.y());
      theTurnToPointSkill(relativeBallPosition);

      // Point at the ball
      Vector3f relativeBallPosition3D(relativeBallPosition.x(), relativeBallPosition.y(), 0.f);
      OUTPUT_TEXT("Pointing at the ball. Relative Ball Position (x, y, z): " << relativeBallPosition3D.x() << ", " << relativeBallPosition3D.y() << ", " << relativeBallPosition3D.z());
      thePointAtSkill(relativeBallPosition3D);
    }
  }
  }

    bool isBallInFieldOfView()
  {
    // Berechne die Differenz der x- und y-Koordinaten zwischen Ihrer Position und der des Balls
    double dx = theFieldBall.recentBallPositionOnField().x() - theRobotPose.translation.x();
    double dy = theFieldBall.recentBallPositionOnField().y() - theRobotPose.translation.y();

    // Berechne den Winkel des Balls relativ zur Position des Roboters
    double angleBall = fmod((atan2(dy, dx) * (180.0 / M_PI) + 360.0), 360.0);
    // Berücksichtige die Drehung des Roboters, um den Winkelbereich des Sichtfelds zu bestimmen
    double rotation = theRobotPose.rotation.toDegrees();

    // Berechne den minimalen und maximalen Winkel des zeigbaren Bereichs
    double minAngle = fmod((rotation - 90.0 + 360.0), 360.0); // Annahme: Sichtfeld von 180 Grad
    double maxAngle = fmod((rotation + 90.0 + 360.0), 360.0);

    // Überprüfe, ob der Winkel des Balls innerhalb des zeigbaren Bereichs liegt
    if (minAngle < maxAngle)
    {
      bool isInFOV = minAngle <= angleBall && angleBall <= maxAngle;
      OUTPUT_TEXT("Ball is in field of view: " << (isInFOV ? "Yes" : "No"));
      return isInFOV;
    }
    else
    {
      bool isInFOV = angleBall >= minAngle || angleBall <= maxAngle;
      OUTPUT_TEXT("Ball is in field of view: " << (isInFOV ? "Yes" : "No"));
      return isInFOV;
    }
  }

};

MAKE_CARD(PointingCard);