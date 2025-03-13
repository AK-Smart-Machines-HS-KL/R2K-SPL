/**
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
 * v.14 card active, changed kick type to be faster (Adrian)  3/25
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
#include "Representations/Communication/TeamCommStatus.h"

#include "Tools/Math/Geometry.h"



CARD(OwnKickoffCard,
{,
  CALLS(Stand),
  CALLS(Activity),
  CALLS(LookForward),
  CALLS(GoToBallAndKick),

  REQUIRES(FieldBall),
  REQUIRES(RobotPose),
  REQUIRES(RobotInfo),
  REQUIRES(FieldDimensions),
  REQUIRES(FrameInfo),
  REQUIRES(OwnTeamInfo),
  REQUIRES(GameInfo),
  REQUIRES(ExtendedGameInfo),
  REQUIRES(TeamBehaviorStatus),
  REQUIRES(TeammateRoles),
  REQUIRES(TeamCommStatus),

  DEFINES_PARAMETERS(
  {,
    (bool)(false) footIsSelected,  // freeze the first decision
    (bool)(true) leftFoot,
    (Vector2f)(Vector2f(1000.0f, -340.0f)) kickTarget, // Based on 20_deg setup angle in ready card; This is a 20 degree shot
  }),
});

class OwnKickoffCard : public OwnKickoffCardBase
{
  KickInfo::KickType kickType;

  /**
   * @brief all tactical offense try to kick the ball
   * 
   */
  bool preconditions() const override
  {
    return theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
      && theExtendedGameInfo.timeSincePlayingStarted < 12000 // 5sec
      && theGameInfo.state == STATE_PLAYING
      && theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive)   // I am the striker
      && theTeammateRoles.isTacticalOffense(theRobotInfo.number); // my recent role;
  }

  /**
   * @brief The condition that needs to be met to exit the this card
   */
  bool postconditions() const override
  {
    return !theExtendedGameInfo.timeSincePlayingStarted < 13000;
  };

  void execute() override
  {
    theActivitySkill(BehaviorStatus::ownKickoff);
    if (!footIsSelected) {  // select only once
      footIsSelected = true;
      leftFoot = theFieldBall.positionRelative.y() < 0;
    }
    KickInfo::KickType kickType = leftFoot ? KickInfo::walkForwardsLeft : KickInfo::walkForwardsRight;
    theGoToBallAndKickSkill(calcAngleToGoal(), kickType, true, 5000); 
    }
 
  Angle calcAngleToGoal() const
  {
    // return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, -3000.f)).angle();
    return (theRobotPose.inversePose * Vector2f(-1000.f, -1000.f)).angle();

  }
};

MAKE_CARD(OwnKickoffCard);