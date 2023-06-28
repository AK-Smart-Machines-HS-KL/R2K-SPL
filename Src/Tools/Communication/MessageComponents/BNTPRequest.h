#pragma once
#include "Tools/Communication/RobotMessage.h"
#include "Tools/Communication/BHumanTeamMessageParts/BNTPMessage.h"

class BNTPRequestComponent: public RobotMessageComponent<BNTPRequestComponent> {
  public:

  //Required
  inline static const std::string name = "BNTPRequest";
  BNTPRequestComponent() : RobotMessageComponent<BNTPRequestComponent>() { }

  size_t compress(uint8_t* buff);

  bool decompress(uint8_t* compressed);

  size_t getSize();
};