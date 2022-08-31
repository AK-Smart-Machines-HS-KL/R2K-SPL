/**
 * @file GoalieLongShotCard.cpp
 * @author Nicholas Pfohlmann, Adrian Müller
 * @version: 1.2
 *
 * Functions, values, side effects:
 * - this card qualifies for the GOALIE player if the ball is below the X-Pos of penalty mark
 * - it will take its time to make a long kick if no opppenent is too close
 * - it will make a faster, more unprecise version of the kick if opponent is closer
 * 
 * Details:
 * Purpose of this card is to clear the own field by a long shot.
 * - opponent is not to close (otherwise see ClearOwnHalf) and my distance to ball < opponents distance to ball
 * - isPlayBall()
 * - long shot to opponent goal, using correct foot:
 *   - left or right is computed once, when the card is called for the first time
 *   - reason: while approaching the ball, the relative y-position might vary (for only a few centimeters),
 *             causing the skill to "freeze"
 * - goalie will not go beyond penalty mark
 *
 * 
 * v1.1: changed limit for goalie to xPosOwnPenaltyMark
 * v1.2: added kick variations (precise and unprecise) and now using opponent-ball as distance
 *       and check if I am closer to ball than opponent
 * 
 * After kick:
 * - a) card exists if role no longer is playsTheBall()
 * - b) exit after kick (isDone()) to avoid goalie player chase the ball
 *
 * Note:
 * - because this is a long shot, the flag "playsTheBall" typicall is re-set after the shot, as a side effect.
 * - However, if the ball is stuck, the flag may still be set, and the player will follow the ball
 *
 * ToDo:
 * check for free shoot vector and opt. change y-coordinate
 * check whether isDone () works correctly
 * 
 * provide max opp distance as LOAD_PARAMETERS
 *
 */

 // B-Human includes
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Modeling/ObstacleModel.h"

#include "Representations/Modeling/RobotPose.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Tools/Math/BHMath.h"

// this is the R2K specific stuff

#include "Representations/BehaviorControl/TeamBehaviorStatus.h" 
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/Communication/RobotInfo.h"

CARD(GoalieLongShotCard,
{ ,
  CALLS(Activity),
  CALLS(GoToBallAndKick),
  REQUIRES(FieldBall),
  REQUIRES(FieldDimensions),
  REQUIRES(PlayerRole),  // R2K
  REQUIRES(ObstacleModel),
  REQUIRES(RobotInfo),
  REQUIRES(RobotPose),
  REQUIRES(TeamBehaviorStatus),
  REQUIRES(TeammateRoles),  // R2K

  DEFINES_PARAMETERS(
    {,
      (float)(1000) minOppDistance,
      (float)(2000) preciseKickDistance,
      (bool)(false) footIsSelected,  // freeze the first decision
      (bool)(true) leftFoot,
    }),
});

class GoalieLongShotCard : public GoalieLongShotCardBase
{
  bool preconditions() const override
  {
    return
      thePlayerRole.playsTheBall() &&  // I am the striker
      opponentIsTooClose() != 0 &&  // see below: distace is minOppDistance
      theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number) &&// my recent role

      theFieldBall.positionOnField.x() <= theFieldDimensions.xPosOwnPenaltyMark &&   //don't go to far way out
      theFieldBall.positionOnField.x() >= theFieldDimensions.xPosOwnGroundLine;  // don't leave the field
  }

  bool postconditions() const override
  {
    return !preconditions();
  }


  void execute() override
  {
    theActivitySkill(BehaviorStatus::goalieLongShotCard);

    if (!footIsSelected) {  // select only once
      footIsSelected = true;
      leftFoot = theFieldBall.positionRelative.y() < 0;
    }
    KickInfo::KickType longShotKickType = leftFoot ? KickInfo::forwardFastLeftLong : KickInfo::forwardFastRightLong;

    switch (opponentIsTooClose())
    {
      case(1): theGoToBallAndKickSkill(calcAngleToGoal(), longShotKickType, false); break;
      case(2): theGoToBallAndKickSkill(calcAngleToGoal(), longShotKickType, true); break;
      default: theGoToBallAndKickSkill(calcAngleToGoal(), longShotKickType); break;
    }
  }

  Angle calcAngleToGoal() const
  {
    return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
  }

  //returns an int value depending on how far the ball is away from the opponent
  //0: opponent is too close or is closer at ball than me
  //1: close  -> fast, unprecise kick
  //2: far    -> precise kick
  int opponentIsTooClose() const {
    int lowestProximityIndex = 2;
    float distanceToBall = theFieldBall.positionRelative.norm();

    for (const Obstacle& ob : theObstacleModel.obstacles)
    {
      if (ob.isOpponent()) { //tbd: check for teammate, add sector wheel)
        float distanceOpponentToBall = (ob.center - theFieldBall.positionRelative).norm();

        //is any opponent closer to ball than me or is too close to ball
        if (distanceToBall >= distanceOpponentToBall || distanceOpponentToBall <= minOppDistance) {
          return 0;
        }

        if (distanceOpponentToBall <= preciseKickDistance) { //close enemy found
          lowestProximityIndex = 1;
        }
      }
    }
    return lowestProximityIndex;
  }
};

MAKE_CARD(GoalieLongShotCard);