/**
 * @file TeammateRoles.cpp
 *
 * This file implements the implementation of the TeammateRoles skill.
 *
 * @author Arne Hasselbring, Adrian Müller
 * @version 1.1.
 * v1.0
 * nartive code B-Human
 * 
 * v1.1.
 * added predicates playsTheBall/1 isTacticalOffense, isTacticalGoalKeeper/1, isTacticalDefense
 * Notes: all predicates checks the bots role via TeammateRoles.roles[]
 * 
 * v1.2
 * now taking care of on/offline situation (see wifipred)
 * added playsTheBall/2, 
 * defenseRoleIndex/1, offenseRoleIndex/2 loop over isTacticalDefense and isTacticalOffense, resp. 
 */

#include <Tools/Communication/BNTP.h>
#include "Representations/BehaviorControl/Libraries/LibCheck.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/BehaviorControl/TeamSkills.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"
#include "Representations/BehaviorControl/TeammateRoles.h"  // GOALKEEPER, OFFENSE, DEFENSE
#include "Representations/BehaviorControl/TeamBehaviorStatus.h" 
#include "Representations/Communication/TeamCommStatus.h" //wifi state



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


// online/offline variant
// 
bool TeammateRoles::playsTheBall(const RobotInfo * info, const bool wifiPred) const {
  if (info->penalty != PENALTY_NONE) return false;
  if (wifiPred) 
    return TeammateRoles::playsTheBall(info->number);
  else  // union of typical offline conditions, who is gonna go for the ball 8-)
    return 0 == TeammateRoles::offenseRoleIndex(info->number) ||  TeammateRoles::playsTheBall(info->number);
}  

bool TeammateRoles::playsTheBall(const int robotNumber) const {
  ASSERT(robotNumber >= Global::getSettings().lowestValidPlayerNumber && robotNumber <= Global::getSettings().highestValidPlayerNumber);
  return (captain == robotNumber);
}

int TeammateRoles::defenseRoleIndex(const RobotInfo* info)const{
  ASSERT(info->number >= Global::getSettings().lowestValidPlayerNumber && info->number <= Global::getSettings().highestValidPlayerNumber);
  int theDefense = -1;

  for (int i = 0; i < 5; i++) {
    // if(info->penalty == PENALTY_NONE) theDefense++;
    if (TeammateRoles::isTacticalDefense(i + 1) && i + 1 == info->number)
      break;
  }
  OUTPUT_TEXT("dI " << theDefense - 1 << " for bot " << info->number);
  return theDefense -1;  //
}

// 0 = left most defense by robot number, 1 = one more defense to the right up to n
int TeammateRoles::defenseRoleIndex(int robotNumber) const {
  ASSERT(robotNumber >= Global::getSettings().lowestValidPlayerNumber && robotNumber <= Global::getSettings().highestValidPlayerNumber);
  int theDefense = -1;

  if (!TeammateRoles::isTacticalDefense(robotNumber)) return theDefense;
  for (int j = 0; j <=4; j++) {
    if (TeammateRoles::isTacticalDefense(j + 1)) {
      theDefense++;
    }
    if (j + 1 == robotNumber) {
      // OUTPUT_TEXT("bot " << robotNumber << " defenseRoleInxes " << theDefense);
      return theDefense;
    }
  }
  return theDefense;
}

// 0 = right most defense, 1 = one more defense to the left 
// returns -1 for defense players and goalie 

int TeammateRoles::offenseRoleIndex(const int robotNumber) const {
  ASSERT(robotNumber >= Global::getSettings().lowestValidPlayerNumber && robotNumber <= Global::getSettings().highestValidPlayerNumber);
  int theOffense = -1;
  if (!TeammateRoles::isTacticalOffense(robotNumber)) return theOffense;
  for (int j = 4; j >= 0; j--) {
    if (TeammateRoles::isTacticalOffense(j + 1)){
      theOffense++;
    }
    if  (j + 1 == robotNumber) {
      // OUTPUT_TEXT("bot " << robotNumber << " offenseRoleInxes " << theOffense);
      return theOffense;;
    }
  }
  return theOffense;
}


// 0 = right most defense, 1 = one more defense to the left 
int TeammateRoles::anyRoleIndex(int robotNumber) const {
  OUTPUT_TEXT("deprecated call TeammateRoles::anyRoleIndex");

  ASSERT(robotNumber >= Global::getSettings().lowestValidPlayerNumber && robotNumber <= Global::getSettings().highestValidPlayerNumber);
  int thePlayer = -1;

  // if (robotNumber == 5) return 0;
  for (int j = 4; j >= 0; j--) {
    thePlayer++;

    /*
    if(theRobotInfo.penalty == PENALTY_NONE)
      break;
    }
    */
    // if (theOwnTeamInfo) break;
  }
  return thePlayer;
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
