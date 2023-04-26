/**
 * @file TeammateRoles.cpp
 *
 * This file implements the implementation of the TeammateRoles skill.
 *
 * @author Arne Hasselbring
 */

#include "Representations/BehaviorControl/Libraries/LibCheck.h"
#include "Representations/BehaviorControl/TeamSkills.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"

#include "Representations/BehaviorControl/TeammateRoles.h"  // GOALKEEPER, OFFENSE, DEFENSE

TEAM_SKILL_IMPLEMENTATION(TeammateRolesImpl,
{,
  IMPLEMENTS(TeammateRoles),
  REQUIRES(LibCheck),
  MODIFIES(TeamBehaviorStatus),
});


bool TeammateRoles::isTacticalGoalKeeper(const int robotNumber) const{
  ASSERT(robotNumber >= Global::getSettings().lowestValidPlayerNumber && robotNumber <= Global::getSettings().highestValidPlayerNumber);
  return (
    roles[robotNumber - 1] == TeammateRoles::GOALKEEPER_NORMAL ||
    roles[robotNumber - 1] == TeammateRoles::GOALKEEPER_ACTIVE
  );
}

bool TeammateRoles::isTacticalDefense(const int robotNumber) const {
  ASSERT(robotNumber >= Global::getSettings().lowestValidPlayerNumber && robotNumber <= Global::getSettings().highestValidPlayerNumber);
  return (
    roles[robotNumber - 1] == TeammateRoles::DEFENSE_RIGHT ||
    roles[robotNumber - 1] == TeammateRoles::DEFENSE_MIDDLE ||
    roles[robotNumber - 1] == TeammateRoles::DEFENSE_LEFT
  );
}

bool TeammateRoles::isTacticalOffense(const int robotNumber) const {
  ASSERT(robotNumber >= Global::getSettings().lowestValidPlayerNumber && robotNumber <= Global::getSettings().highestValidPlayerNumber);
  return (
    roles[robotNumber - 1] == TeammateRoles::OFFENSE_RIGHT ||
    roles[robotNumber - 1] == TeammateRoles::OFFENSE_MIDDLE ||
    roles[robotNumber - 1] == TeammateRoles::OFFENSE_LEFT
    );
}

bool TeammateRoles::playsTheBall(int robotNumber) const {
  ASSERT(robotNumber >= Global::getSettings().lowestValidPlayerNumber && robotNumber <= Global::getSettings().highestValidPlayerNumber);

  return (captain == robotNumber);
}


class TeammateRolesImpl : public TeammateRolesImplBase
{
  void execute(const TeammateRoles& p) override
  {
    theTeamBehaviorStatus.teammateRoles = p.teammateRoles;
    theLibCheck.inc(LibCheck::teammateRoles);
  }
};

MAKE_TEAM_SKILL_IMPLEMENTATION(TeammateRolesImpl);
