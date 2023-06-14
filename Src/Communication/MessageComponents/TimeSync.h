#pragma once
#include "Tools/Communication/RobotMessage.h"
#include "Tools/Communication/BHumanTeamMessageParts/BNTPMessage.h"

class BNTPComponent: public RobotMessageComponent<BNTPComponent> {
  public:

  

  //Required
  inline static const std::string name = "BNTP";
  BNTPComponent() : RobotMessageComponent<BNTPComponent>() { }

  size_t compress(char* buff);

  bool decompress(char* compressed);

  size_t getSize();
};