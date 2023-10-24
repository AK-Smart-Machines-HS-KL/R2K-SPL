/**
 * @file OppCornerKickCard.cpp
 * @author Andy Hobelsberger
 * @brief Covers the Free Kick: Opponent has Corner Kick
 * @version 1.2 Adrian
 * @date 2022-11-22
 * 
 * Notes v0.1: 
 *  - Currently only calls stand    
 *  - Currently Triggers for all Robots. Use this Card as a template for preconditions
 * Notes 1.0
 * - PathToTarget default position is implemented. Will be called in R2KDefensive mode.
 * 
 * V1.2 Card migrated (Nicholas), uses WalkToPose instead of PathToTarget skill -> WalkToPose has obstacle avoidance
 */

#include "Representations/BehaviorControl/Libraries/LibWalk.h"
#include "Representations/BehaviorControl/Skills.h" 
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Configuration/GlobalOptions.h"

#include "Tools/Math/Geometry.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

#include "Representations/Modeling/RobotPose.h"

// default actions for GORE2022
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/DefaultPose.h"
#include "Representations/Configuration/GlobalOptions.h"


CARD(OppCornerKickCard,
{,
  CALLS(Stand),
  CALLS(Activity),
  CALLS(LookForward),
  CALLS(WalkToPoint),

  REQUIRES(DefaultPose),
	REQUIRES(FieldBall),
  REQUIRES(GameInfo),
	REQUIRES(GlobalOptions),
  REQUIRES(LibWalk),
  REQUIRES(OwnTeamInfo),
  REQUIRES(RobotPose),
});

class OppCornerKickCard : public OppCornerKickCardBase
{
  /**
   * @brief The condition that needs to be met to execute this card
   */
  bool preconditions() const override
  {
    return theGameInfo.kickingTeam != theOwnTeamInfo.teamNumber
        && theGameInfo.setPlay == SET_PLAY_CORNER_KICK 
        && !theDefaultPose.isOnDefaultPose;
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
};

MAKE_CARD(OppCornerKickCard);