/**
 * @file FieldBall.h
 *
 * Declaration of a representation that contains additional information
 * about the ball that is required by the behavior.
 *
 * @author Tim Laue
 */

#pragma once

#include "Tools/Streams/AutoStreamable.h"
#include "Representations/Configuration/KickInfo.h"

STREAMABLE(KickTypeData,
  {,
    (KickInfo::KickType) name, // name of the kick taken from KickInfo.cpp
    (Vector2f) offset, // The offset from the robot's center that the ball should be
    (float) duration,    // Time it takes to kick the ball after positioning
    (Angle) angleAccSD, // Standard Deviation of the angular accuracy of this kick
    (float) distAccSD, // Standard Deviation of the distance accuracy of this kick
    (float) range,     // Mean Range of the Kick at max power
  });

STREAMABLE(Shot,
{,
  (Vector2f) target, 
  (KickTypeData) kickType,
  (float) power,

  (float) executionTime,
  (float) failureProbability,
});

/**
 * @struct Shots
 * A representation that contains information about possible shot angles and reachable targets
 */
STREAMABLE(Shots,
{
  /** Debug drawings */
  void draw() const;
  ,
  (Shot) goalShot, 
});


