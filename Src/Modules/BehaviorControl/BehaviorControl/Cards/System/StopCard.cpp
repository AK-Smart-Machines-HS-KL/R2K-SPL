/**
 * @file StopCard.cpp
 *
 * This file specifies the behavior for a robot in the Stop game state.
 *
 * @author Tim Simon
 */

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Communication/GameInfo.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"

CARD(StopCard,
{,
  CALLS(Activity),
  CALLS(LookActive),
  CALLS(Stand),
  REQUIRES(GameInfo),
  REQUIRES(RawGameInfo),
});

class StopCard : public StopCardBase
{
  bool preconditions() const override
  {
    return (theRawGameInfo.stopped == 1);
  }

  bool postconditions() const override
  {
    return (theRawGameInfo.stopped == 0);
  }

  void execute() override
  {
    theActivitySkill(BehaviorStatus::stop);
    theStandSkill(/* high: */ true);
    theLookActiveSkill();
  }
};

MAKE_CARD(StopCard);
