/**
 * @file WalkTestCard.cpp
 * @author Dennis Fuhrmann    
 * @brief This card is for testing the mechanical capabilities of the bots with a simple walk forward, backwards and sideways.
 * @version 0.5
 * @date 2025-04-23
 * 
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
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Communication/RobotInfo.h"

//#include <filesystem>

// Modify this card but don't commit changes to keep it clean for other developers
// Also don't forget to put this card at the top of your Card Stack!
CARD(WalkTestCard,
     {
        ,
        REQUIRES(RobotPose),
        REQUIRES(FieldDimensions),
        REQUIRES(RobotInfo),

        CALLS(WalkToPoint),
        CALLS(Activity),
        CALLS(LookForward),
        CALLS(Stand),
        CALLS(Say),

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

class WalkTestCard : public WalkTestCardBase
{

  //always active
  bool preconditions() const override
  {
    return theRobotInfo.mode == RobotInfo::walktest;
  }

  bool postconditions() const override
  {
    return !preconditions();  
  }

  option
  {
    theActivitySkill(BehaviorStatus::testingBehavior);

    initial_state(walkPenaltyPoint)
    {
      transition
      {
        theSaySkill("Now walking to start.");
        if(theRobotPose.translation.x() > (theFieldDimensions.xPosOwnPenaltyMark - 50.f) &&
           theRobotPose.translation.x() < (theFieldDimensions.xPosOwnPenaltyMark + 50.f) &&
           theRobotPose.translation.y() > (-50.f) && theRobotPose.translation.y() < (50.f))
        {
          goto walkForward;
        }
      }

      action
      {
        //calc angle look to Middlepoint
        Angle angle = (theRobotPose.inversePose * Vector2f(0.f, 0.f)).angle();;
        // target Penaltymark 
        Pose2f targetPenaltyMark = Pose2f(angle, theFieldDimensions.xPosOwnPenaltyMark, 50.f) + theRobotPose.inversePose;
        // walk to starting point penaltymark
        theWalkToPointSkill(targetPenaltyMark , 1.0f, false, false, true);
      }
    }


    state(walkForward)
    {
      transition
      {
        theSaySkill("Now walking forward");
        if(theRobotPose.translation.x() >= 0)
        {
          theSaySkill("Now walking right.");
          goto walkSidewardRight;
        }
      }

      action
      {
        // targeting the middle point in robot relative coordinates, overshooting by half the width of the field lines
        Pose2f targetMiddlePoint = Pose2f(0_deg, theFieldDimensions.xPosHalfWayLine + 50.f, 0) + theRobotPose.inversePose;
        // walk to middle point
        theWalkToPointSkill(targetMiddlePoint , 1.0f, false, false, true);
        
        theLookForwardSkill(); // Head Motion Request
      }
    }

    state(walkSidewardRight)
    {
      transition
      {
        if((theRobotPose.translation.y()) <= (theFieldDimensions.yPosRightSideline))
        {
          theSaySkill("I'm walking backwards");
          goto walkBackward;
        }
      }

      action
      {
        // targeting the right field border
        Pose2f targetRightBorder = Pose2f(0_deg, theFieldDimensions.xPosHalfWayLine, theFieldDimensions.yPosRightSideline - 50.f) + theRobotPose.inversePose;
        // walk to field border
        theWalkToPointSkill(targetRightBorder , 1.0f, false, false, true);
        
        theLookForwardSkill(); // Head Motion Request
      }
    }
    
    state(walkBackward)
    {
      transition
      {
        if( (theRobotPose.translation.x()) <= (theFieldDimensions.xPosOwnPenaltyMark))
        {
          theSaySkill("Now walking left");
          goto walkSidewardLeft;
        }
      }

      action
      {
        // target right field border 
        Pose2f targetBehind = Pose2f(0_deg, theFieldDimensions.xPosOwnPenaltyMark - 50.f, theFieldDimensions.yPosRightSideline) + theRobotPose.inversePose;
        // walk backward
        theWalkToPointSkill(targetBehind , 1.0f, false, false, true);
        
        theLookForwardSkill(); // Head Motion Request
      }
    }

    state(walkSidewardLeft)
    {
      transition
      {
        if((theRobotPose.translation.y()) >= 0)
        {
          theSaySkill("Walking test done.");
          goto done;
        }
      }

      action
      {
        // target Penaltymark 
        Pose2f targetPenaltyMark = Pose2f(0_deg, theFieldDimensions.xPosOwnPenaltyMark, 50.f) + theRobotPose.inversePose;
        // walk backward
        theWalkToPointSkill(targetPenaltyMark , 1.0f, false, false, true);
        
        theLookForwardSkill(); // Head Motion Request
      }
    }

    state(done)
    {
      transition
      {
        
      }

      action
      {
        theLookForwardSkill(); // Head Motion Request
        theStandSkill();
      }
    }
  }
};

MAKE_CARD(WalkTestCard);
