/**
 * @file TIRecorder.h
 * @author Andy Hobelsberger
 * @brief This Representation stores data for the Recording of TI Sequences, along with pointers to functions that control the recording process. 
 * @version 1.0
 * @date 2022-01-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once

#include "Representations/BehaviorControl/TIData.h"
#include "Tools/Streams/AutoStreamable.h"
#include "Tools/Function.h"
#include "Tools/RingBuffer.h"

STREAMABLE(TIRecorder, 
{
    void clear() {
        data.clear();
        worldData.clear();
    }

    FUNCTION(void()) start;
    FUNCTION(void()) stop;
    FUNCTION(void(playbackAction& action)) changeAction;
    FUNCTION(void()) save;
    ,

    (WorldData) worldData,
    (PlaybackData) data, 
    (bool) (false) recording, 
});