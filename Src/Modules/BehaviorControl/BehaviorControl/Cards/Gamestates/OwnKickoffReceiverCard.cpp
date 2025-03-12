/**
 * @file OwnKickoffReceiverCard.cpp
 * @author Adrian Mueller
 * @brief Buddy walking into opp. fied, awaiting ball from kickoff
 * @version 1
 * @date 2025-03-06
 * 
 * Behavior: 
 * OwnKickOff: During the Own Kickoff, Robot 5 attempts to kick the ball 20_deg to the right
 * this card: goe into opp field. wait

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
#include "Representations/BehaviorControl/DefaultPose.h"

#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/BehaviorControl/PlayerRole.h"


CARD(OwnKickoffReceiverCard,
  {,
    CALLS(Activity),
    CALLS(LookActive),
    CALLS(WalkToPoint),
    CALLS(InterceptBall),
    REQUIRES(FieldBall),
    REQUIRES(FieldDimensions),
    REQUIRES(RobotPose),
    REQUIRES(RobotInfo),
    REQUIRES(PlayerRole),
    REQUIRES(FrameInfo),
    REQUIRES(OwnTeamInfo),
    REQUIRES(GameInfo),
    REQUIRES(ExtendedGameInfo),
    REQUIRES(TeamBehaviorStatus),
    REQUIRES(TeammateRoles),
    REQUIRES(TeamCommStatus),  // wifi on off?
    REQUIRES(DefaultPose),  // wifi on off?

  DEFINES_PARAMETERS(
  {,
      

  }),
});

class OwnKickoffReceiverCard : public OwnKickoffReceiverCardBase
{
  KickInfo::KickType kickType;

  /**
   * @brief all tactical offense try to kick the ball
   *
   */
  bool preconditions() const override
  {
    return theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
      && theExtendedGameInfo.timeSincePlayingStarted < 12000 // 10sec
      && theGameInfo.state == STATE_PLAYING
      // && !(theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive))   // I am not the striker
      && thePlayerRole.supporterIndex() == thePlayerRole.numOfActiveSupporters - 1 // second right most bot
      // && 4 == theRobotInfo.number
      && theTeammateRoles.isTacticalOffense(theRobotInfo.number); // my recent role;
  }

  /**
   * @brief The condition that needs to be met to exit the this card
   */
  bool postconditions() const override
  {
    return theExtendedGameInfo.timeSincePlayingStarted >= 12000;
  };

  void execute() override
  {

    theActivitySkill(BehaviorStatus::ownKickoff);
    theLookActiveSkill(); // Head Motion Request

    Pose2f targetRelative = theRobotPose.toRelative(Pose2f(theFieldDimensions.xPosOpponentFieldBorder, -2000.f));
    theWalkToPointSkill(targetRelative, 1.0f, true);

    }
};

MAKE_CARD(OwnKickoffReceiverCard);
