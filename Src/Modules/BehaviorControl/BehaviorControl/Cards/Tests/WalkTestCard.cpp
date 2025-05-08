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

//#include <filesystem>

// Modify this card but don't commit changes to keep it clean for other developers
// Also don't forget to put this card at the top of your Card Stack!
CARD(WalkTestCard,
     {
        ,
        REQUIRES(RobotPose),
        REQUIRES(FieldDimensions),

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
    return true;
  }

  bool postconditions() const override
  {
    return false;   // set to true, when used as default card, ie, lowest card on stack
  }

  option
  {
    theActivitySkill(BehaviorStatus::testingBehavior);

    initial_state(walkForward)
    {
      transition
      {
        theSaySkill("I start walking forward from the penalty point.");
        if(theRobotPose.translation.x() >= 0)
        {
          theSaySkill("I have reached the field center point. Now walking right.");
          goto walkSidewardRight;
        }
      }

      action
      {
        // targeting the middle point in robot relative coordinates, overshooting by half the width of the field lines
        Pose2f targetMiddlePoint = Pose2f(0_deg, theFieldDimensions.xPosHalfWayLine + 25.f, 0) + theRobotPose.inversePose;
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
          theSaySkill("Now I have reached the right sideline and I'm walking backwards until I'm at about the height of the penalty point.");
          goto walkBackward;
        }
      }

      action
      {
        // targeting the right field border
        Pose2f targetRightBorder = Pose2f(0_deg, theFieldDimensions.xPosHalfWayLine, theFieldDimensions.yPosRightSideline - 25.f) + theRobotPose.inversePose;
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
          theSaySkill("I reached the height of the penalty point. Now walking left towards it.");
          goto walkSidewardLeft;
        }
      }

      action
      {
        // target right field border 
        Pose2f targetBehind = Pose2f(0_deg, theFieldDimensions.xPosOwnPenaltyMark - 25.f, theFieldDimensions.yPosRightSideline) + theRobotPose.inversePose;
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
          theSaySkill("I'm back at the starting point. Walking test done.");
          goto done;
        }
      }

      action
      {
        // target Penaltymark 
        Pose2f targetPenaltyMark = Pose2f(0_deg, theFieldDimensions.xPosOwnPenaltyMark, 25.f) + theRobotPose.inversePose;
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
