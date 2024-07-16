#pragma once

#include "Tools/Streams/AutoStreamable.h"
#include <limits>

/**
 * @struct SACCommands
 * Representation of the Time to reach the ball
 */
STREAMABLE(SACCommands,
{,
  (int)(1) mode,         /**< If 1: robot is human-controlled */
  (int)(1) cardIdx,      /**< The card to be played selected by the human operator*/
});