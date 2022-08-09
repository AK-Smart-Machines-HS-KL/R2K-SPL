/**
 * @file TeammateRoles.h
 *
 * This file declares a representation of a team role assignment in the 2019 behavior.
 *
 * @author Arne Hasselbring
 * @author Adrian Mueller, R2K
 * - added R2K_TEAM_ROLES, 7/22 
 */

#pragma once

#include "Tools/Streams/AutoStreamable.h"
#include <vector>

STREAMABLE(TeammateRoles,
{
 ENUM(R2K_TEAM_ROLES,
  {,
    UNDEFINED,
    GOALKEEPER,
    DEFENSE,
    OFFENSE,
  });

  int operator[](const size_t i) const;
  int& operator[](const size_t i),

    (std::vector<int>) ({ TeammateRoles::GOALKEEPER,
                          TeammateRoles::DEFENSE,
                          TeammateRoles::DEFENSE,
                          TeammateRoles::OFFENSE,
                          TeammateRoles::OFFENSE,
                          TeammateRoles::UNDEFINED })roles, /** The R2K role assignment for all robots in the team */
    (int)(-1) captain, /**< The number of the robot which calculated this role assignment. */
    (unsigned)(0) timestamp, /**< The timestamp when this role assignment has been calculated. */ 
});
