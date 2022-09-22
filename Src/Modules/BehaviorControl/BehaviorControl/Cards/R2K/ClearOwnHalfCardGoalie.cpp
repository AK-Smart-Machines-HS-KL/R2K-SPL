/**
 * @file ClearOwnHalfCardGoalie.cpp
 * @author  Adrian Müller 
 * @version: 1.0
 *
 * Functions, values, side effects: 
 * any TACTICAL_GOALIE  with playsTheBall()
 * no other card qualifies for 
 * - ball is in own half, near goalbox
 * - there is not enough time to do a long shot (see ....LongShotCards), 
 *  -so bot does a fast kick (walkForwardsRight,walkForwardsLeft), in his walking direction 
 *
 * Details: 
 * Purpose of this card is to clear the own field by a long shot.
 * - opponent is to close (otherwise see ClearOwnHalf)
 * - isPlayBall()
 * - quick using correct foot: 
 *   - left or right is computed once, when the card is called for the first time
 *   - reason: while approaching the ball, the relative y-position might vary (for only a few centimeters),
 *             causing the skill to "freeze"
 * - bot will not leaf own half,
 * 
 * v.1.1: increased minOppDistance to 2000
 * v1.2. added setPlay==SET_PLAY_NONE to prevent goalie be activated for CORNER_KICK
 *       changed to theTeammateRoles.isTacticalGoalkeeper
         goalie must not leave the goal box + maxDistanceFromGoalArea
 * 
 * Note: 
 * - because this is a short shot, the flag "playsTheBall" may not re-set after the shot, 
 * - However, if the ball is stuck, the flag may still be set, and the player will follow the ball
 * - This behavior is ok if not in SPARSE mode
 * - minOppDistance must be maintained with the ...LongShotCards
 * 
 * 
 * ToDo:
 * - we need a better shooting direction!! 
 * - maybe add OFFENSIVE mode as a blocker?
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
#include "Representations/Communication/GameInfo.h"

CARD(ClearOwnHalfCardGoalie,
  { ,
    CALLS(Activity),
    CALLS(GoToBallAndKick),
    USES(GameInfo),
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
      (float)(1500) minOppDistance,
      (float)(500) maxDistanceFromGoalArea,  // how far  goalie will leave the goal box
      (bool)(false) footIsSelected,  // freeze the first decision
      (bool)(true) leftFoot,
    }),
  });

class ClearOwnHalfCardGoalie : public ClearOwnHalfCardGoalieBase
{
  bool preconditions() const override
  {
    return
      theGameInfo.setPlay == SET_PLAY_NONE &&
      //theTeammateRoles.playsTheBall(theRobotInfo.number) &&  // I am the striker
      // opponentIsClose() &&  // see LongShotCard, !opponentIsTooClose()
      theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number) && // my recent role
      theFieldBall.endPositionOnField.x() <= theFieldDimensions.xPosOwnGoalArea + maxDistanceFromGoalArea &&
      !(theTeamBehaviorStatus.teamActivity == TeamBehaviorStatus::R2K_SPARSE_GAME);
  }

  bool postconditions() const override
  {
    return !preconditions();
  }

 
  void execute() override
  {

    theActivitySkill(BehaviorStatus::clearOwnHalfCardGoalie);

    if (!footIsSelected) {  // select only once
      footIsSelected = true;
      leftFoot = theFieldBall.positionRelative.y() < 0;
    }
    if(leftFoot)
      theGoToBallAndKickSkill(0, KickInfo::walkForwardsLeft);
    else
      theGoToBallAndKickSkill(0, KickInfo::walkForwardsRight);
  }


  // need to clarify: opponent detection
  bool opponentIsClose() const{
    for (const Obstacle& ob : theObstacleModel.obstacles)
    {if (ob.isOpponent()) // || ob.isTeammate())
      return ob.center.norm() <= minOppDistance;
    }
    return false;
  }
};

MAKE_CARD(ClearOwnHalfCardGoalie);
