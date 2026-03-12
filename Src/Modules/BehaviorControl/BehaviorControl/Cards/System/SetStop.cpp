/**
 * @file SetStop.cpp
 *
 * This file specifies the behavior for a robot in the Stop game state.
 *
 * @author Tim Simon, 
 */

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Configuration/BallSpecification.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Modeling/RobotPose.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"

CARD(StopCard,
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

class StopCard : public StopCardBase
{
  bool preconditions() const override
  {
    return theGameInfo.stopped != 0;
  }

  bool postconditions() const override
  {
    return theGameInfo.stopped == 0;
  }

  void execute() override
  {
    theActivitySkill(BehaviorStatus::set);
    theStandSkill(/* high: */ true);
    theLookActiveSkill();
    
  }
};

MAKE_CARD(StopCard);
