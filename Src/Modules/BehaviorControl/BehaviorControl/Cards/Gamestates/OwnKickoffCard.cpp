/**
 * @file OwnKickoffCard.cpp
 * @author Andy Hobelsberger
 * @brief Covers Own Kickoff — ball carrier at center circle
 * @version 3.0
 * @date 2022-11-22
 *
 * State machine:
 *
 *  [sweep] ──buddy found──► [pass]   walk-kick along buddy's heading
 *          └─no buddy──────► [dribble] ──ball 1 m forward──► [shoot]
 *
 *  sweep : fast camera left-right (200°/s, 1 s) to refresh teammate positions
 *  pass  : soft walk-kick toward buddy's runway (target frozen at sweep end)
 *  dribble: dribble straight toward opponent goal until ball crosses x = +1000 mm
 *  shoot : precise forwardFast shot at opponent goal center
 *
 * Buddy qualification (evaluated at sweep end):
 *   - not penalised, upright
 *   - |y| > 500 mm  (clearly left or right — attractive pass lane)
 *   - x  > kicker.x − 1000 mm  (not more than 1 m behind the kicker)
 *   - buddy attracted toward ball  OR  already entering opponent half
 *
 * V1.1 Card migrated (Nicholas)
 * V1.2 Changed to long kick (Adrian)
 * V1.3 Card disabled
 * V2.0 Re-enabled with buddy-runway pass logic (R2K)
 * V2.1 Camera sweep before target selection (R2K)
 * V2.2 Precise goal shot as no-buddy fallback (R2K)
 * V3.0 Dribble 1 m + precise shot replaces direct goal shot fallback;
 *       fast sweep (1 s / 200°/s); pass target computed once (R2K)
 */

#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Infrastructure/ExtendedGameInfo.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/TeamData.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/MotionControl/MotionRequest.h"


CARD(OwnKickoffCard,
{,
  CALLS(Activity),
  CALLS(GoToBallAndKick),
  CALLS(LookLeftAndRight),
  CALLS(LookAtBall),
  CALLS(Stand),
  CALLS(Dribble),

  REQUIRES(FieldBall),
  REQUIRES(FieldDimensions),
  REQUIRES(RobotPose),
  REQUIRES(RobotInfo),
  REQUIRES(OwnTeamInfo),
  REQUIRES(GameInfo),
  REQUIRES(ExtendedGameInfo),
  REQUIRES(FrameInfo),
  REQUIRES(TeammateRoles),
  REQUIRES(TeamData),

  DEFINES_PARAMETERS(
  {,
    (Vector2f)(Vector2f::Zero()) passTarget,                        // absolute field target, frozen at sweep end
    (bool)(true)                 kickLeft,                          // foot: true = left
    (KickInfo::KickType)(KickInfo::walkForwardsLeft) kickType,      // frozen at sweep end (pass) or dribble end (shot)
  }),
});

class OwnKickoffCard : public OwnKickoffCardBase
{
  /**
   * @brief Fires for the robot playing the ball during our own kickoff
   *        within the first 10 seconds of PLAYING.
   */
  bool preconditions() const override
  {
    return theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
      && theExtendedGameInfo.timeSincePlayingStarted < 10000
      && theGameInfo.state == STATE_PLAYING
      && theTeammateRoles.playsTheBall(theRobotInfo.number);
  }

  bool postconditions() const override
  {
    return !preconditions();
  }

  option
  {
    theActivitySkill(BehaviorStatus::ownKickoff);

    // ── State 1: fast camera sweep to refresh teammate positions ─────────────
    // LookLeftAndRight defaults: startLeft=true, maxPan=50°, tilt=23°.
    // At speed=200°/s a full left-right-left takes ~1 s.
    initial_state(sweep)
    {
      transition
      {
        if (state_time > 1000)
        {
          // ── pick the best qualifying buddy ──────────────────────────────
          const Teammate* bestBuddy = nullptr;
          float bestAbsY = 0.f;

          for (const auto& buddy : theTeamData.teammates)
          {
            if (!buddy.isPenalized                                                    // WiFi-derived fast path
              && theOwnTeamInfo.players[buddy.number - 1].penalty == PENALTY_NONE   // GC-authoritative
              && buddy.isUpright
              && theFrameInfo.time - buddy.timeWhenLastPacketReceived < 3000)        // data < 3 s old
            {
              const float absY   = std::abs(buddy.theRobotPose.translation.y());
              const float buddyX = buddy.theRobotPose.translation.x();

              // clearly left/right, not more than 1 m behind the kicker
              if (absY  > 500.f
                && buddyX > theRobotPose.translation.x() - 1000.f
                && absY  > bestAbsY)
              {
                bestBuddy = &buddy;
                bestAbsY  = absY;
              }
            }
          }

          if (bestBuddy != nullptr)
          {
            // ── buddy found → pass along their current heading (runway) ──
            const Angle heading = bestBuddy->theRobotPose.rotation;
            passTarget = bestBuddy->theRobotPose.translation
                       + Vector2f(std::cos(heading), std::sin(heading)) * 1500.f;
            kickLeft  = bestBuddy->theRobotPose.translation.y() > 0.f;
            kickType  = kickLeft ? KickInfo::walkForwardsLeft : KickInfo::walkForwardsRight;
            goto pass;
          }
          else
          {
            // ── no buddy → dribble forward first ────────────────────────
            goto dribble;
          }
        }
      }

      action
      {
        theLookLeftAndRightSkill(true, 50_deg, 23_deg, 200_deg);
        theStandSkill();
      }
    }

    // ── State 2: buddy pass ───────────────────────────────────────────────────
    // Target and kick type were frozen in the sweep transition; kick once.
    state(pass)
    {
      transition {}

      action
      {
        theGoToBallAndKickSkill(theRobotPose.toRelative(passTarget).angle(), kickType);
      }
    }

    // ── State 3: dribble toward opponent goal ─────────────────────────────────
    // Drive the ball ~1 m into the opponent half so the subsequent shot is
    // no longer a direct kick-off goal shot (SPL rule compliance).
    state(dribble)
    {
      transition
      {
        // ball has reached 1 m inside opponent half, or 3-second safety timeout
        if (theFieldBall.positionOnField.x() > 1000.f || state_time > 3000)
        {
          // decide foot from live ball position at the moment of transition
          kickLeft   = theFieldBall.positionRelative.y() < 0.f;
          kickType   = kickLeft ? KickInfo::forwardFastLeft : KickInfo::forwardFastRight;
          passTarget = Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f);
          goto shoot;
        }
      }

      action
      {
        // dribble straight toward the opponent goal (robot faces goal at kickoff)
        const Angle goalDir = theRobotPose.toRelative(
          Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
        theDribbleSkill(goalDir, Pose2f(1.f, 1.f, 1.f), MotionRequest::ObstacleAvoidance{});
        theLookAtBallSkill();
      }
    }

    // ── State 4: precise goal shot after dribble ──────────────────────────────
    state(shoot)
    {
      transition {}

      action
      {
        theGoToBallAndKickSkill(theRobotPose.toRelative(passTarget).angle(), kickType);
      }
    }
  }
};

MAKE_CARD(OwnKickoffCard);
