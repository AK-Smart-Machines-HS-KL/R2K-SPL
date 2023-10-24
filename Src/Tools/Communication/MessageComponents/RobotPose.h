#pragma once
#include "Tools/Communication/RobotMessage.h"
#include "Tools/Math/Pose2f.h"

class RobotPoseComponent : public RobotMessageComponent<RobotPoseComponent> {
  public:

  Pose2f pose;

  //Required
  inline static const std::string name = "RobotPose";
  RobotPoseComponent() : RobotMessageComponent<RobotPoseComponent>() { }

  size_t compress(uint8_t* buff);

  bool decompress(uint8_t* compressed);

  size_t getSize();
};