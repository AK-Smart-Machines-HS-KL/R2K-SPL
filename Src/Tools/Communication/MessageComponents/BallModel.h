#pragma once
#include "Tools/Communication/RobotMessage.h"
#include "Representations/Modeling/BallModel.h"
// #include "Proto/FieldBall_bp.h"

class BallModelComponent : public RobotMessageComponent<BallModelComponent> {
  public:

  BallModel model;

  //Required
  inline static const std::string name = "BallModel";
  BallModelComponent() : RobotMessageComponent<BallModelComponent>() { }

  size_t compress(uint8_t* buff);

  bool decompress(uint8_t* compressed);

  size_t getSize();
};