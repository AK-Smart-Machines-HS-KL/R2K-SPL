/**
 * @file BallContestCard.cpp
 * @author Adrian Müller
 * @brief Escapes a ball-contest stalemate by kicking laterally to the free side.
 * @version 1.2
 * @date 2026-03
 *
 * Behaviour:
 *   Activates for any non-goalkeeper striker when an opponent has been within
 *   contestRange mm for more than contestTimeout ms.  Executes a three-phase escape:
 *   1. assess             — normal dribble toward opponent goal; monitor opponent proximity.
 *   2. escape             — lateral side-kick to the free side (away from clustered opponents).
 *   3. dribbleFollowthrough — continue dribbling toward goal; exit when opponent is no longer close.
 *
 * Free-side selection (calcFreeSide):
 *   Weights visible opponents by inverse distance and sums their y-components (robot frame).
 *   Positive sum → opponents cluster to the left → kick right.
 *   Field-boundary pressure is added to avoid kicking the ball out of bounds.
 *
 * searchForBall behaviour (v1.1):
 *   The card remembers the ball's last known field position.  On entering searchForBall it
 *   first rotates the body toward that position (phase 1, up to turnToBallTimeout ms or
 *   until heading error < 15°), then falls back to a forward walk + LookForward search.
 *
 * Persistence (v1.2):
 *   postconditions() no longer mirrors preconditions(). The card stays active through the
 *   full escape + dribble sequence even after the opponent leaves, exiting only on game-state
 *   changes, permanent role loss, or prolonged ball loss (2 × ballNotSeenTimeout).
 *   searchForBall phase 2 now uses WalkToPoint toward the ball's predicted end position
 *   (endPositionOnField) instead of a generic forward walk.
 *
 * Notes:
 *   - Only the striker (playsTheBall) executes the escape; buddies yield via aBuddyIsChasingOrClearing().
 *   - The goalKeeper is excluded because its cards handle conflict differently.
 *   - contestStartTime is reset when the card exits or the opponent leaves.
 */

// Skills - Must be included BEFORE Card Base
#include "Representations/BehaviorControl/Skills.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

#include <algorithm>
#include <cmath>

// Representations
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/Communication/TeamData.h"
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamCommStatus.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Modeling/ObstacleModel.h"


CARD(BallContestCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookActive),
        CALLS(WalkToPoint),
        CALLS(GoToBallAndDribble),
        CALLS(GoToBallAndKick),
        CALLS(WalkAtRelativeSpeed),
        USES(GameInfo),
        REQUIRES(ObstacleModel),
        REQUIRES(TeamBehaviorStatus),
        REQUIRES(RobotPose),
        REQUIRES(RobotInfo),
        REQUIRES(FieldBall),
        REQUIRES(FieldDimensions),
        REQUIRES(TeamData),
        REQUIRES(TeammateRoles),
        REQUIRES(TeamCommStatus),
        REQUIRES(FrameInfo),

        DEFINES_PARAMETERS(
             {,
                (float)(0.8f)   walkSpeed,
                (int)(5000)     ballNotSeenTimeout,
                // Opponent must be closer than this to count as a contest [mm].
                (float)(800.f)  contestRange,
                // Opponent must be within contestRange for this long before escape triggers [ms].
                (int)(2000)     contestTimeout,
                // Maximum time spent in the escape (side-kick) state [ms].
                (int)(4000)     kickTimeout,
                // Duration of the dribble followthrough after a successful side-kick [ms].
                (int)(3000)     followthroughDuration,
                // Angle from the forward direction (robot frame) used for the lateral kick [rad].
                (Angle)(80_deg) lateralKickAngle,
                // Distance from sideline [mm] below which boundary pressure is applied.
                (float)(1000.f) boundaryMargin,
                // Max time [ms] spent turning toward last ball position in searchForBall.
                (int)(1500)     turnToBallTimeout,
             }),
     });

class BallContestCard : public BallContestCardBase
{
  // Timestamp (theFrameInfo.time) of the first frame in the current contest window.
  // 0 means no opponent was close on the previous frame.
  unsigned int contestStartTime = 0;

  // Last known ball position in absolute field coordinates.
  // Updated every frame the ball is visible; used to orient during searchForBall.
  Vector2f lastBallPosition = Vector2f::Zero();

  // Ball's predicted stopping position (endPositionOnField) at last sight.
  // Points ahead of the ball along its velocity — used as walk target in searchForBall phase 2.
  Vector2f lastBallEndPosition = Vector2f::Zero();

