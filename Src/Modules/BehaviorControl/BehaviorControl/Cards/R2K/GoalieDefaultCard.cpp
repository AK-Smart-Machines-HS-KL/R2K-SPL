/**
 * @file GoalieDefaultCard.cpp
 * @author Andy Hobelsberger    
 * @brief This card's Positions the Goalie and makes it search for the ball if it can't see it. 
 * @version 1.0
 * @date 2023-01-31
 * 
 * This Card Dows two things: 
 * 1. It searches for the ball if it can not be found 
 * 2. It places the goalie in a position between the ball and the goal
 * 
 */

// Skills - Must be included BEFORE Card Base
#include "Representations/BehaviorControl/Skills.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

// Representations
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Modeling/RobotPose.h"

CARD(GoalieDefaultCard,
     {
        ,
        CALLS(Activity),
        CALLS(Stand),
        CALLS(LookActive),
        CALLS(LookAtBall),
        CALLS(WalkAtRelativeSpeed),
        REQUIRES(TeammateRoles), 
        REQUIRES(RobotInfo),
        REQUIRES(RobotPose),
        REQUIRES(FieldBall),
        REQUIRES(FieldDimensions),

        DEFINES_PARAMETERS(
             {,
                (unsigned int) (1000) ballSeenTimeout,
                (Vector2f) (Vector2f(-3500, 1500)) goalAreaUpperBound,
                (Vector2f) (Vector2f(-4800, -1500)) goalAreaLowerBound,
                (float) (2000) blockingArcDepth,
                (int) (400) goalposesafetyarea,
             }),
     });

class GoalieDefaultCard : public GoalieDefaultCardBase
{
  bool ballPosLost;
  Angle lowerBlockingArcLimit;
  Angle upperBlockingArcLimit;
  Vector2f blockingArcCenter;
  float blockingArcRadius;

  bool isNearGoal() const{
    return theRobotPose.translation.x() > goalAreaLowerBound.x()
      && theRobotPose.translation.x() < goalAreaUpperBound.x()
      && theRobotPose.translation.y() > goalAreaLowerBound.y()
      && theRobotPose.translation.y() < goalAreaUpperBound.y();       
  }

  // Condition: Is tacticalGoalKeeper (From TeammateRoles) && is near Goal
  bool preconditions() const override
  {
    return theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number) && isNearGoal();
  }

  bool postconditions() const override
  {
    return true;   // i.e. When preconditions are false, card exits. Equivalent to !preconditions()
  }

  option
  {
    initial_state(init)
    {
      // Calculate parameters for blocking positioning
      blockingArcCenter = Vector2f(theFieldDimensions.xPosOwnGroundLine - blockingArcDepth, 0);
      blockingArcRadius = (Vector2f(theFieldDimensions.xPosOwnGoalPost, theFieldDimensions.yPosRightGoal) - blockingArcCenter).norm();
      lowerBlockingArcLimit = (Vector2f(theFieldDimensions.xPosOwnGoalPost, theFieldDimensions.yPosRightGoal + goalposesafetyarea) - blockingArcCenter).angle();
      upperBlockingArcLimit = (Vector2f(theFieldDimensions.xPosOwnGoalPost, theFieldDimensions.yPosLeftGoal - goalposesafetyarea) - blockingArcCenter).angle();

      ballPosLost = false;

      transition
      {
        goto findBall;
      }

      action {
        theActivitySkill(BehaviorStatus::defaultBehavior);
      }
    }

    state(findBall)
    {
      transition
      {
        if (theFieldBall.ballWasSeen(ballSeenTimeout)) {
           goto block; 
        }
      }

      action
      {        
        Angle angleToBallEstimate = Angle::normalize(theFieldBall.recentBallPositionRelative().angle());
        
        if (std::abs(angleToBallEstimate) < 5_deg) {
          theStandSkill();
        } else {
          theWalkAtRelativeSpeedSkill(Pose2f(std::clamp((float)angleToBallEstimate, -1.0f, 1.0f)));
        }

        theLookActiveSkill();
        theActivitySkill(BehaviorStatus::searchingForBall);
      }
    }

    state(block)
    {
      transition
      {
        if (!theFieldBall.ballWasSeen(ballSeenTimeout)) {
           goto findBall; 
        }
      }

      action
      {
        theActivitySkill(BehaviorStatus::blocking);
        Angle blockingArcAngle = std::clamp((theFieldBall.positionOnField - blockingArcCenter).angle(), (float) lowerBlockingArcLimit, (float) upperBlockingArcLimit);
        Vector2f blockingPos = Vector2f(blockingArcRadius, 0).rotated(blockingArcAngle) + blockingArcCenter;
        Vector2f blockingPosRelative = theRobotPose.toRelative(blockingPos);

        theLookAtBallSkill();
        
        if (blockingPosRelative.norm() > 100) {
          theWalkAtRelativeSpeedSkill(Pose2f(Angle::normalize(theFieldBall.positionRelative.angle()), blockingPosRelative.normalized())); // Walk to blockingPos AND turn to ball
        } else if (Angle::normalize(theFieldBall.positionRelative.angle()) > 5_deg) {
          theWalkAtRelativeSpeedSkill(Pose2f(Angle::normalize(theFieldBall.positionRelative.angle()))); // Just turn to ball
        } else {
          theStandSkill();
        }
      }
    }
  }
};

MAKE_CARD(GoalieDefaultCard);
