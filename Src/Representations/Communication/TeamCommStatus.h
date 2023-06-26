/**
 * @file TeamCommStatus.h
 * @author Andy Hobelsberger
 * @brief 
 * @version 1
 * @date 2023-05-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include "Tools/Streams/AutoStreamable.h"

STREAMABLE(TeamCommStatus,
{,
  (bool)(true) isWifiCommActive,      /** If the teamm communication is active*/
});