/**
 * @file PointingCard.cpp
 * @author Jonathan Brauch
 * @version 1.5
 * @date 2024-03-21
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
       (int)(300000) maxRuntime,
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

    return isActive && (timeSinceLastStart < maxRuntime || timeSinceLastStart > maxRuntime + cooldown)
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
        goto pointing;
      }

      action
      {
        // Head Motion Request
        theLookForwardSkill();

        //OUTPUT_TEXT("Test pointing");

      
        // Absolute Koordinate
        //Vector2f absoluteCoordinate(-4500.f, 0.f);
        Vector2f absoluteCoordinate = theFieldBall.recentBallPositionOnField();
      
        OUTPUT_TEXT("Absolute Coordinate: " << absoluteCoordinate.x() << ", " << absoluteCoordinate.y());

        // Wandele absolute Koordinate in relative Koordinate um
        Vector2f relativeCoordinate = theRobotPose.toRelative(absoluteCoordinate);
        OUTPUT_TEXT("Relative Coordinate: " << relativeCoordinate.x() << ", " << relativeCoordinate.y());
       
        // Die Z-Koordinate auf 0 setzen, um einen Vector3f zu erstellen
        Vector3f relativeCoordinate3D(relativeCoordinate.x(), relativeCoordinate.y(), 0.f);
       // OUTPUT_TEXT("Relative Coordinate3D: " << relativeCoordinate3D.x() << ", " << relativeCoordinate3D.y() << ", " << relativeCoordinate3D.z());

        // Die relative Ballposition im theTurnToPointSkill verwenden
        theTurnToPointSkill(relativeCoordinate);
        OUTPUT_TEXT("Robot Rotation: " << theRobotPose.rotation.toDegrees());

        // Zeige auf die relative Koordinate
        thePointAtSkill(relativeCoordinate3D);



        // Mit festen Koordinaten
        // theTurnToPointSkill(Vector2f(-900.f, -500.f));
        // thePointAtWithArmSkill(Vector3f(-1000.f, 0.f, 0.f), Arms::right);
        // thePointAtWithArmSkill(Vector3f(-1000.f, 0.f, 300.f), Arms::left);
      }
    }
  }
};

MAKE_CARD(PointingCard);