  bool preconditions() const override
  {
    return
      theGameInfo.setPlay == SET_PLAY_NONE                                               &&
      theFieldBall.ballWasSeen(ballNotSeenTimeout)                                      &&
      !theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number)                       &&
      theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive)  &&
      theObstacleModel.opponentIsClose(contestRange);
  }

  bool postconditions() const override
  {
    // Do NOT exit on playsTheBall() flicker: during a contest the ball briefly drifts toward
    // the opponent, causing transient striker-role loss. Buddy-checks in the other chase cards
    // already block them from activating while ballContestCard is reported active, so no second
    // robot rushes in. Exit only on hard game-state changes or prolonged ball disappearance.
    return
      theGameInfo.setPlay != SET_PLAY_NONE               ||
      !theFieldBall.ballWasSeen(ballNotSeenTimeout * 2)  ||
      theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number);
  }

  option
  {
    theActivitySkill(BehaviorStatus::ballContestCard);

    // Keep last-seen ball positions current; captured before any transition fires this frame.
    if (theFieldBall.ballWasSeen(ballNotSeenTimeout))
    {
      lastBallPosition    = theFieldBall.positionOnField;
      lastBallEndPosition = theFieldBall.endPositionOnField;
    }

    // ------------------------------------------------------------------
    // assess: normal dribble; count how long an opponent has been close
    // ------------------------------------------------------------------
    initial_state(assess)
    {
      transition
      {
        if (!theFieldBall.ballWasSeen(ballNotSeenTimeout))
          goto searchForBall;
        if (theObstacleModel.opponentIsClose(contestRange))
        {
          if (contestStartTime == 0)
            contestStartTime = theFrameInfo.time;
          if (theFrameInfo.getTimeSince(contestStartTime) > contestTimeout)
            goto escape;
        }
        else
        {
          contestStartTime = 0;
        }
      }
      action
      {
        theGoToBallAndDribbleSkill(calcAngleToGoal(), true);
      }
    }

    // ------------------------------------------------------------------
    // escape: kick the ball sideways to the free side
    // ------------------------------------------------------------------
    state(escape)
    {
      transition
      {
        if (!theFieldBall.ballWasSeen(ballNotSeenTimeout))
          goto searchForBall;
        if (state_time > kickTimeout)
          goto dribbleFollowthrough;
      }
      action
      {
        const int side = calcFreeSide();
        const KickInfo::KickType kType = (side > 0)
            ? KickInfo::walkSidewardsLeftFootToLeft
            : KickInfo::walkSidewardsRightFootToRight;
        theGoToBallAndKickSkill(side > 0 ? lateralKickAngle : -lateralKickAngle, kType, false);
      }
    }

    // ------------------------------------------------------------------
    // dribbleFollowthrough: push into the freed space
    // ------------------------------------------------------------------
    state(dribbleFollowthrough)
    {
      transition
      {
        if (!theFieldBall.ballWasSeen(ballNotSeenTimeout))
          goto searchForBall;
        if (state_time > followthroughDuration || !theObstacleModel.opponentIsClose(contestRange))
        {
          contestStartTime = 0;
          goto assess;
        }
      }
      action
      {
        theGoToBallAndDribbleSkill(calcAngleToGoal(), true);
      }
    }

    // ------------------------------------------------------------------
    // searchForBall: lost sight of ball
    //   Phase 1 (state_time < turnToBallTimeout, heading error > 15°):
    //     rotate body toward lastBallPosition.
    //   Phase 2: walk toward endPositionOnField (where the ball is predicted to stop),
    //     falling back to lastBallPosition if endPositionOnField is stale.
    // ------------------------------------------------------------------
    state(searchForBall)
    {
      transition
      {
        if (theFieldBall.ballWasSeen())
        {
          contestStartTime = 0;
          goto assess;
        }
      }
      action
      {
        const Angle angleToLastBall =
            (theRobotPose.inversePose * lastBallPosition).angle();
        if (state_time < turnToBallTimeout && std::abs(angleToLastBall) > 15_deg)
        {
          // Phase 1: rotate toward the last known ball position
          theLookActiveSkill();
          theWalkAtRelativeSpeedSkill(
              Pose2f(std::clamp((float)angleToLastBall, -1.f, 1.f), 0.f, 0.f));
        }
        else
        {
          // Phase 2: walk toward the ball's predicted stop position (optimistic pursuit).
          // Use endPositionOnField if available, fall back to last seen position.
          const Vector2f& walkTarget =
              (lastBallEndPosition.squaredNorm() > 0.f) ? lastBallEndPosition : lastBallPosition;
          const Vector2f targetRel = theRobotPose.inversePose * walkTarget;
          theLookActiveSkill();
          theWalkToPointSkill(Pose2f(targetRel.angle(), targetRel), 1.f);
        }
      }
    }
  }

  // -----------------------------------------------------------------------
  // calcFreeSide: +1 = kick left is safer, -1 = kick right is safer.
  //
  // Weights each visible opponent by 1/distance and sums their y-components
  // (robot frame: positive y = opponent is to the LEFT).
  // A positive sum means the left side is more congested → kick right (-1).
  // Field boundary pressure is added to avoid sending the ball out of bounds.
  // -----------------------------------------------------------------------
  int calcFreeSide() const
  {
    float oppSum = 0.f;
    for (const auto& obs : theObstacleModel.obstacles)
    {
      if (obs.isOpponent())
      {
        const float dist = obs.center.norm();
        if (dist > 0.f)
          oppSum += obs.center.y() / dist;
      }
    }

    // Field boundary pressure: push away from nearby sidelines.
    // yPosLeftSideline > 0, yPosRightSideline < 0 in SPL coordinates.
    const float myY = theRobotPose.translation.y();
    if (myY > theFieldDimensions.yPosLeftSideline  - boundaryMargin)  oppSum += 2.f;
    if (myY < theFieldDimensions.yPosRightSideline + boundaryMargin)  oppSum -= 2.f;

    // oppSum > 0: left is busier → kick right
    return (oppSum <= 0.f) ? 1 : -1;
  }

  Angle calcAngleToGoal() const
  {
    return (theRobotPose.inversePose *
            Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
  }
};

MAKE_CARD(BallContestCard);
