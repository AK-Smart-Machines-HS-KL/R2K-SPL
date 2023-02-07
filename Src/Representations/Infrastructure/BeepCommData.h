/**
 * @file BeepCommData.h
 * @author Nicolas Fortune
 * @brief 
 * @version 0.1
 * @date 2023-01-13
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include "Tools/Streams/AutoStreamable.h"

STREAMABLE(BeepCommData,
{,
    (std::vector<int>) broadcastQueue,
});
