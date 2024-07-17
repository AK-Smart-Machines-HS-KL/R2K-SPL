#pragma once

#include "Tools/Streams/AutoStreamable.h"
#include <limits>

/**
 * @struct SACCommands
 * Representation made for SAC
 */
STREAMABLE(SACCommands,
{,
  (int)(1) mode,         /**< If 1: robot is human-controlled */
  (int)(0) cardIdx,      /**< The card to be played selected by the human operator*/
  (int)(0) direction,     /** The direction selected by the operator */
});