/**
 * @file BHumanArbitraryMessage.cpp
 *
 * @author <A href="mailto:jesse@tzi.de">Jesse Richter-Klug</A>
 */

#include "BHumanArbitraryMessage.h"
#include "Platform/BHAssert.h"

int BHumanArbitraryMessage::sizeOfArbitraryMessage() const
{
  static_assert(BHUMAN_ARBITRARY_MESSAGE_STRUCT_VERSION == 0, "This method is not adjusted for the current message version");
  return static_cast<int>(queue.getStreamedSize());
}

void BHumanArbitraryMessage::write(void* data)
{
  OutBinaryMemory memory(queue.getStreamedSize(), reinterpret_cast<char*>(data));
  memory << queue;
}

bool BHumanArbitraryMessage::read(const void* data)
{
  queue.clear();

  InBinaryMemory memory(data);
  memory >> queue;

  return true;
}

void BHumanArbitraryMessage::read(In& stream)
{
  static_assert(BHUMAN_ARBITRARY_MESSAGE_STRUCT_VERSION == 0, "This method is not adjusted for the current message version");
  //TODO stream messagequeue
}

void BHumanArbitraryMessage::write(Out& stream) const
{
  static_assert(BHUMAN_ARBITRARY_MESSAGE_STRUCT_VERSION == 0, "This method is not adjusted for the current message version");
  //TODO stream messagequeue
}

void BHumanArbitraryMessage::reg()
{
  PUBLISH(reg);
  REG_CLASS(BHumanArbitraryMessage);
}
