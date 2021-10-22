/**
 * @file GyroOffset.h
 * @author Philip Reichenberg
 */

#pragma once

#include "Tools/Streams/AutoStreamable.h"
#include "Tools/Math/Eigen.h"
#include "Tools/Math/Angle.h"

STREAMABLE(GyroOffset,
{,
  (Vector3a)(Vector3a::Zero()) offset, /**< Current offsets of the gyros. */
  (bool)(false) bodyDisconnect, /**< Head lost connection to the body */
});
