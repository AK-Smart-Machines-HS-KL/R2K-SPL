#pragma once

#include "Tools/Streams/AutoStreamable.h"
#include <vector>

/** The definintion of an NTP message we send - in response to a previously received request. */
STREAMABLE(BNTPMessage,
{,
  (uint32_t) requestOrigination,  /**<                        The timestamp of the generation of the request. */
  (uint32_t) requestReceipt,      /**< [delta 0..-4095]       The timestamp of the receipt of the request. */
  (uint8_t) receiver,             /**< [#_MAX_NUM_OF_PLAYERS] The robot to which this message should be sent. */
});