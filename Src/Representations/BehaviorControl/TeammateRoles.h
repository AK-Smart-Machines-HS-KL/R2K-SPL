/**
 * @file TeammateRoles.h
 *
 * This file declares a representation of a team role assignment in the 2019 behavior.
 *
 * @author Arne Hasselbring
 * @author Adrian Mueller, R2K
 * - added R2K_TEAM_ROLES, 7/22 
 * v1.2: (9/22) added wrapper functions isTacticalGoalKeeper() and others
 */

#pragma once

#include "Tools/Streams/AutoStreamable.h"
#include "Tools/Settings.h"
#include <vector>
#include "Representations/Communication/RobotInfo.h"

STREAMABLE(TeammateRoles,
{
 ENUM(R2K_TEAM_ROLES,
  {,
    GOALKEEPER_NORMAL,  // 0
    GOALKEEPER_ACTIVE,
    DEFENSE_LEFT,       // 2
    DEFENSE_MIDDLE,
    DEFENSE_RIGHT,
    OFFENSE_LEFT,       // 5
    OFFENSE_MIDDLE,
    OFFENSE_RIGHT,      // 7
    UNDEFINED,
  });


 
  bool isTacticalGoalKeeper(const int robotNumber) const;
  bool isTacticalOffense(const int robotNumber) const;
  bool isTacticalDefense(const int robotNumber) const;
  bool playsTheBall(const int robotNumber) const;
  bool playsTheBall(const RobotInfo *info, const bool wifi) const;   // online/offline variant

  // communication offline utils 

  int  defenseRoleIndex(const int robotNumber) const;    // 0 = left most defense by robot number, 1 = one more defense to the right up to n
  int  defenseRoleIndex(const RobotInfo* info) const;
  int  offenseRoleIndex(const int robotNumber) const;    // 0 = right most defense, 1 = one more defense to the left
  int  anyRoleIndex(const int robotNumber) const;    // 0 = right most first any role 



  int operator[](const size_t i) const;
  int& operator[](const size_t i),

    (std::vector<int>) (std::vector<int>(Settings::highestValidPlayerNumber))roles, /** The R2K role assignment for all robots in the team */
    (int)(-1) captain, /**< The number of the robot which calculated this role assignment. */
    (unsigned)(0) timestamp, /**< The timestamp when this role assignment has been calculated. */ 
});
