/**
 * @file OppPushingFreeKickCard.cpp
 * @author Andy Hobelsberger
 * @brief Covers the Free Kick: Opponent has Pushing Free Kick
 * @version 1.1
 * @date 2022-11-22
 * 
 * Notes: 
 *  - Currently Triggers for all Robots. Use this Card as a template for preconditions
 *  - Currently only calls stand   
 * 
 * V1.1 Card migrated (Nicholas)
 */

#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

#include "Representations/BehaviorControl/Skills.h"

#include "Representations/Configuration/FieldDimensions.h"

#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"

#include "Representations/Modeling/RobotPose.h"

// default actions for GORE2022
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/DefaultPose.h"



CARD(OppPushingFreeKickCard,
{,
  CALLS(Activity),
  CALLS(LookForward),
  CALLS(WalkToPoint),

  REQUIRES(DefaultPose),
  REQUIRES(FieldBall),
  REQUIRES(FieldDimensions),
  REQUIRES(GameInfo),
  REQUIRES(RobotPose),
  REQUIRES(OwnTeamInfo),
});

class OppPushingFreeKickCard : public OppPushingFreeKickCardBase
{
  /**
   * @brief The condition that needs to be met to execute this card
   */
  bool preconditions() const override
  {
    return theGameInfo.kickingTeam != theOwnTeamInfo.teamNumber
        && theGameInfo.setPlay == SET_PLAY_PUSHING_FREE_KICK;
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

        //Translation for walk
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
  
  Angle calcAngleToGoal() const
  {
    return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
  }
};

MAKE_CARD(OppPushingFreeKickCard);