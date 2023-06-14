#pragma once
#include "Tools/Communication/RobotMessage.h"

class BallModelComponent: public RobotMessageComponent<BallModelComponent> {
  public:
  //Required
  inline static const std::string name = "BallModel";
  BallModelComponent() : RobotMessageComponent<BallModelComponent>() { }

  size_t compress(char* buff);

  bool decompress(char* compressed);

  size_t getSize();
};