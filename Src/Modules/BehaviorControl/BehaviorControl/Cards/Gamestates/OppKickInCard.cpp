/**
 * @file OppKickInCard.cpp
 * @author Andy Hobelsberger
 * @brief Covers the Free Kick: Opponent Side Kick In
 * @version 1.1
 * @date 2022-11-22
 *  
 * Behavior:
 * Positions the closest robot to the ball between the ball and the goal
 * 
 * Parameters:
 *  blockingDistance : The distance at which to stand to the ball. Make sure this follows the rules and your defender keeps it's distance 
 *
 * V1.1 Card migrated (Nicholas)
 */

#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

#include "Representations/BehaviorControl/Libraries/LibWalk.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"

#include "Representations/Configuration/FieldDimensions.h"

#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/TeamData.h"
#include "Representations/BehaviorControl/TeammateRoles.h"

#include "Representations/Modeling/RobotPose.h"

#include "Tools/Math/Geometry.h"

// default actions for GORE2022
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/DefaultPose.h"
#include "Representations/Configuration/GlobalOptions.h"

CARD(OppKickInCard,
{,
  CALLS(Stand),
  CALLS(Activity),
  CALLS(LookForward),
  CALLS(GoToBallAndDribble),
  CALLS(WalkToPoint),

  REQUIRES(DefaultPose),
  REQUIRES(FieldBall),
  REQUIRES(FieldDimensions),
  REQUIRES(GameInfo),
  REQUIRES(GlobalOptions),
  REQUIRES(LibWalk),
  REQUIRES(OwnTeamInfo),
  REQUIRES(RobotPose),
  REQUIRES(TeamData),
  REQUIRES(RobotInfo),
  REQUIRES(TeammateRoles),


  DEFINES_PARAMETERS(
    {,
      (float) (1000.f) blockingDistance,
    }
  ),
});

class OppKickInCard : public OppKickInCardBase
{

  Vector2f blockingPos;
  bool isBlocking;

  /**
   * @brief The condition that needs to be met to execute this card
   */
  bool preconditions() const override
  {
    return theGameInfo.kickingTeam != theOwnTeamInfo.teamNumber
      && theGameInfo.setPlay == SET_PLAY_KICK_IN
      && !theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number);
  }

  /**
   * @brief The condition that needs to be met to exit the this card
   */
  bool postconditions() const override
  {
    return !preconditions();
  }

  option
  {
    theActivitySkill(BehaviorStatus::oppFreeKick);

    initial_state(init)
    {
      isBlocking = true;
      float ownDistToBall = Geometry::distance(theRobotPose.translation, theFieldBall.positionOnField);

      for(auto& teammate : theTeamData.teammates)
      {
        if (ownDistToBall > Geometry::distance(teammate.theRobotPose.translation, theFieldBall.positionOnField)) {
          isBlocking = false;
          break;
        }
      }

      transition
      {
        if (isBlocking) {
          goto block;
        } else {
          goto standby;
        }
      }

      action
      {
        theLookForwardSkill();
        theStandSkill();
      }
    }

    state(block)
    {
      transition
      {

      }

      action
      {
        theLookForwardSkill();

        Vector2f ownGoalPos = Vector2f(theFieldDimensions.xPosOwnGoal, theFieldDimensions.yPosCenterGoal);
        Vector2f ballToGoal = (ownGoalPos - theFieldBall.positionOnField).normalized();
        //Translation for walking
        blockingPos = theRobotPose.toRelative(theFieldBall.positionOnField + ballToGoal * blockingDistance);
        //Walk closer to blockingPos and face ball
        theWalkToPointSkill(Pose2f(theFieldBall.positionRelative.angle(), blockingPos));
      }
    }

    state(standby)
    {
      transition
      {

      }

      action
      { 
        theLookForwardSkill();
        Vector2f targetRelative = theRobotPose.toRelative(theDefaultPose.ownDefaultPose.translation);
        theWalkToPointSkill(targetRelative);
      }
    }
  }
};

MAKE_CARD(OppKickInCard);