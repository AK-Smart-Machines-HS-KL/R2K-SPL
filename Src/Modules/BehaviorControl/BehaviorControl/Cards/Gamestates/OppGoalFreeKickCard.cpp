/**
 * @file OppGoalFreeKickCard.cpp
 * @author Andy Hobelsberger
 * @brief Covers the Free Kick: Opponent has goal free (11m) Kick
 * @version 1.1
 * @date 2022-11-22
 * 
 * Notes: 
 *  - Currently Triggers for all Robots. Use this Card as a template for preconditions
 *  - Currently only calls stand   
 * 
 * V1.1 Card migrated (Nicholas)
 */

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

#include "Representations/Modeling/RobotPose.h"

// default actions for GORE2022
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/DefaultPose.h"

CARD(OppGoalFreeKickCard,
{,
  CALLS(Activity),
  CALLS(LookForward),
  CALLS(WalkToPoint),

	REQUIRES(DefaultPose),
	REQUIRES(FieldBall), 
  REQUIRES(GameInfo),
  REQUIRES(OwnTeamInfo),
  REQUIRES(RobotPose),
});

class OppGoalFreeKickCard : public OppGoalFreeKickCardBase
{
  /**
   * @brief The condition that needs to be met to execute this card
   */
  bool preconditions() const override
  {
    return theGameInfo.kickingTeam != theOwnTeamInfo.teamNumber
        && theGameInfo.setPlay == SET_PLAY_GOAL_KICK;
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

      transition
      {
        
      }

      action
      {
        theLookForwardSkill();

        //Translation for walking
        Vector2f blockingPos = theRobotPose.toRelative(theDefaultPose.ownDefaultPose.translation);
        //Walk closer to blockingPos and face ball
        theWalkToPointSkill(Pose2f(theFieldBall.positionRelative.angle(), blockingPos));
      }
    }

    state(active)
    {
      transition
      {

      }

      action
      {

      }
    }

    state(standby)
    {
      transition
      {

      }

      action
      {
        
      }
    }
  }
};

MAKE_CARD(OppGoalFreeKickCard);