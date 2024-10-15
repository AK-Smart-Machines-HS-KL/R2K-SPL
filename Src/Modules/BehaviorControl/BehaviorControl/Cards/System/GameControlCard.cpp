/**
 * @file GameControlCard.cpp
 *
 * This file implements a card that handles the top level robot control in normal games.
 *
 * @author Arne Hasselbring
 */

#include "Representations/BehaviorControl/SACCommands.h"
#include "Representations/Communication/RobotInfo.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/Dealer.h"

CARD(GameControlCard,
{,
  REQUIRES(SACCommands),
  REQUIRES(RobotInfo),
  LOADS_PARAMETERS(
  {,
    (DeckOfCards<CardRegistry>) deck, /**< The deck from which a card is played. */
    (DeckOfCards<CardRegistry>) sacDeck, /**< The deck from which a card is played if the robot is human-controlled. */

  }),
});

class GameControlCard : public GameControlCardBase
{
  bool preconditions() const override
  {
    return theRobotInfo.penalty == PENALTY_NONE;
  }

  bool postconditions() const override
  {
    return theRobotInfo.penalty != PENALTY_NONE;
  }

  void execute() override
  {
    if(theSACCommands.mode == 1)
    {
      dealer.deal_by_idx(sacDeck, theSACCommands.cardIdx)->call();
    } else {
      dealer.deal(deck)->call();
    }
  }

  void reset() override
  {
    dealer.reset();
  }

  PriorityListDealer dealer; /**< The dealer which selects the card to play. */
};

MAKE_CARD(GameControlCard);
