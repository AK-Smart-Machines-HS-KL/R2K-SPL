/**
 * @file DefenseCoverBackCard.cpp
 * @author Niklas Schmidts, Adrian Müller
 * @brief Defender covers the angle to own goal by positioning between ball and goal.
 * @version 1.4
 * @date 2023-01-06
 *
 * Behaviour:
 *   Activates for any robot with a DEFENSE tactical role when no teammate is already
 *   chasing/clearing the ball.  The robot positions itself 600 mm ahead of the ball
 *   on the ball-to-own-goal axis, facing the ball.  This blocks the direct shot angle.
 *
 * v1.1  Avoid two defenders fighting over the ball: exit if any buddy is already
 *       DefenseChaseBallCard / ClearOwnHalfCard / blocking.
 * v1.2  Quit if a passing event (OffenseForwardPassCard / OffenseReceivePassCard) is active.
 * v1.3  (Asrar) Use TeammateRoles::playsTheBall() for both wifi-on and wifi-off.
 *       Restrict activation to own half (x < 0 - threshold).
 * v1.4  Replaced WalkAtRelativeSpeed near-range branch with WalkToPoint for all distances.
 *       Side-stepping was physically much slower than a pivot-and-walk on the NAO; the
 *       previous close-range path caused "lame side-step" interception behaviour.
 *       Removed unused CALLS (GoToBallAndDribble, LookForward, WalkAtRelativeSpeed),
 *       unused walkSpeed parameter, and dead helper methods.
 */

// Skills - Must be included BEFORE Card Base
#include "Representations/BehaviorControl/Skills.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

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


CARD(DefenseCoverBackCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookAtBall),
        CALLS(WalkToPoint),
        CALLS(Stand),
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

        DEFINES_PARAMETERS(
             {,
                (int)(5000) ballNotSeenTimeout,
                // Offset [mm] from the ball toward own goal where the defender aims to stand.
                // Large enough to not interfere with a chasing teammate, small enough to be
                // a meaningful obstacle for an opponent shot.
                (float)(600.f) blockingOffset,
                // Distance tolerance [mm] below which the robot considers itself arrived.
                // Matches PositionTolerance in defaultPoseProvider.cfg (250 mm).
                (float)(250.f) arrivalTolerance,
             }),

     });

class DefenseCoverBackCard : public DefenseCoverBackCardBase
{

  bool preconditions() const override
  {
    Vector2f ownGoal = Vector2f(theFieldDimensions.xPosOwnGroundLine, 0);
    float distToGoal = (ownGoal - theFieldBall.positionOnField).norm();

    return
      theFieldBall.ballWasSeen()                                        &&
      distToGoal > 1000                                                 &&
      theGameInfo.setPlay == SET_PLAY_NONE                              &&
      !aBuddyIsChasingOrClearing()                                      &&
      theTeammateRoles.isTacticalDefense(theRobotInfo.number);
  }

  bool postconditions() const override
  {
    return !preconditions();
  }

  void execute() override
  {
    // Target: blockingOffset mm from ball toward own goal, facing the ball
    Vector2f ownGoalRel   = theRobotPose.toRelative(Vector2f(theFieldDimensions.xPosOwnGroundLine, 0));
    Vector2f ballToGoal   = ownGoalRel - theFieldBall.positionRelative;
    Vector2f blockDir     = ballToGoal.normalized();
    Pose2f   target       = Pose2f(theFieldBall.positionRelative.angle(),
                                   theFieldBall.positionRelative + blockDir * blockingOffset);

    theActivitySkill(BehaviorStatus::blocking);
    theLookAtBallSkill();

    if (target.translation.norm() > arrivalTolerance || std::abs(target.rotation) > 10_deg)
    {
      // WalkToPoint pivots then walks forward regardless of direction — much faster than
      // the previous WalkAtRelativeSpeed path which produced pure lateral side-stepping.
      theWalkToPointSkill(target);
    }
    else
    {
      theStandSkill();
    }
  }

  bool aBuddyIsChasingOrClearing() const
  {
    for (const auto& buddy : theTeamData.teammates)
    {
      if (buddy.theBehaviorStatus.activity == BehaviorStatus::defenseChaseBallCard  ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::ballContestCard        ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::blocking               ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::clearOwnHalfCard       ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::clearOwnHalfCardGoalie ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::defenseLongShotCard    ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::goalieLongShotCard)
        return true;
    }
    return false;
  }
};

MAKE_CARD(DefenseCoverBackCard);
