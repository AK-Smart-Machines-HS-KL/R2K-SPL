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

STREAMABLE(AudioData,
{
  using Sample = float,

  (unsigned)(2) channels,
  (unsigned)(48000) sampleRate,
  (std::vector<Sample>) samples, /**< Samples are interleaved. This means For 3 Channels, A, B and C Audio Samples look like this: [A0,B0,C0,A1,B1,C1,A2,B2,C2,...] i.e. the samples are together in a single array */
});
