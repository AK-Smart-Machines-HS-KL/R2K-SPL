/**
 * @file TeamBehaviorStatus.h
 *
 * This file declares a representation of the team behavior status in the 2019 behavior.
 *
 * @author Arne Hasselbring
 */

#pragma once

#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/BehaviorControl/TimeToReachBall.h"
#include "Tools/Communication/BHumanTeamMessageParts/BHumanMessageParticle.h"
#include "Tools/Streams/AutoStreamable.h"
#include "Tools/Streams/Enum.h"

STREAMABLE(TeamBehaviorStatus, COMMA public BHumanCompressedMessageParticle<TeamBehaviorStatus>
{
  ENUM(TeamActivity,
  {,
    noTeam,
    R2K_NORMAL_GAME,
    R2K_DEFENSIVE_GAME,
    R2K_OFFENSIVE_GAME,
    R2K_SPARSE_GAME,
  });
  static constexpr TeamActivity numOfTeamActivities = numOfTeamActivitys,

  (TeamActivity)(noTeam) teamActivity,
  (TimeToReachBall) timeToReachBall,
  (TeammateRoles) teammateRoles,
  (PlayerRole) role,
});
