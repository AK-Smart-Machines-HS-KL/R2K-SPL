/**
 * @file ReadyOppKickoffCard.cpp
 * @author Andy Hobelsberger
 * @brief Set individual ready behavior for the Opponent Kickoff
 * @version 1.2
 * @date 2021-11-28
 * 
 * Behavior:
 * Sets up Robot 5 Blocking the center line of the own field during kickoff
 * 
 * version 1.1.
 * OFFENSIVE Robots march to their default position (see DefaultPoseProvider for details)
 * version 1.2 Migrated (Adrian). Offense Players step back 1m from default position to stay clear from center circle
 * version 1.3. (Andy): changing default position for offense players too even more distance to center circle)
 * version 1.4. (Adrian): code migration using theTeammateRoles.offenseRoleIndex
 */

#include "Representations/BehaviorControl/DefaultPose.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h" 
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Configuration/GlobalOptions.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Modeling/RobotPose.h"


#include "Tools/Math/Geometry.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Representations/BehaviorControl/TeammateRoles.h"


CARD(ReadyOppKickoffCard,
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
    

});

class ReadyOppKickoffCard : public ReadyOppKickoffCardBase
{
  /**
   * @brief walk to default position and exit to DefaultCards
   */
  bool preconditions() const override
  {
    return theGameInfo.kickingTeam != theOwnTeamInfo.teamNumber
      && theGameInfo.state == STATE_READY;
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

      Pose2f targetAbsolute = theDefaultPose.ownDefaultPose;

      int nOffenseFound = 0;
      int i;

      switch (theTeammateRoles.offenseRoleIndex(theRobotInfo.number)) {
      case 0: // right-most offense
        targetAbsolute = Pose2f(0, -1000, 0);
        break;
      case 1: // 
        targetAbsolute = Pose2f(0, -1300, -1300);
        break;
      case 2: // 
        targetAbsolute = Pose2f(0, -900, 1200);
        break;
      }

      Pose2f targetRelative = theRobotPose.toRelative(targetAbsolute);

      theLookActiveSkill(); // Head Motion Request
      theWalkToPointSkill(targetRelative, 1.0f, true);
    }
};

MAKE_CARD(ReadyOppKickoffCard);