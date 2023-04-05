/**
 * @file TIRecorderData.h
 * @author Andy Hobelsberger
 * @brief This Representation stores data for the Recording of TI Sequences, along with pointers to functions that control the recording process. 
 * @version 1.0
 * @date 2022-01-05
 * 
 * @copyright Copyright (c) 2022
 * 
 * @author Wilhelm Simus
 * @version 2.0
 * @date 2023-04-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once

#include "Representations/BehaviorControl/TI/TIData.h"
#include "Tools/Streams/AutoStreamable.h"
#include "Tools/Function.h"
#include "Tools/RingBuffer.h"

STREAMABLE(TIRecorderData, 
{
    void clear() {
        sequence.clear();
        worldData.clear();
    }

    FUNCTION(void()) start;
    FUNCTION(void()) stop;
    FUNCTION(void(PlaybackAction& pAction)) changeAction;
    FUNCTION(void()) save;
    ,

    (WorldData) worldData,
    (PlaybackSequence) sequence, 
    (bool) (false) recording, 
});