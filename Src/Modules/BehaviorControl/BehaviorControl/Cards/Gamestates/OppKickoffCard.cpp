/**
 * @file OppKickoffCard.cpp
 * @author Andy Hobelsberger
 * @brief Covers Opponent Free Kick
 * @version 1.1
 * @date 2022-11-22
 *  
 * Makes the Offensive and Defensive robots wait until the ball has been Kicked after Kickoff
 * 
 * V1.1 Card migrated (Nicholas)
 * V1.2 Wait (do nothing): for 10 sec after STATE_PLAYING was seen first
        or opponent has moved the ball for at least .25cm (Adrian)
        Note: using ExtendedGameInfo to get time since last STATE_PLAYING
 */

#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Infrastructure/ExtendedGameInfo.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Communication/RobotInfo.h"

#include "Representations/Modeling/RobotPose.h"

#include "Tools/Math/Geometry.h"


CARD(OppKickoffCard,
  { ,
    CALLS(Stand),
    CALLS(Activity),
    CALLS(LookForward),
    CALLS(GoToBallAndKick),

    REQUIRES(FieldBall),
    REQUIRES(FieldDimensions),
    REQUIRES(FrameInfo),
    REQUIRES(GameInfo),
    REQUIRES(ExtendedGameInfo),
    REQUIRES(RobotInfo),
    REQUIRES(RobotPose),
    REQUIRES(OwnTeamInfo),
    REQUIRES(TeamBehaviorStatus),
    REQUIRES(TeammateRoles),

    DEFINES_PARAMETERS(
    {,
      (float)(200.0f) ballKickedThreshold,
    }),
  });

// 

class OppKickoffCard : public OppKickoffCardBase
{

  /**
   * @brief 
   * IF the other Team is kicking 
   * AND It's the first 10 seconds of the game
   * AND we're in the playing state
   * AND the ball is still near the cneter point
   * AND the Robot is not the Goalie
   */

  bool preconditions() const override
  {

    return (
      theGameInfo.kickingTeam != theOwnTeamInfo.teamNumber
      && !theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number)
      && theFieldBall.positionOnField.norm() < ballKickedThreshold
      // && theGameInfo.secondaryTime > 0  // does not work intentionally in GC 
      && theExtendedGameInfo.timeSincePlayingStarted < 10000
      );
  };

  /**
   * @brief Not Precondition
   */
  bool postconditions() const override
  {
    return !preconditions();
  }

  option
  {
    theActivitySkill(BehaviorStatus::oppKickoff);

    initial_state(init)
    {
      transition
      {
        if (theTeammateRoles.isTacticalOffense(theRobotInfo.number)) {
          goto forward;
        } else if (theTeammateRoles.isTacticalDefense(theRobotInfo.number)) {
          goto back;
        }
      }

      action
      {
        theLookForwardSkill();
        theStandSkill();
      }
    }

    state(forward)
    {
      transition
      {}

      action
      {
        theLookForwardSkill();
        theStandSkill();
      }
    }

    state(back)
    {
      transition
      {}

      action
      {
        theLookForwardSkill();
        theStandSkill();
      }
    }
  }
};

MAKE_CARD(OppKickoffCard);