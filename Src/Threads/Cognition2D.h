/**
 * @file Threads/Cognition2D.h
 *
 * This file declares the execution unit for the cognition thread.
 *
 * @author Jan Fiedler
 * @author Arne Hasselbring
 */

#pragma once

#include "Tools/Communication/RobotMessageHandler.h" // include this first to prevent WinSock2.h/Windows.h conflicts
#include "Tools/Framework/FrameExecutionUnit.h"

/**
 * @class Cognition
 *
 * The execution unit for the cognition thread.
 */
class Cognition2D : public FrameExecutionUnit
{
private:
  RobotMessageHandler robotMessageHandler;

public:

  Cognition2D();
  bool beforeFrame() override;
  void beforeModules() override;
  void afterModules() override;
};
