/**
 * @file OwnKickoffRunnerCard.cpp
 * @author R2K
 * @brief Positions tactical offenders as good pass targets during own kickoff.
 * @version 1.0
 * @date 2024-01-01
 *
 * Behavior: Tactical offense robots that are NOT the ball carrier advance
 * toward the opponent half to become a good pass target for the kicker.
 * SPL constraint: they must not cross the center line before the ball is
 * touched. Ball-in-play is detected by the ball moving > 300 mm from center.
 *
 * Phase 1 (ball not yet played): walk up to just inside own half (x = -100 mm),
 *   maintaining current lateral (y) position.
 * Phase 2 (ball played):         advance to 1 m inside opponent half (x = +1000 mm),
 *   maintaining current lateral (y) position.
 * Head always tracks actively so the kicker can be seen.
 */

#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Infrastructure/ExtendedGameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/BehaviorControl/TeammateRoles.h"


CARD(OwnKickoffRunnerCard,
{,
  CALLS(Activity),
  CALLS(LookActive),
  CALLS(WalkToPoint),

  REQUIRES(FieldBall),
  REQUIRES(RobotPose),
  REQUIRES(RobotInfo),
  REQUIRES(OwnTeamInfo),
  REQUIRES(GameInfo),
  REQUIRES(ExtendedGameInfo),
  REQUIRES(TeammateRoles),
});

class OwnKickoffRunnerCard : public OwnKickoffRunnerCardBase
{
  /**
   * @brief Fires for non-ball-carrying tactical offense robots during own kickoff.
   */
  bool preconditions() const override
  {
    return theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
      && theExtendedGameInfo.timeSincePlayingStarted < 10000   // 10 sec window
      && theGameInfo.state == STATE_PLAYING
      && !theTeammateRoles.playsTheBall(theRobotInfo.number)    // not the kicker
      && theTeammateRoles.isTacticalOffense(theRobotInfo.number);
  }

  bool postconditions() const override
  {
    return !preconditions();
  }

  void execute() override
  {
    theActivitySkill(BehaviorStatus::ownKickoff);
    theLookActiveSkill();

    // Ball is considered "in play" once it has moved 300 mm from the center spot.
    // Until then, SPL rules forbid crossing the center line.
    const bool ballPlayed = theFieldBall.positionOnField.norm() > 300.f;
    const float targetX   = ballPlayed ? 1000.f : -100.f;

    // Keep current lateral position — robot is already a good pass-lane target.
    const Pose2f targetAbsolute(0.f, targetX, theRobotPose.translation.y());
    const Pose2f targetRelative = theRobotPose.toRelative(targetAbsolute);

    // Face the ball while walking so the kicker can see us as a target.
    theWalkToPointSkill(
      Pose2f(theFieldBall.positionRelative.angle(), targetRelative.translation),
      1.f,    // full speed
      true);  // rough — approximate arrival is fine
  }
};

MAKE_CARD(OwnKickoffRunnerCard);
