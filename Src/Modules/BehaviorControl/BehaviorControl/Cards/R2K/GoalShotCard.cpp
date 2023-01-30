/**
 * @file GoalShotCard.cpp
 * @author Andy Hobelsberger    
 * @brief 
 * @version 1.0
 * 
 * 
 */

// Skills - Must be included BEFORE Card Base
#include "Representations/BehaviorControl/Skills.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

// Representations
#include "Representations/BehaviorControl/Shots.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Tools/Math/Geometry.h"

// Debug Drawings
#include "Tools/Debugging/DebugDrawings.h"

#define drawID "ObstaclesLR"

CARD(GoalShotCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookActive),
        CALLS(GoToBallAndKick),
        CALLS(Stand),
        CALLS(WalkToPoint),
        REQUIRES(Shots),
        REQUIRES(RobotPose),
        REQUIRES(FieldBall),
        REQUIRES(FrameInfo),

        DEFINES_PARAMETERS(
             {,
                (unsigned int)(1000) initalCheckTime,
                (bool)(false) done,
                (Shot) currentShot,
                (unsigned int) (0) timeLastFail,
                (unsigned int) (10000) cooldown,
             }),

     });

class GoalShotCard : public GoalShotCardBase
{
  
  void preProcess() override {
    DECLARE_DEBUG_DRAWING(drawID, "drawingOnField");
  }

  //always active
  bool preconditions() const override
  {
    return theFieldBall.positionRelative.norm() < 500 && theFrameInfo.getTimeSince(timeLastFail) > cooldown && theShots.goalShot.failureProbability < 0.95;
  }

  bool postconditions() const override
  {
    return done;   // set to true, when used as default card, ie, lowest card on stack
  }

  option
  {
    theActivitySkill(BehaviorStatus::codeReleaseKickAtGoal);

    initial_state(align)
    {
      done = false;
      Angle angleToGoal = (Vector2f(4500, 0) - theRobotPose.translation).angle() - theRobotPose.rotation; 
      transition
      {
        if(abs(angleToGoal.normalize()) < 10_deg || state_time > 2000) {
          goto check;
        }
      }

      action
      {
        theWalkToPointSkill(Pose2f(angleToGoal));
        theLookActiveSkill();
      }
    }

    state(check)
    {
      done = false;
      transition
      {
        if(state_time > initalCheckTime) {
          currentShot = theShots.goalShot;
          OUTPUT_TEXT("Locking Target: (" << currentShot.target.x() << ", " << currentShot.target.y() << ")\n" << currentShot);
          if (currentShot.failureProbability > 0.2) {
            OUTPUT_TEXT("Aborting! shot too likely to fail");
            timeLastFail = theFrameInfo.time;
            goto done;
          }
          
          goto kick;
        }
      }

      action
      {
        theLookActiveSkill();
        theStandSkill();
      }
    }

    state(kick)
    {
      transition
      {
        if(theGoToBallAndKickSkill.isDone()) {
           goto done;
        }
      }

      action
      {
        theGoToBallAndKickSkill(theRobotPose.toRelative(currentShot.target).angle(), currentShot.kickType.name);
      }
    }

    state(done)
    {
      action
      {
        reset();
        theLookActiveSkill();
        theStandSkill();
        done = true;
      }
    }
  }

  void postProcess() override {
    
  }
};

MAKE_CARD(GoalShotCard);