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

// 0 = left most defense by robot number, 1 = one more defense to the right up to n
int TeammateRoles::defenseRoleIndex(int robotNumber) const {
  ASSERT(robotNumber >= Global::getSettings().lowestValidPlayerNumber && robotNumber <= Global::getSettings().highestValidPlayerNumber);
  int theDefense = -1;

  for (int i = 0; i < 5; i++) {
    theDefense++;
    if (TeammateRoles::isTacticalDefense(i + 1) && i+1 == robotNumber)
      break;
  }
  // OUTPUT_TEXT("dI " << theDefense << " " << robotNumber);
  return theDefense-1;
}

// 0 = right most defense, 1 = one more defense to the left 
int TeammateRoles::offenseRoleIndex(int robotNumber) const {
  ASSERT(robotNumber >= Global::getSettings().lowestValidPlayerNumber && robotNumber <= Global::getSettings().highestValidPlayerNumber);
  int theOffense = -1;
  for (int j = 4; j >= 0; j--) {
    theOffense++;
    if (TeammateRoles::isTacticalOffense(j + 1) && j + 1 == robotNumber) {
      break;
    }
  }
  return theOffense;
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
