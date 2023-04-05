/**
 * @file TIRecorder.h
 * @author Andy Hobelsberger, Wilhelm Simus
 * @brief This Representation stores data for the Recording of TI Sequences, along with pointers to functions that control the recording process. 
 * @version 1.0
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

STREAMABLE(TIRecorder, 
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