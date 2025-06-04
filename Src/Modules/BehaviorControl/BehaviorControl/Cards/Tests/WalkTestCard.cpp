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

  // activates on mode switch
  bool preconditions() const override
  {
    return theRobotInfo.mode == RobotInfo::walktest;
  }

  bool postconditions() const override
  {
    return !preconditions();  
  }

  // check whether the robot position is close enough on a target (epsilon-environment), set epsilon within this function
  bool targetCloseEnough(Pose2f target)
  {
    double eps = 25.f; // epsilon value for self location precision
    if(theRobotPose.translation.x() > (target.translation.x() - eps) && 
        theRobotPose.translation.x() < (target.translation.x() + eps) &&
        theRobotPose.translation.y() > (target.translation.y() - eps) &&
        theRobotPose.translation.y() < (target.translation.y() + eps))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  option
  {
    theActivitySkill(BehaviorStatus::testingBehavior);

    initial_state(walkToStart)
    {
      // set starting point for walk test half a meter in front of own penalty mark
      Pose2f startPos = Pose2f(0_deg, theFieldDimensions.xPosOwnPenaltyMark - 500.f , 0.f);
      transition
      {
        theSaySkill("Walking to start.");
        if(targetCloseEnough(startPos))
        {
          goto walkForward;
        }
      }

      action
      {
        // target the starting point 
        Pose2f targetStartPos = startPos + theRobotPose.inversePose;
        // walk to starting point penaltymark
        theWalkToPointSkill(targetStartPos , 1.0f, false, false, true);
        theLookForwardSkill(); // Head Motion Request
      }
    }


    state(walkForward)
    {
      // set target of forward walk half a meter in front of center point
      Pose2f forwardTarget = Pose2f(0_deg, theFieldDimensions.xPosHalfWayLine - 500.f, 0);
      transition
      {
        theSaySkill("Now walking forward.");
        // trigger next state when reaching a distance of 30cm in front of middle point
        if(targetCloseEnough(forwardTarget))
        {
          goto walkSidewardRight;
        }
      }

      action
      {
        // targeting by adding inverse robot pose
        Pose2f targetForward = forwardTarget + theRobotPose.inversePose;
        // execute walk
        theWalkToPointSkill(targetForward , 1.0f, false, false, true);
        
        theLookForwardSkill(); // Head Motion Request
      }
    }

    state(walkSidewardRight)
    {
      // set target of this sideward walk half a meter from the right border aswell as half a meter from the middle line
      Pose2f rightTarget = Pose2f(0_deg, theFieldDimensions.xPosHalfWayLine - 500.f, theFieldDimensions.yPosRightSideline + 500.f);
      transition
      {
        theSaySkill("Now walking right.");
        if(targetCloseEnough(rightTarget))
        {
          goto walkBackward;
        }
      }

      action
      {
        // targeting 
        Pose2f targetRight = rightTarget + theRobotPose.inversePose;
        // execute walk
        theWalkToPointSkill(targetRight , 1.0f, false, false, true);
        
        theLookForwardSkill(); // Head Motion Request
      }
    }
    
    state(walkBackward)
    {
      Pose2f backwardsTarget = Pose2f(0_deg, theFieldDimensions.xPosOwnPenaltyMark - 500.f, theFieldDimensions.yPosRightSideline + 500.f);
      transition
      {
        theSaySkill("Now walking backwards.");
        if(targetCloseEnough(backwardsTarget))
        {
          goto walkSidewardLeft;
        }
      }

      action
      {
        // targeting
        Pose2f targetBackwards = backwardsTarget + theRobotPose.inversePose;
        // walk backward
        theWalkToPointSkill(targetBackwards , 1.0f, false, false, true);
        
        theLookForwardSkill(); // Head Motion Request
      }
    }

    state(walkSidewardLeft)
    {
      Pose2f leftTarget = Pose2f(0_deg, theFieldDimensions.xPosOwnPenaltyMark - 500.f , 0.f);
      transition
      {
        theSaySkill("Now walking left.");
        if((theRobotPose.translation.y()) >= -10.f)
        {
          goto done;
        }
      }

      action
      {
        // target area behind penalty mark 
        Pose2f targetLeft = leftTarget + theRobotPose.inversePose;
        // walk left
        theWalkToPointSkill(targetLeft , 1.0f, false, false, true);
        
        theLookForwardSkill(); // Head Motion Request
      }
    }

    state(done)
    {
      transition
      {
        theSaySkill("Walking test done.");
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
