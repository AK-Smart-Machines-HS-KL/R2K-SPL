/**
 * @file ReferenceCard.cpp
 * @author Adrian Müller
 * @version: 1.3
 * @date: 8/2022
 *
 * Note: have a look at the pre-conditions ;-) this is reference code
 * 
 * Functions, values, side effects: this card qualifies for the one OFFENSE player only who is closest to the ball
 * Details: actually, this a one2one copy of the CodeReleaseKickAtGoalCard, BUT it is qualified only iff
 * - TeamBehaviorStatus: !SPARSE_GAME (ie NORMAL, OFFENSIVE,DEFENSIVE)
 * - PlayerRole: I am the striker(ie playsTheBall() ie. I am closest to the ball
 * - TeammateRoles: I am an OFFENSE player
 * v.1.2 added example code for
 * - usage of supporterIndex()
 * - usage of dynamic role isGoalkeeper()
 * 
 * v1.3: changed TeamBehaviorStatus to not SPARSE_MODE
 * 
 */

// B-Human includes
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Modeling/RobotPose.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Tools/Math/BHMath.h"

// this is the R2K specific stuff
#include "Representations/BehaviorControl/TeamBehaviorStatus.h" 
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/GameInfo.h"   // eg theGameInfo.setPlay == SET_PLAY_NONE, SET_PLAY_CORNER_KICK etc.

CARD(ReferenceCard,
  { ,
    CALLS(Activity),
    CALLS(GoToBallAndKick),
    CALLS(LookForward),
    CALLS(Stand),
    CALLS(WalkAtRelativeSpeed),
    USES(GameInfo),
    REQUIRES(FieldBall),
    REQUIRES(FieldDimensions),
    REQUIRES(RobotPose),
    REQUIRES(TeamBehaviorStatus),  // R2K 
    REQUIRES(TeammateRoles),  // R2K
    REQUIRES(PlayerRole),  // R2K
    REQUIRES(RobotInfo),      // R2K
    DEFINES_PARAMETERS(
    {,
      (float)(0.8f) walkSpeed,
      (int)(1000) initialWaitTime,
      (int)(7000) ballNotSeenTimeout,
    }),
  });

class ReferenceCard : public ReferenceCardBase
{
  bool preconditions() const override
  {
    return
      theGameInfo.setPlay == SET_PLAY_NONE && // normal play - not SET_PLAY_CORNER_KICK etc
      // theRobotPose.isInbeetween(xmin,xmay,ymin,ymax)  // grid -1, +1, -3, +3 == nahe Mittelinie
      theTeammateRoles.playsTheBall(theRobotInfo.number)&&  // I am the striker
      !theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number) &&  // goalie or substitute goalie
      thePlayerRole.supporterIndex() < thePlayerRole.numOfActiveSupporters && // I am not the right most player

      !(theTeamBehaviorStatus.teamActivity == TeamBehaviorStatus::R2K_SPARSE_GAME) &&


      // note: just use the real robot number for tactic look up
      theTeammateRoles.isTacticalOffense(theRobotInfo.number); // OFFENSE_RIGHT, OFFENSE_MIDDLE, OFFENSE_LEFT
      // be more specific:
      // theTeammateRoles.roles[theRobotInfo.number-1] == TeammateRoles::OFFENSE_RIGHT;  // my recent R2K strategy dependent role

    // ToDo: OFFENSE_LEFT,  ANY_OFFENSE (l,m,r)
  }

  bool postconditions() const override
  {
    return !preconditions();
  }

  option
  {
    theActivitySkill(BehaviorStatus::referenceCard);

    initial_state(start)
    {
      transition
      {
        if (state_time > initialWaitTime)
          goto goToBallAndKick;
      }

      action
      {
        theLookForwardSkill();
        theStandSkill();
      }
    }

    state(goToBallAndKick)
    {
      transition
      {
        if (!theFieldBall.ballWasSeen(ballNotSeenTimeout))
          goto searchForBall;
      }

      action
      {
        theGoToBallAndKickSkill(calcAngleToGoal(), KickInfo::walkForwardsLeft);
      }
    }

    state(searchForBall)
    {
      transition
      {
        if (theFieldBall.ballWasSeen())
          goto goToBallAndKick;
      }

      action
      {
        theLookForwardSkill();
        theWalkAtRelativeSpeedSkill(Pose2f(walkSpeed, 0.f, 0.f));
      }
    }
  }

    Angle calcAngleToGoal() const
  {
    return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
  }
};

MAKE_CARD(ReferenceCard);
