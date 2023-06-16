#pragma once
#include "Tools/Communication/RobotMessage.h"
#include "Tools/Communication/BHumanTeamMessageParts/BNTPMessage.h"

class BNTPRequestComponent: public RobotMessageComponent<BNTPRequestComponent> {
  public:

  //Required
  inline static const std::string name = "BNTPRequest";
  BNTPRequestComponent() : RobotMessageComponent<BNTPRequestComponent>() { }

  size_t compress(char* buff);

  bool decompress(char* compressed);

  size_t getSize();
};