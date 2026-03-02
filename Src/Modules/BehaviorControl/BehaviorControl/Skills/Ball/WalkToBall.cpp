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
#include "Representations/MotionControl/MotionInfo.h"
#include "Tools/BehaviorControl/Framework/Skill/CabslSkill.h"

SKILL_IMPLEMENTATION(WalkToBallImpl,
{,
  IMPLEMENTS(WalkToBall),
  REQUIRES(FieldBall),
  REQUIRES(MotionInfo),
  CALLS(WalkAtRelativeSpeed),
  CALLS(Stand),
  DEFINES_PARAMETERS(
  {,
    (float)(200.f) ballReachedDistance, /**< Distance threshold to consider ball as reached (in mm). */
  }),
});

class WalkToBallImpl : public WalkToBallImplBase
{
  option(WalkToBall)
  {
    const float ballDistance = theFieldBall.endPositionRelative.norm();

    initial_state(walkToBall)
    {
      transition
      {
        if(ballDistance < ballReachedDistance)
          goto ballReached;
      }

      action
      {
        // Walk with constant forward speed toward the ball
        // Use unit vector in ball direction as the walking target
        const Vector2f ballDirection = ballDistance > 0.f ? theFieldBall.endPositionRelative.normalized() : Vector2f(1.f, 0.f);
        theWalkAtRelativeSpeedSkill(Pose2f(0.f, ballDirection.x() * 0.8f, ballDirection.y() * 0.3f));
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
