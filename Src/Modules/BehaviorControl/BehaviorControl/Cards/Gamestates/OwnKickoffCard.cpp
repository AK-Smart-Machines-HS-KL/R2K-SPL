/*
 * @file OwnKickoffCard.cpp
 * @author Andy Hobelsberger
 * @brief Covers Own Kickoff
 * @version 0.1
 * @date 2022-11-22
 * 
 * Behavior: During the Own Kickoff, Robot 5 attempts to kick the ball 20_deg to the left 
 * 
 * V1.1 Card migrated (Nicholas)
 * V 1.2. changed to long kick (Adrian)
 * v 1.3 card disabled
 * 
 * Note: all tactical offense try to kick the ball. So default position is crucial
 */

#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Infrastructure/ExtendedGameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/BehaviorControl/Shots.h"

#include "Tools/Math/Geometry.h"


// Representations
#include "Representations/BehaviorControl/Shots.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Tools/Math/Geometry.h"
#include "Representations/Communication/TeamData.h"

CARD(OwnKickoffCard,
{
    ,
    CALLS(Stand),
    CALLS(Activity),
    CALLS(LookForward),
    CALLS(GoToBallAndKick),
    CALLS(WalkToPoint),
    CALLS(WalkAtRelativeSpeed),
    CALLS(LookActive),
    REQUIRES(FieldBall),
    REQUIRES(RobotPose),
    REQUIRES(RobotInfo),
    REQUIRES(Shots),
    REQUIRES(FieldDimensions),
    REQUIRES(FrameInfo),
    REQUIRES(OwnTeamInfo),
    REQUIRES(GameInfo),
    REQUIRES(ExtendedGameInfo),
    REQUIRES(TeamBehaviorStatus),
    REQUIRES(TeammateRoles),
    REQUIRES(TeamData),

      DEFINES_PARAMETERS(
      {,
        (bool)(false) footIsSelected,  // freeze the first decision
        (bool)(true) leftFoot,
        (Vector2f)(Vector2f(1000.0f, -340.0f)) kickTarget, // Based on 20_deg setup angle in ready card; This is a 20 degree shot
        (unsigned int)(500) initalCheckTime,
        /*params goal shot*/
        (bool)(false) done,
        (Shot) currentShot,
        (unsigned int) (0) timeLastFail,
        (unsigned int) (3000) cooldown,
        }),
});

class OwnKickoffCard : public OwnKickoffCardBase
{
  KickInfo::KickType kickType;

  /**
   * @brief all tactical offense try to kick the ball
   */
   
  bool preconditions() const override
  {
    return theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
      && theExtendedGameInfo.timeSincePlayingStarted < 10000 // 10sec
      && theGameInfo.state == STATE_PLAYING
      && theTeammateRoles.isTacticalOffense(theRobotInfo.number); // my recent role;
      /*pre conds for goal shot*/
            // theFieldBall.ballWasSeen() &&
      theFieldBall.positionRelative.norm() < 600
      && theFrameInfo.getTimeSince(timeLastFail) > cooldown
      && theShots.goalShot.failureProbability < 0.70
      && theFieldBall.teamPositionOnField.x() > theRobotPose.translation.x();
  }
  bool postconditions() const override
  {
    return done;   
  }

  option
  {
    theActivitySkill(BehaviorStatus::goalShotCard);

    initial_state(align)
    {
      done = false;
      Angle angleToGoal = (Vector2f(4500, 0) - theRobotPose.translation).angle() - theRobotPose.rotation; 
      transition
      {
        if(abs(angleToGoal.normalize()) < 20_deg || state_time > 2000) {
          goto check;
        }
      }

      action
      {
        // face the goal
        theWalkAtRelativeSpeedSkill(Pose2f(std::clamp((float) angleToGoal, -1.f, 1.f)));
        // look around
        theLookActiveSkill();
      }
    }

    state(check)
    {
      done = false;
      transition
      {
        if(state_time > initalCheckTime) {
          currentShot = theShots.goalShot;
          OUTPUT_TEXT("Locking Target: (" << currentShot.target.x() << ", " << currentShot.target.y() << ")\n" << currentShot);
          if (currentShot.failureProbability > 0.3) {
            OUTPUT_TEXT("Aborting! shot too likely to fail");
            timeLastFail = theFrameInfo.time;
            goto passRight;
          }
          
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
        if(theGoToBallAndKickSkill.isDone()) {
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
    state(passRight)
    {
      action
      { // no long shot possible
        int dist = (int)Geometry::distance(theFieldBall.teamPositionOnField, theRobotPose.translation);  
        if(dist < 1500) {  // ball is on the center pos
          if (!footIsSelected) {  // select only once
            footIsSelected = true;
            leftFoot = theFieldBall.positionRelative.y() < 0;
          }
          KickInfo::KickType kickType = leftFoot ? KickInfo::forwardFastLeftLong : KickInfo::forwardFastRightLong;
          // theGoToBallAndKickSkill(calcAngleToGoal(), kickType, true); 
          theGoToBallAndKickSkill(calcAngleToPass(), KickInfo::forwardFastLeft, true, 2500);
          done = true;
        }
        else { // i am the passing target ie right defender
         auto target = Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f);
         theWalkToPointSkill(target, 1.0f, false,false,false);   
         theLookActiveSkill();
        }
      }
    }
  }

   Angle calcAngleToPass() const
      {
        return (theRobotPose.inversePose * Vector2f(1000.f, -3500.f)).angle();
      }
      
  bool aBuddyIsChasingOrClearing() const
    {
      for (const auto& buddy : theTeamData.teammates) 
      {
        if (// buddy.theBehaviorStatus.activity == BehaviorStatus::OffenseChaseBallCard ||
          // buddy.theBehaviorStatus.activity == BehaviorStatus::clearOwnHalfCard ||
          // buddy.theBehaviorStatus.activity == BehaviorStatus::clearOwnHalfCardGoalie ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::defenseLongShotCard ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::goalieLongShotCard ||
          // buddy.theBehaviorStatus.activity == BehaviorStatus::goalShotCard ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::offenseForwardPassCard )
          // buddy.theBehaviorStatus.activity == BehaviorStatus::offenseReceivePassCard)
          return true;
      }
      return false;
    }
};

MAKE_CARD(OwnKickoffCard);