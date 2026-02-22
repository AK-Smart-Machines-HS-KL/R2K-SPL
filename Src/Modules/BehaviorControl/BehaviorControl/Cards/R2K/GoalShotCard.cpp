/**
 * @file GoalShotCard.cpp
 * @author Andy Hobelsberger
 * @version 1.1
 *
 * OpenPoints status:
 * - uses shared shot gating (hold/safe/precise) and ball-source logging.
 */

#include "Representations/BehaviorControl/Skills.h"

#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

#include "Representations/BehaviorControl/Shots.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Communication/TeamData.h"
#include "Representations/Communication/TeamCommStatus.h"

#include "Tools/BehaviorControl/R2KAttackLogic.h"
#include "Tools/BehaviorControl/R2KBallSourceLogic.h"
#include "Tools/BehaviorControl/R2KDecisionLog.h"
#include "Tools/Debugging/DebugDrawings.h"

#include <cmath>

#define drawID "ObstaclesLR"

CARD(GoalShotCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookActive),
        CALLS(GoToBallAndKick),
        CALLS(Stand),
        CALLS(WalkAtRelativeSpeed),
        REQUIRES(Shots),
        REQUIRES(RobotPose),
        REQUIRES(FieldBall),
        REQUIRES(FrameInfo),
        REQUIRES(TeamData),
        REQUIRES(TeamCommStatus),

        DEFINES_PARAMETERS(
             {,
                (unsigned int)(500) initalCheckTime,
                (bool)(false) done,
                (Shot) currentShot,
                (unsigned int)(0) timeLastFail,
                (unsigned int)(6000) cooldown,
                (float)(2500.f) minGoalDist,
                (unsigned int)(500) ballSeenTimeoutMs,
                (float)(250.f) forecastBallTravelThresholdMm,
             }),

     });

class GoalShotCard : public GoalShotCardBase
{
  void preProcess() override
  {
    DECLARE_DEBUG_DRAWING(drawID, "drawingOnField");
  }

  bool preconditions() const override
  {
    return theFieldBall.ballWasSeen() &&
           theRobotPose.translation.x() > 1500 &&
           theFieldBall.positionRelative.norm() < 600 &&
           theFrameInfo.getTimeSince(timeLastFail) > cooldown &&
           theShots.goalShot.failureProbability < 0.50 &&
           theFieldBall.positionOnField.x() > theRobotPose.translation.x() &&
           !aBuddyIsChasingOrClearing();
  }

  bool postconditions() const override
  {
    return !preconditions() || done;
  }

  option
  {
    theActivitySkill(BehaviorStatus::goalShotCard);

    initial_state(align)
    {
      done = false;
      const Angle angleToGoal = (Vector2f(4500.f, 0.f) - theRobotPose.translation).angle() - theRobotPose.rotation;
      transition
      {
        if(std::abs(angleToGoal.normalize()) < 20_deg || state_time > 2000)
          goto check;
      }

      action
      {
        theWalkAtRelativeSpeedSkill(Pose2f(std::clamp(static_cast<float>(angleToGoal), -1.f, 1.f)));
        theLookActiveSkill();
      }
    }

    state(check)
    {
      done = false;
      transition
      {
        if(state_time > initalCheckTime)
        {
          const bool localizationPoor = theRobotPose.quality == RobotPose::poor;
          const bool ballSeenRecently = theFieldBall.ballWasSeen(static_cast<int>(ballSeenTimeoutMs));
          const bool useForecast = (theFieldBall.endPositionRelative - theFieldBall.positionRelative).norm() > forecastBallTravelThresholdMm;
          const auto ballSource = R2KBallSourceLogic::chooseBallSource(ballSeenRecently,
                                                                        theTeamCommStatus.isWifiCommActive,
                                                                        useForecast);
          const float distanceToGoal = std::abs(4500.f - theFieldBall.endPositionOnField.x());
          const auto shotDecision = R2KAttackLogic::decideShotExecution(ballSeenRecently,
                                                                         localizationPoor,
                                                                         distanceToGoal,
                                                                         minGoalDist);

          if(shotDecision.mode == R2KAttackLogic::ShotExecutionMode::hold)
          {
            R2KDecisionLog::annotation("shot_gate", {{"card", "GoalShot"},
                                                 {"mode", "hold"},
                                                 {"reason", R2KAttackLogic::toString(shotDecision.reason)},
                                                 {"ballSource", R2KBallSourceLogic::toString(ballSource)}});
            timeLastFail = theFrameInfo.time;
            goto done;
          }

          currentShot = theShots.goalShot;
          if(currentShot.failureProbability > 0.3f && shotDecision.mode == R2KAttackLogic::ShotExecutionMode::preciseKick)
          {
            timeLastFail = theFrameInfo.time;
            goto done;
          }

          if(shotDecision.mode == R2KAttackLogic::ShotExecutionMode::safeKick)
          {
            currentShot.target = Vector2f(4500.f, 0.f);
            currentShot.kickType.name = theFieldBall.positionRelative.y() < 0.f ? KickInfo::walkForwardsLeft : KickInfo::walkForwardsRight;
            R2KDecisionLog::annotation("shot_gate", {{"card", "GoalShot"},
                                                 {"mode", "fallback"},
                                                 {"reason", R2KAttackLogic::toString(shotDecision.reason)},
                                                 {"ballSource", R2KBallSourceLogic::toString(ballSource)}});
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
        if(theGoToBallAndKickSkill.isDone())
          goto done;
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

  bool aBuddyIsChasingOrClearing() const
  {
    for(const auto& buddy : theTeamData.teammates)
    {
      if(buddy.theBehaviorStatus.activity == BehaviorStatus::clearOwnHalfCard ||
         buddy.theBehaviorStatus.activity == BehaviorStatus::clearOwnHalfGoalieCard ||
         buddy.theBehaviorStatus.activity == BehaviorStatus::defenseLongShotCard ||
         buddy.theBehaviorStatus.activity == BehaviorStatus::goalieLongShotCard ||
         buddy.theBehaviorStatus.activity == BehaviorStatus::goalShotCard ||
         buddy.theBehaviorStatus.activity == BehaviorStatus::offenseForwardPassCard)
        return true;
    }
    return false;
  }
};

MAKE_CARD(GoalShotCard);
