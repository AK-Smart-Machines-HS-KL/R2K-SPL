/**
/**
 * @file ReferenceCard.cpp
 * @author Adrian Müller
 * @version: 1.0
 *
 * Functions, values, side effects: this card qualifies for the one OFFENSE player only who is closest to the ball
 * Details: actually, this a one2one copy of the CodeReleaseKickAtGoalCard, BUT it is qualified only iff
 * - TeamBehaviorStatus: NORMAL_GAME
 * - PlayerRole: I am the striker(ie playsTheBall() ie. I am closest to the ball
 * - TeammateRoles: I am an OFFENSE player
 *
 * Note: checkout the pre-conditions ;-) this is reference code
 * 
 * 
 * To Do: 
 */
#

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

CARD(ReferenceCard,
  { ,
    CALLS(Activity),
    CALLS(GoToBallAndKick),
    CALLS(LookForward),
    CALLS(Stand),
    CALLS(WalkAtRelativeSpeed),
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
      thePlayerRole.playsTheBall() &&  // I am the striker
      theTeamBehaviorStatus.teamActivity == TeamBehaviorStatus::R2K_NORMAL_GAME &&
      theTeammateRoles.roles[theRobotInfo.number-1] == TeammateRoles::OFFENSE;  // my recent role
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
