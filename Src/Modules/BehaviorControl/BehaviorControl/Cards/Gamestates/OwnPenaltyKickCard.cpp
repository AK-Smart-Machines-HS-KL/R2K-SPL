/**
 * @file OwnPenaltyKickCard.cpp 
 * @author Mohammed && Asfiya
 * @brief Covers the Penalty Kick: Own Team has Penalty Kick
 * @version 1.1
 * @date 2023-04-18
 * @version 1.2 online: supporterIndex, offline offenseIndex 
 *  apply in after penalty shootout only
 * - see ReadyOwnPenaly for ingame penalty
 * v 1.3: (Asrar) "theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive)" this is for online and offline role assignment
 *  v1.4: (Asrar) card is  for  ballWasSeenStickyPeriod (5000msec), i.e., bot assumes ball to be at the last-seen position
 *                 Applied this parameter by changing the postcondition().
 * Notes:
 *  - 
 *
 * 
 */


#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"


#include "Representations/BehaviorControl/Skills.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"

#include "Representations/Configuration/FieldDimensions.h"

#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/BehaviorControl/Shots.h"

#include "Representations/Modeling/RobotPose.h"
#include "Representations/Communication/TeamCommStatus.h"

#include "Tools/Math/Geometry.h"

 // Debug Drawings
#include "Tools/Debugging/DebugDrawings.h"

#define drawID "ObstaclesLR"

CARD(OwnPenaltyKickCard,
    { ,
      CALLS(Activity),
      CALLS(LookForward),
      CALLS(GoToBallAndKick),
      CALLS(LookActive),
      CALLS(Stand),
      CALLS(WalkAtRelativeSpeed),
      CALLS(PenaltyStrikerGoToBallAndKick),

      REQUIRES(FieldBall),
      REQUIRES(RobotPose),
      REQUIRES(RobotInfo),
      REQUIRES(FieldDimensions),
      REQUIRES(OwnTeamInfo),
      REQUIRES(GameInfo),
      REQUIRES(TeamBehaviorStatus),
      REQUIRES(TeammateRoles),
      REQUIRES(PlayerRole),
      REQUIRES(Shots),
      REQUIRES(FrameInfo),
      REQUIRES(TeamCommStatus),  // wifi on off?

      DEFINES_PARAMETERS(
             {,
                (unsigned int)(1000) initalCheckTime,
                (bool)(false) done,
                (Shot) currentShot,
                (int)(5000) ballWasSeenStickyPeriod,  // freeze the first decision
             }),
    });

class OwnPenaltyKickCard : public OwnPenaltyKickCardBase
{

  /**
   * @brief The condition that needs to be met to execute this card
   */
  void preProcess() override {
    DECLARE_DEBUG_DRAWING(drawID, "drawingOnField");
  }
  bool preconditions() const override
  {
    return theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
      && theGameInfo.setPlay == SET_PLAY_PENALTY_KICK
      && theGameInfo.state == STATE_PLAYING
      && theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive);  // I am the striker
  }

  /**
   * @brief The condition that needs to be met to exit the this card
   */
  bool postconditions() const override
  {
    return !theFieldBall.ballWasSeen(ballWasSeenStickyPeriod)
      ||
      theGameInfo.kickingTeam != theOwnTeamInfo.teamNumber
      || theGameInfo.setPlay != SET_PLAY_PENALTY_KICK;
  }

  option
  {
    theActivitySkill(BehaviorStatus::ownPenaltyKick);

  initial_state(align)
  {

    done = false;
    Angle angleToGoal = (Vector2f(4500, 0) - theRobotPose.translation).angle() - theRobotPose.rotation;
    transition
    {
      if (abs(angleToGoal.normalize()) < 10_deg || state_time > 2000) 
    
     {
        goto check;
      }
    }

      action
    {
      theWalkAtRelativeSpeedSkill(Pose2f(angleToGoal));
      theLookActiveSkill();
    }
  }


  state(check)
  {
    done = false;
    transition
    {
      if (state_time > initalCheckTime) {
        currentShot = theShots.goalShot;
    
        goto kick;
      }
    }
      action
    {
      theLookActiveSkill();
      theStandSkill();
    }
  }

  state(kick)
  {
    transition
    {
      if (theGoToBallAndKickSkill.isDone()) {
         goto done;
      }
    }

      action
    {
      theGoToBallAndKickSkill(theRobotPose.toRelative(currentShot.target).angle(), currentShot.kickType.name);
    }
  }

  state(done)
  {
    action
    {
      reset();
      theLookActiveSkill();
      theStandSkill();
      done = true;
    }
  }
  }


};

MAKE_CARD(OwnPenaltyKickCard);