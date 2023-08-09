/**
 * @file GoalShotCard.cpp
 * @author Andy Hobelsberger    
 * @brief 
 * @version 1.0
 * 
 * Note: we have two checks for theShots.goalShot.failureProbability < x
 * x = 0.5 in pre-cond
 * x = 0.4 in state machine (aka "done")
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
#include "Representations/Communication/TeamData.h"

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
        CALLS(WalkAtRelativeSpeed),
        REQUIRES(Shots),
        REQUIRES(RobotPose),
        REQUIRES(FieldBall),
        REQUIRES(FrameInfo),
        REQUIRES(TeamData),

        DEFINES_PARAMETERS(
             {,
                (unsigned int)(500) initalCheckTime,
                (bool)(false) done,
                (Shot) currentShot,
                (unsigned int) (0) timeLastFail,
                (unsigned int) (6000) cooldown,
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
    return theFieldBall.positionRelative.norm() < 600
      && theFrameInfo.getTimeSince(timeLastFail) > cooldown
      && theShots.goalShot.failureProbability < 0.50
      && theFieldBall.positionOnField.x() > theRobotPose.translation.x()
      && !aBuddyIsChasingOrClearing()
    ;
  }

  bool postconditions() const override
  {
    return done;   
  }

  option
  {
    theActivitySkill(BehaviorStatus::goalShotCard);

    initial_state(align)
    {
      done = false;
      Angle angleToGoal = (Vector2f(4500, 0) - theRobotPose.translation).angle() - theRobotPose.rotation; 
      transition
      {
        if(abs(angleToGoal.normalize()) < 20_deg || state_time > 2000) {
          goto check;
        }
      }

      action
      {
        // face the goal
        theWalkAtRelativeSpeedSkill(Pose2f(std::clamp((float) angleToGoal, -1.f, 1.f)));
        // look around
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
          if (currentShot.failureProbability > 0.3) {
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
  bool aBuddyIsChasingOrClearing() const
    {
      for (const auto& buddy : theTeamData.teammates) 
      {
        if (// buddy.theBehaviorStatus.activity == BehaviorStatus::OffenseChaseBallCard ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::clearOwnHalfCard ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::clearOwnHalfCardGoalie ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::defenseLongShotCard ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::goalieLongShotCard ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::goalShotCard ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::offenseForwardPassCard )
          // buddy.theBehaviorStatus.activity == BehaviorStatus::offenseReceivePassCard)
          return true;
      }
      return false;
    }
};

MAKE_CARD(GoalShotCard);