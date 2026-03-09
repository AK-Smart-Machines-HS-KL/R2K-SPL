/**
 * @file GoToBallAndDribble.cpp
 *
 * This file implements an implementation of the GoToBallAndDribble skill.
 *
 * @author Arne Hasselbring
 */

#include "Representations/BehaviorControl/BehaviorStatus.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/Libraries/LibWalk.h"
#include "Representations/BehaviorControl/PathPlanner.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"
#include "Representations/Modeling/ObstacleModel.h"
#include "Representations/Modeling/RobotPose.h"
#include "Tools/BehaviorControl/Framework/Skill/Skill.h"
#include "Tools/BehaviorControl/Framework/Skill/CabslSkill.h"

SKILL_IMPLEMENTATION(GoToBallAndDribbleImpl,
{,
  IMPLEMENTS(GoToBallAndDribble),
  REQUIRES(FieldBall),
  REQUIRES(LibWalk),
  REQUIRES(ObstacleModel),
  REQUIRES(PathPlanner),
  REQUIRES(RobotPose),
  REQUIRES(TeamBehaviorStatus),
  MODIFIES(BehaviorStatus),
  CALLS(GoToBallHeadControl),
  CALLS(Dribble),
  CALLS(RecordTargetAndSpeed),
  DEFINES_PARAMETERS(
  {,
    (float)(1000.f) switchToPathPlannerDistance, /**< If the target is further away than this distance, the path planner is used. */
    (float)(900.f) switchToLibWalkDistance, /**< If the target is closer than this distance, LibWalk is used. */
    (float)(0.25f) minKickPower, /**< Minimale Kick-Power für enges Dribbling. */
    (float)(0.35f) maxKickPower, /**< Maximale Kick-Power für enges Dribbling. */
    (float)(1200.f) opponentCheckDistance, /**< Prüfe Gegner in dieser Distanz. */
    (Angle)(40_deg) opponentCheckAngle, /**< Prüfe Gegner in diesem Winkel-Kegel. */
    (Angle)(35_deg) avoidanceAngle, /**< Weiche mit diesem Winkel aus. */
  }),
});

class GoToBallAndDribbleImpl : public GoToBallAndDribbleImplBase
{
  option(GoToBallAndDribble)
  {
    // Prüfe ob Gegner direkt im Dribbling-Pfad ist
    Angle dribbleDirection = p.targetDirection;
    bool opponentInPath = false;
    float leftSpace = 1000.f; // Mehr = freier
    float rightSpace = 1000.f;
    
    for(const auto& obstacle : theObstacleModel.obstacles)
    {
      if(obstacle.type == Obstacle::opponent || obstacle.type == Obstacle::unknown)
      {
        const float dist = obstacle.center.norm();
        const Angle obsAngle = obstacle.center.angle();
        const Angle diff = obsAngle - p.targetDirection;
        
        // Gegner direkt im Pfad?
        if(std::abs(diff) < opponentCheckAngle && dist < opponentCheckDistance)
        {
          opponentInPath = true;
        }
        
        // Berechne Platz links/rechts
        if(dist < opponentCheckDistance * 1.5f)
        {
          if(obsAngle > 0_deg) // Links
            leftSpace = std::min(leftSpace, dist);
          else // Rechts
            rightSpace = std::min(rightSpace, dist);
        }
      }
    }
    
    // Wenn Gegner im Weg: Weiche zur freieren Seite aus
    if(opponentInPath)
    {
      if(leftSpace > rightSpace)
        dribbleDirection = p.targetDirection + avoidanceAngle;
      else
        dribbleDirection = p.targetDirection - avoidanceAngle;
    }

    Pose2f dribblePose(dribbleDirection, theFieldBall.endPositionRelative);

    theRecordTargetAndSpeedSkill(dribblePose.translation, 1.f);
    theGoToBallHeadControlSkill(dribblePose.translation.norm());

    // Standard Kick-Power mit Ball-Distanz Anpassung
    const float ballDistance = theFieldBall.positionRelative.norm();
    const float t = (ballDistance - 150.f) / (300.f - 150.f);
    const float adaptiveKickPower = minKickPower + t * (maxKickPower - minKickPower);
    const float finalKickPower = std::max(minKickPower, std::min(maxKickPower, p.kickPower > 0.f ? p.kickPower : adaptiveKickPower));

    initial_state(dribbleFarRange)
    {
      transition
      {
        if(dribblePose.translation.squaredNorm() < sqr(switchToLibWalkDistance))
          goto dribbleCloseRange;
      }

      action
      {
        // PathPlanner macht automatisch Obstacle Avoidance
        auto obstacleAvoidance = thePathPlanner.plan(theRobotPose * dribblePose, Pose2f(1.f, 1.f, 1.f));
        theDribbleSkill(dribbleDirection, Pose2f(1.f, 1.f, 1.f), obstacleAvoidance, false, finalKickPower, p.preStepAllowed, p.turnKickAllowed, p.directionPrecision);
      }
    }

    state(dribbleCloseRange)
    {
      transition
      {
        if(dribblePose.translation.squaredNorm() > sqr(switchToPathPlannerDistance))
          goto dribbleFarRange;
      }

      action
      {
        // LibWalk's calcObstacleAvoidance macht automatisch Gegner-Ausweichung
        auto obstacleAvoidance = theLibWalk.calcObstacleAvoidance(dribblePose, /* rough: */ true, /* disableObstacleAvoidance: */ false);
        theDribbleSkill(dribbleDirection, Pose2f(1.f, 1.f, 1.f), obstacleAvoidance, false, finalKickPower, p.preStepAllowed, p.turnKickAllowed, p.directionPrecision);
      }
    }
  }
};

MAKE_SKILL_IMPLEMENTATION(GoToBallAndDribbleImpl);