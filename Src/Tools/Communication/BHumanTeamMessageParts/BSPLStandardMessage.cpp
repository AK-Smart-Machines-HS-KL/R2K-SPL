/**
 * @file BSPLStandardMessage.cpp
 *
 * @author <A href="mailto:jesse@tzi.de">Jesse Richter-Klug</A>
 */
#include "BSPLStandardMessage.h"
#include "Platform/BHAssert.h"
#include "Tools/FunctionList.h"
#include <cstddef>

size_t BSPLStandardMessage::sizeOfBSPLMessage() const
{
  return offsetof(RoboCup::SPLStandardMessage, data);
}

void BSPLStandardMessage::write(void* data) const
{
  memcpy(data, &playerNum, sizeOfBSPLMessage());
}

bool BSPLStandardMessage::read(const void* data)
{
  memcpy(reinterpret_cast<void*>(&playerNum), data, sizeOfBSPLMessage());

  return true;
}

void BSPLStandardMessage::read(In& stream)
{
  STREAM(playerNum);
  STREAM(teamNum);
  STREAM(pose);
  STREAM(ball);
  STREAM(numOfDataBytes);
}

void BSPLStandardMessage::write(Out& stream) const
{
  STREAM(playerNum);
  STREAM(teamNum);
  STREAM(pose);
  STREAM(ball);
  STREAM(numOfDataBytes);
}

void BSPLStandardMessage::reg()
{
  PUBLISH(reg);
  REG_CLASS(BSPLStandardMessage);
  REG(playerNum);
  REG(teamNum);
  REG(pose);
  REG(ball);
  REG(numOfDataBytes);
}
