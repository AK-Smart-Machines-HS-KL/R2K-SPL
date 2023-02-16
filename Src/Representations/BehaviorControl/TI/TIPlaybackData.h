/**
 * @file TIPlaybackData.h
 * @author Jonas Lambing (R2K)
 * @brief This file contains memory representations of worldmodel and their corresponding playback files.
 * @version 1.0
 * @date 2021-08-20
 * 
 */

#pragma once
#include <iostream>
#include <fstream>

#include "Tools/Math/Eigen.h"
#include "Tools/Math/Angle.h"
#include "Tools/Math/Pose2f.h"
#include "Tools/Streams/AutoStreamable.h"
#include "Tools/Streams/Enum.h"
#include "Platform/File.h"
#include "TIData.h"

// Both vectors most be valid for each item, otherwise both will be discarded (checkParity() in TIPlaybackProvider.cpp)
STREAMABLE(TIPlaybackSequences,
{,
  (bool) (false) loaded,
  (int) maxIdx,                       // Highest Index used by the system. used for saving new ones.
  (std::vector<PlaybackSequence>) data,   // all loaded playbacks
  (std::vector<WorldData>)   models,  // all loaded worldmodels
  (int) activePlayback,
});