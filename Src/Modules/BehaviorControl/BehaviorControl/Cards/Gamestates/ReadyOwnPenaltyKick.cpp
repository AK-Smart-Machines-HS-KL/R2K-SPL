/**
 * @file ReadyOwnPenaltyKickCard.cpp
 * @author Andy Hobelsberger
 * @brief Set individual ready behavior for the own Kickoff
 * @version 1.2
 * @date 2022-03-16
 * 
 * Behavior:
 * Sets up Robot 5 Behind the Ball so that it may be kicked towards the opponents Field. 
 * 
 * v1.1: Card dynamically select robot instead hardcoding number  (Adrian)
 * v1.2. Card migrated (Adrian)
 * v 1.3. (Adrian) using theTeammateRoles.offenseRoleIndex(theRobotInfo.number) 
 * 
 */

#include "Representations/BehaviorControl/DefaultPose.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Configuration/GlobalOptions.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h" 
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Communication/TeamCommStatus.h"

#include "Tools/Math/Geometry.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"


CARD(ReadyOwnPenaltyKickCard,
  {,
    CALLS(Activity),
    CALLS(LookActive),
    CALLS(WalkToPoint),
    REQUIRES(DefaultPose),
    REQUIRES(GameInfo),
    REQUIRES(OwnTeamInfo),
    REQUIRES(RobotPose),
    REQUIRES(RobotInfo),
    REQUIRES(TeammateRoles),
    REQUIRES(FieldDimensions),
    REQUIRES(TeamCommStatus),  // wifi on off?
  });

class ReadyOwnPenaltyKickCard : public ReadyOwnPenaltyKickCardBase
{
  /**
   * @brief walk to default position and exit to DefaultCards
   */
  bool preconditions() const override
  {
    return theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
      && theGameInfo.state == STATE_READY
      //&& theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive);
      && theTeammateRoles.offenseRoleIndex(theRobotInfo.number)==0;
  }

  /**
   * @brief The condition that needs to be met to exit the this card
   */
  bool postconditions() const override
  {
    return !preconditions();
  }

  void execute() override
  {

    theActivitySkill(BehaviorStatus::defaultBehavior);
    Pose2f targetAbsolute = Pose2f(0 ,theFieldDimensions.xPosOpponentPenaltyMark - 300, 0);
    Pose2f targetRelative = theRobotPose.toRelative(targetAbsolute);

    theLookActiveSkill(); // Head Motion Request
    theWalkToPointSkill(targetRelative);
  }
  
};

MAKE_CARD(ReadyOwnPenaltyKickCard);