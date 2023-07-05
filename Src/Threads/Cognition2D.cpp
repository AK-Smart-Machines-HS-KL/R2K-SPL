/**
 * @file Threads/Cognition2D.cpp
 *
 * This file implements the execution unit for the cognition thread.
 *
 * @author Jan Fiedler
 * @author Arne Hasselbring
 */

#include "Cognition2D.h"
#include "Modules/Infrastructure/LogDataProvider/LogDataProvider.h"
#include "Tools/Debugging/Debugging.h"
#include "Tools/Settings.h"

REGISTER_EXECUTION_UNIT(Cognition2D)

Cognition2D::Cognition2D()
: robotMessageHandler()
{
#ifndef TARGET_ROBOT
  robotMessageHandler.startLocal(Global::getSettings().teamPort, static_cast<unsigned>(Global::getSettings().playerNumber));
#else
  std::string bcastAddr = UdpComm::getWifiBroadcastAddress();
  robotMessageHandler.start(Global::getSettings().teamPort, bcastAddr.c_str());
#endif
}

bool Cognition2D::beforeFrame()
{
  // read from team comm udp socket
  robotMessageHandler.receive();

  return LogDataProvider::isFrameDataComplete();
}

void Cognition2D::beforeModules()
{
  BH_TRACE_MSG("before TeamData");
}

void Cognition2D::afterModules()
{
  if(Blackboard::getInstance().exists("BHumanMessageOutputGenerator")
     && static_cast<const BHumanMessageOutputGenerator&>(Blackboard::getInstance()["BHumanMessageOutputGenerator"]).sendThisFrame)
  {
    BH_TRACE_MSG("before theSPLMessageHandler.send()");
    robotMessageHandler.send();
  }
}
