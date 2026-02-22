/**
 * @file OffenseFastGoalKickCard.cpp
 * @author  Adrian Müller 
 * @version: 1.0
 *
 * Functions, values, side effects: 
 * any TACTICAL_OFFENSE that playsTheBall()
 * - bot is close to oppenent goal
 * - bot does a fast kick (walkForwardsRight,walkForwardsLeft), in direction opp. goal
 *
 * Details: 
 * Purpose of this card is to take over from ChaseBallCard; trigger is distance to opp. goal
 * - isPlayBall()
 * - quick shot using correct foot: 
 *   - left or right is computed once, when the card is called for the first time

  * v1.1. 
  * card is wifi on/off ready (Adrian)
 * 
 * Note: 
 * - because this is a short shot, the flag "playsTheBall" may not re-set after the shot, 
 * - However, if the ball is stuck, the flag may still be set, and the player will follow the ball
 * - This behavior is ok if not in SPARSE mode
 * 
 * 
 * OpenPoints status:
 * - this card now uses shared shot gating and ball-source logic
 * - full sector-wheel driven replacement with GoalShot remains future work

 */


 // B-Human includes
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/Shots.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Configuration/FieldDimensions.h"

#include "Representations/Modeling/RobotPose.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Tools/Math/BHMath.h"
#include "Tools/BehaviorControl/R2KAttackLogic.h"
#include "Tools/BehaviorControl/R2KBallSourceLogic.h"
#include "Tools/BehaviorControl/R2KDecisionLog.h"
#include <string>
#include <vector>

// this is the R2K specific stuff

#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/TeamCommStatus.h"

CARD(OffenseFastGoalKickCard,
  { ,
    CALLS(Activity),
    CALLS(GoToBallAndKick),
    REQUIRES(FieldBall),
    REQUIRES(FieldDimensions),
    REQUIRES(Shots),
    REQUIRES(RobotInfo),
    REQUIRES(RobotPose),
    REQUIRES(TeammateRoles),  // R2K
    REQUIRES(TeamCommStatus),  // wifi on off?

    DEFINES_PARAMETERS(
    {,
      (float)(2500) minGoalDist,
      (int)(500) ballSeenTimeoutMs,
      (float)(250.f) forecastBallTravelThresholdMm,
      (bool)(false) footIsSelected,  // freeze the first decision
      (bool)(true) leftFoot,
    }),
  });

class OffenseFastGoalKickCard : public OffenseFastGoalKickCardBase
{
  bool preconditions() const override
  {
    // OUTPUT_TEXT("offense index " << theTeammateRoles.offenseRoleIndex(theRobotInfo.number) << " nr " << theRobotInfo.number);
    return
      theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive) &&   // I am the striker
      theTeammateRoles.isTacticalOffense(theRobotInfo.number) && // my recent role
      theFieldBall.endPositionOnField.x() >= (theFieldDimensions.xPosOpponentGoalArea-1000) &&
      theShots.goalShot.failureProbability >= 0.50f; // GoalShotCard has priority for better opportunities
  }

  bool postconditions() const override
  {
    return !preconditions();
  }


  void execute() override
  {

    theActivitySkill(BehaviorStatus::offenseFastGoalKick);

    if (!footIsSelected) {  // select only once
      footIsSelected = true;
      leftFoot = theFieldBall.positionRelative.y() < 0;
    }
    const bool localizationPoor = theRobotPose.quality == RobotPose::poor;
    const bool ballSeenRecently = theFieldBall.ballWasSeen(ballSeenTimeoutMs);
    const bool useForecast = (theFieldBall.endPositionRelative - theFieldBall.positionRelative).norm() > forecastBallTravelThresholdMm;
    const auto ballSource = R2KBallSourceLogic::chooseBallSource(ballSeenRecently, theTeamCommStatus.isWifiCommActive, useForecast);
    const float distanceToGoal = std::abs(theFieldDimensions.xPosOpponentGroundLine - theFieldBall.endPositionOnField.x());

    const R2KAttackLogic::ShotDecision shotDecision = R2KAttackLogic::decideShotExecution(ballSeenRecently,
                                                                                           localizationPoor,
                                                                                           distanceToGoal,
                                                                                           minGoalDist);

    if(ballSource == R2KBallSourceLogic::BallSource::forecast)
      R2KDecisionLog::annotation("ball_prediction_source", {{"card", "OffenseFastGoalKick"},
                                                       {"player", std::to_string(theRobotInfo.number)},
                                                       {"source", R2KBallSourceLogic::toString(ballSource)}});

    if(shotDecision.mode == R2KAttackLogic::ShotExecutionMode::hold)
    {
      R2KDecisionLog::annotation("shot_gate", {{"card", "OffenseFastGoalKick"},
                                            {"mode", "hold"},
                                            {"reason", R2KAttackLogic::toString(shotDecision.reason)},
                                            {"player", std::to_string(theRobotInfo.number)},
                                            {"ballSource", R2KBallSourceLogic::toString(ballSource)}});
      return;
    }

    const KickInfo::KickType kickType = (shotDecision.mode == R2KAttackLogic::ShotExecutionMode::preciseKick)
                                          ? (leftFoot ? KickInfo::forwardFastLeft : KickInfo::forwardFastRight)
                                          : (leftFoot ? KickInfo::walkForwardsLeft : KickInfo::walkForwardsRight);

    if(shotDecision.reason != R2KAttackLogic::DecisionReason::ready)
      R2KDecisionLog::annotation("shot_gate", {{"card", "OffenseFastGoalKick"},
                                            {"mode", "fallback"},
                                            {"reason", R2KAttackLogic::toString(shotDecision.reason)},
                                            {"player", std::to_string(theRobotInfo.number)},
                                            {"distGoal", std::to_string(static_cast<int>(distanceToGoal))},
                                            {"ballSource", R2KBallSourceLogic::toString(ballSource)}});

    theGoToBallAndKickSkill(calcAngleToGoal(), kickType, shotDecision.mode == R2KAttackLogic::ShotExecutionMode::preciseKick);
  }

  Angle calcAngleToGoal() const
  {
    return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
  }
};

MAKE_CARD(OffenseFastGoalKickCard);
