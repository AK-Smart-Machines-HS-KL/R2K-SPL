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
 * V1.2 Card migrated (Nicholas)
 */

#include "Representations/BehaviorControl/Skills.h"  //Path2Target
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Configuration/GlobalOptions.h"

#include "Tools/Math/Geometry.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

// default actions for GORE2022
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/DefaultPose.h"
#include "Representations/Configuration/GlobalOptions.h"


CARD(OppCornerKickCard,
{,
  CALLS(Stand),
  CALLS(Activity),
  CALLS(LookForward),
	CALLS(PathToTarget),

  REQUIRES(DefaultPose),
	REQUIRES(GlobalOptions),
  REQUIRES(OwnTeamInfo),
  REQUIRES(GameInfo),
	REQUIRES(FieldBall),
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
				thePathToTargetSkill(theGlobalOptions.walkSpeed, Pose2f(theFieldBall.positionRelative.angle(), theDefaultPose.defaultPosition));
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