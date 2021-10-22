/**
 * @file NoTeamCard.cpp
 *
 * This file implements card that is active in situations where there is no team behavior.
 *
 * @author Arne Hasselbring
 */

#include "Representations/BehaviorControl/TeamSkills.h"
#include "Tools/BehaviorControl/Framework/Card/TeamCard.h"

TEAM_CARD(NoTeamCard,
{,
  CALLS(Role),
  CALLS(TeamActivity),
  CALLS(TeammateRoles),
  CALLS(TimeToReachBall),
});

class NoTeamCard : public NoTeamCardBase
{
  bool preconditions() const override
  {
    return true;
  }

  bool postconditions() const override
  {
    return true;
  }

  void execute() override
  {
    theTeamActivitySkill(TeamBehaviorStatus::noTeam);
    theTimeToReachBallSkill(TimeToReachBall());
    theTeammateRolesSkill(TeammateRoles());
    theRoleSkill(PlayerRole());
  }
};

MAKE_TEAM_CARD(NoTeamCard);
