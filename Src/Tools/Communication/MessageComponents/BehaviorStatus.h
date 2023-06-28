#pragma once
#include "Tools/Communication/RobotMessage.h"

class BehaviorStatusComponent : public RobotMessageComponent<BehaviorStatusComponent> {
  public:
  uint8_t activity;

  //Required
  inline static const std::string name = "BehaviorStatus";
  BehaviorStatusComponent() : RobotMessageComponent<BehaviorStatusComponent>() {}

  size_t compress(uint8_t* buff);

  bool decompress(uint8_t* compressed);

  size_t getSize();
};