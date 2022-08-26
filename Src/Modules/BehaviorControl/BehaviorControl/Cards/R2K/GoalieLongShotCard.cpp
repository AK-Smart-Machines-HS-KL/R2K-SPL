/**
 * @file GoalieLongShotCard.cpp
 * @author Nicholas Pfohlmann, Adrian Müller
 * @version: 1.0
 *
 * Functions, values, side effects:
 * - this card qualifies for the GOALIE player if the balls x-Pos is below the 
 *   circumcenter x-Pos of triangle formed by default pos of GL, DL, DR
 * - it will take its time to make a long kick if no opppenent is too close
 *
 * Details:
 * Purpose of this card is to clear the own field by a long shot.
 * - opponent is not to close (otherwise see ClearOwnHalf)
 * - isPlayBall()
 * - long shot to opponent goal, using correct foot:
 *   - left or right is computed once, when the card is called for the first time
 *   - reason: while approaching the ball, the relative y-position might vary (for only a few centimeters),
 *             causing the skill to "freeze"
 * - goalie will not go beyond circumcenter x-Pos
 *
 * After kick:
 * - a) card exists if role no longer is playsTheBall()
 * - b) exit after kick (isDone()) to avoid goalie player chase the ball
 *
 * Note:
 * - because this is a long shot, the flag "playsTheBall" typicall is re-set after the shot, as a side effect.
 * - However, if the ball is stuck, the flag may still be set, and the player will follow the ball
 * - If goalie walks closer to ball it can occur that he is still playsTheBall but ball is beyond 
 *   the centroid x-Pos if ball was moved
 *
 * ToDo:
 * check for free shoot vector and opt. change y-coordinate
 * check whether isDone () works correctly
 * check for GoToBallAndKick options (eg "precisey") for better pormance
 * 
 * read default Positions from DefualtPoseProvider
 * provide max opp distance as LOAD_PARAMETERS
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
      (float)(-2000) circumcenterPosX,
      (float)(1000) minOppDistance,
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
      !opponentIsTooClose() &&  // see below: distace is minOppDistance
      theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number) &&// my recent role

      //don't go beyond the circumcenter of triangle formed by the default poses of GL, DL, DR
      (
        theFieldBall.positionOnField.x() <= circumcenterPosX //Circumcenter X-Coord 
      );
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
    if (leftFoot)
      theGoToBallAndKickSkill(calcAngleToGoal(), KickInfo::forwardFastLeftLong);
    else
      theGoToBallAndKickSkill(calcAngleToGoal(), KickInfo::forwardFastRightLong);
  }

  Angle calcAngleToGoal() const
  {
    return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
  }

  bool opponentIsTooClose() const {
    for (const Obstacle& ob : theObstacleModel.obstacles)
    {
      if (ob.isOpponent()) //tbd: check for teammate, add sector wheel)
        return ob.center.norm() < minOppDistance;
    }
    return false;
  }
};

MAKE_CARD(GoalieLongShotCard);