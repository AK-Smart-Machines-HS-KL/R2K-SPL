/**
 * @file TeamCommBufferManager.h
 * @author Andy Hobelsberger
 * 
 * This module manages copying data from the rest of the system into buffers to be used for sending team communication
 * Currenctly just for TeamCommBuffer, To be used in Cognition Thread
 */

#pragma once

#include "Tools/Module/Module.h"
#include "Representations/Communication/TeamCommStatus.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Modeling/BallModel.h"
#include "Representations/Communication/TeamCommBuffer.h"

MODULE(TeamCommBufferManager,
{,
  REQUIRES(RobotPose),
  REQUIRES(BallModel),
  PROVIDES(TeamCommBuffer),

  DEFINES_PARAMETERS(
  {,
  }),
});

class TeamCommBufferManager : public TeamCommBufferManagerBase
{
public:
  void update(TeamCommBuffer& buffer) override;
};
