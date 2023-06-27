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
 * ToDo:
 * - replace this card with GoalShot with more sophisticated features, incl. applying sector wheel 

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
#include "Representations/Communication/TeamCommStatus.h"

CARD(OffenseFastGoalKick,
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
    REQUIRES(TeamCommStatus),  // wifi on off?

    DEFINES_PARAMETERS(
    {,
      (float)(2500) minGoalDist,
      (bool)(false) footIsSelected,  // freeze the first decision
      (bool)(true) leftFoot,
    }),
  });

class OffenseFastGoalKick : public OffenseFastGoalKickBase
{
  bool preconditions() const override
  {
    // OUTPUT_TEXT("offense index " << theTeammateRoles.offenseRoleIndex(theRobotInfo.number) << " nr " << theRobotInfo.number);
    return
      theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive) &&   // I am the striker
      theTeammateRoles.isTacticalOffense(theRobotInfo.number) && // my recent role
      theFieldBall.endPositionOnField.x() >= theFieldDimensions.xPosOpponentGoalArea;
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
    if (leftFoot)
      theGoToBallAndKickSkill(calcAngleToGoal(), KickInfo::forwardFastLeft, false);
    else
      theGoToBallAndKickSkill(calcAngleToGoal(), KickInfo::forwardFastRight, false);
  }

  Angle calcAngleToGoal() const
  {
    return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
  }
};

MAKE_CARD(OffenseFastGoalKick);

