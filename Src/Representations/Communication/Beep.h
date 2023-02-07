/**
 * @file Beep.h
 *
 * Identified beep sound
 *
 * @author Wilhelm Simus
 */

#pragma once

#include "Tools/Streams/AutoStreamable.h"

STREAMABLE(Beep,
{,
    (std::vector<int>) messages,
});
