/**
 * @file AttackerGoalShotCard.cpp
 * @author Asrar    
 * @brief 
 * @version 1.1
 * @date 22-06-2023
 *
 * Functions, values, side effects:
 * Robot 3 and Robot 2 qualifies for making the goal
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
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/TeamData.h"
#include "Representations/Infrastructure/ExtendedGameInfo.h"
#include "Tools/Math/Geometry.h"

// Debug Drawings
#include "Tools/Debugging/DebugDrawings.h"

#define drawID "ObstaclesLR"

CARD(AttackerGoalShotCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookActive),
        CALLS(GoToBallAndKick),
        CALLS(Stand),
        CALLS(WalkToPoint),
        CALLS(WalkAtRelativeSpeed),
        REQUIRES(Shots),
        REQUIRES(ObstacleModel),
        REQUIRES(ExtendedGameInfo),
        REQUIRES(RobotPose),
        REQUIRES(FieldBall),
        REQUIRES(FrameInfo),
        REQUIRES(TeammateRoles),        // R2K
        REQUIRES(PlayerRole),           // R2K
        REQUIRES(RobotInfo),

        DEFINES_PARAMETERS(
             {,
                (unsigned int)(500) initalCheckTime,
                (bool)(false) done,
                (Shot) currentShot,
                (unsigned int) (0) timeLastFail,
                (unsigned int) (6000) cooldown,
             }),

     });

class AttackerGoalShotCard : public AttackerGoalShotCardBase
{
  
  void preProcess() override {
    DECLARE_DEBUG_DRAWING(drawID, "drawingOnField");
  }

  //always active
  bool preconditions() const override
  {
    return
        theExtendedGameInfo.timeSincePlayingStarted > 23000 &&
        theFieldBall.ballWasSeen(1000) && 
        theShots.goalShot.failureProbability < 0.6 &&
        (
          // theTeammateRoles.playsTheBall(theRobotInfo.number) &&   // I am the striker
          (theRobotInfo.number == 3 && theFieldBall.positionOnField.y() >= 0 )  || 
          (theRobotInfo.number == 2 && theFieldBall.positionOnField.y() < 0) 
          // theTeammateRoles.isTacticalOffense(theRobotIforwardFastLeftd > 25000) && (theRobotInfo.number == 2)
        )
        ;
        
      
    ;
  }

  bool postconditions() const override
  {
    
  return done || 
        (theRobotInfo.number == 2 && theFieldBall.positionOnField.y() >= 0 ) ||
        (theRobotInfo.number == 3 && theFieldBall.positionOnField.y() < 0 )
        // !preconditions() 
    ;   
  }

  option
  {
    theActivitySkill(BehaviorStatus::codeReleaseKickAtGoal);
    // OUTPUT_TEXT(theFieldBall.positionOnField.y());
    initial_state(align)
    {
      done = false;
      Angle angleToGoal = (Vector2f(4500, 0) - theRobotPose.translation).angle() - theRobotPose.rotation; 
      transition
      {
        if(abs(angleToGoal.normalize()) < 20_deg || state_time > 1500) {
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
          if (currentShot.failureProbability > 0.4) {
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

MAKE_CARD(AttackerGoalShotCard);