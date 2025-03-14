/**
 * @file StandbyCard.cpp
 *
 * This file specifies the behavior for a robot in the SetStandby game state.
 *
 * @author Adrian Mueller
 */

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Configuration/BallSpecification.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Modeling/RobotPose.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"

CARD(StandbyCard,
{,
  CALLS(Activity),
  CALLS(LookActive),
  CALLS(LookAtPoint),
  CALLS(LookForward),
  CALLS(Stand),
  REQUIRES(BallSpecification),
  REQUIRES(FieldDimensions),
  REQUIRES(GameInfo),
  REQUIRES(OwnTeamInfo),
  REQUIRES(RobotPose),
  REQUIRES(TeamBehaviorStatus),
});

class StandbyCard : public StandbyCardBase
{
  bool preconditions() const override
  {
    return theGameInfo.state == STATE_STANDBY;
  }

  bool postconditions() const override
  {
    return theGameInfo.state != STATE_STANDBY;
  }

  void execute() override
  {
    theActivitySkill(BehaviorStatus::set);
    theStandSkill(/* high: */ true);
    theLookForwardSkill();

  }
};

MAKE_CARD(StandbyCard);
