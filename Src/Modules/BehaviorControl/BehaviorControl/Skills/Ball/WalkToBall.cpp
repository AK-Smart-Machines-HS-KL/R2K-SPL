/**
 * @file WalkToBall.cpp
 *
 * This file implements a skill to walk toward the ball until reaching it.
 * Used by TeachIn sequences for ball approach movements.
 *
 * @author Copilot
 */

#include "Representations/BehaviorControl/BehaviorStatus.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/MotionControl/MotionInfo.h"
#include "Representations/Modeling/RobotPose.h"
#include "Tools/BehaviorControl/Framework/Skill/CabslSkill.h"

SKILL_IMPLEMENTATION(WalkToBallImpl,
{,
  IMPLEMENTS(WalkToBall),
  REQUIRES(FieldBall),
  REQUIRES(FrameInfo),
  REQUIRES(MotionInfo),
  REQUIRES(RobotPose),
  CALLS(WalkAtAbsoluteSpeed),
  CALLS(Stand),
  DEFINES_PARAMETERS(
  {,
    (float)(200.f) ballReachedDistance, /**< Distance threshold to consider ball as reached (in mm). */
    (float)(200.f) walkSpeed, /**< Walking speed towards ball in mm/s. */
    (float)(21.f) timePerMillimeter, /**< Extra time in ms per mm of ball distance (e.g., 21ms/mm = 4200ms for 200mm). */
  }),
});

class WalkToBallImpl : public WalkToBallImplBase
{
  unsigned int walkToBallStartTime = 0;
  float initialBallDistance = 0.f;
  unsigned int dynamicMaxTime = 0;

  option(WalkToBall)
  {
    const float ballDistance = theFieldBall.endPositionRelative.norm();
    const bool ballIsValid = theFieldBall.timeSinceBallWasSeen < 500;

    initial_state(walkToBall)
    {
      transition
      {
        // Only transition to ballReached on timeout (CSV maxTime) or ball reached + timeout buffer
        // The CSV maxTime is enforced by TIPlaybackCard, so honor it here too
        // Reached the ball: transition if ball is valid, very close, AND timeout buffer passed
        const unsigned int timeoutBuffer = 100;  // 100ms buffer to allow reaching motion to stabilize
        const unsigned int timeElapsed = theFrameInfo.getTimeSince(walkToBallStartTime);
        if(ballIsValid && ballDistance < ballReachedDistance && 
           dynamicMaxTime > 0 && timeElapsed >= (dynamicMaxTime - timeoutBuffer))
          goto ballReached;
        
        // Safety timeout: if dynamic timeout exceeded with buffer, give up
        if(dynamicMaxTime > 0 && timeElapsed >= dynamicMaxTime)
          goto ballReached;
      }

      action
      {
        // Initialize on first execution
        if(state_time == 0)
        {
          walkToBallStartTime = theFrameInfo.time;
          initialBallDistance = ballDistance;
          // Calculate dynamic timeout: distance * timePerMillimeter
          // This scales the timeout based on how far the ball is
          dynamicMaxTime = static_cast<unsigned int>(initialBallDistance * timePerMillimeter);
        }

        // Always walk forward towards where the ball should be
        // If ball is visible: walk towards it
        // If ball is not visible: walk forward to search
        if(ballIsValid && ballDistance > 10.f)
        {
          // Ball found: calculate walking parameters
          // Calculate bearing angle to ball (atan2 returns angle in radians)
          const float angleTowardsBall = std::atan2(theFieldBall.endPositionRelative.y(), theFieldBall.endPositionRelative.x());
          // Walk at: rotation to face ball, forward speed, zero lateral speed
          // Pose2f(rotation, forward, sideways)
          theWalkAtAbsoluteSpeedSkill(Pose2f(angleTowardsBall, walkSpeed, 0.f));
        }
        else
        {
          // Ball not found: walk straight forward at search speed
          theWalkAtAbsoluteSpeedSkill(Pose2f(0.f, walkSpeed * 0.5f, 0.f));
        }
      }
    }

    target_state(ballReached)
    {
      action
      {
        theStandSkill();
      }
    }
  }
};

MAKE_SKILL_IMPLEMENTATION(WalkToBallImpl);
