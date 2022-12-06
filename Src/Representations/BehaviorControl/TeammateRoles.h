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
#include <vector>

STREAMABLE(TeammateRoles,
{
 ENUM(R2K_TEAM_ROLES,
  {,
    UNDEFINED,
    GOALKEEPER_NORMAL,
    GOALKEEPER_ACTIVE,
    DEFENSE_LEFT,
    DEFENSE_MIDDLE,
    DEFENSE_RIGHT,
    OFFENSE_LEFT,
    OFFENSE_MIDDLE,
    OFFENSE_RIGHT,
  });


 
  bool isTacticalGoalKeeper(const int robotNumber) const;
  bool isTacticalOffense(const int robotNumber) const;
  bool isTacticalDefense(const int robotNumber) const;
  bool playsTheBall(const int robotNumber) const;

  int operator[](const size_t i) const;
  int& operator[](const size_t i),

    (std::vector<int>) ({ TeammateRoles::GOALKEEPER_NORMAL,
                          TeammateRoles::DEFENSE_LEFT,
                          TeammateRoles::DEFENSE_RIGHT,
                          TeammateRoles::OFFENSE_LEFT,
                          TeammateRoles::OFFENSE_RIGHT,
                          TeammateRoles::UNDEFINED })roles, /** The R2K role assignment for all robots in the team */
    (int)(-1) captain, /**< The number of the robot which calculated this role assignment. */
    (unsigned)(0) timestamp, /**< The timestamp when this role assignment has been calculated. */ 
});
