/**
 * @file AudioData.h
 * The file declares a struct that stores audio data of up to four channels.
 * 0: back left microphone
 * 1: back right microphone
 * 2: front left microphone
 * 3: front right microphone
 * @author Thomas Röfer
 */

#pragma once

#include "Tools/Streams/AutoStreamable.h"

STREAMABLE(R2KAudioData,
{,
    (int) dummyData,
});
