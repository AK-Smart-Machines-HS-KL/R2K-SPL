#pragma once
#include "Tools/Communication/RobotMessage.h"
#include "Tools/Communication/BHumanTeamMessageParts/BNTPMessage.h"

class BNTPResponseComponent: public RobotMessageComponent<BNTPResponseComponent> {
  public:
  std::list<BNTPMessage> messages;

  //Required
  inline static const std::string name = "BNTPResponse";
  BNTPResponseComponent() : RobotMessageComponent<BNTPResponseComponent>() { }

  size_t compress(uint8_t* buff);

  bool decompress(uint8_t* compressed);

  size_t getSize();
};