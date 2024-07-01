/**
 * @file TeamCommBuffer.h
 * @author Andy Hobelsberger
 * @brief 
 * @version 1
 * @date 2023-11-01
 * 
 * @copyright Copyright (c) 2023
 * 
 * Buffer for team Comm Data. With this, data can be moved to the Cognition thread and Callbacks registered properly. 
 * Should be written to by TeamCommBufferManager when possible
 */

#pragma once

#include "Tools/Streams/AutoStreamable.h"
#include "Tools/Math/Pose2f.h"
#include "Tools/Communication/MessageComponents/RobotPose.h"
#include "Tools/Communication/MessageComponents/BallModel.h"

STREAMABLE(TeamCommBuffer,
{


  // Create and Register RobotPose Compiler
  void compileRobotPose(RobotPoseComponent *);
  RobotPoseComponent::Compiler robotPoseCompilerRef = RobotPoseComponent::onCompile.add(std::bind(&TeamCommBuffer::compileRobotPose, this, _1));

  // Create and Register BallModel Compiler
  void compileBallModel(BallModelComponent*);
  BallModelComponent::Compiler ballModelCompilerRef = BallModelComponent::onCompile.add(std::bind(&TeamCommBuffer::compileBallModel, this, _1));
  ,

  (Pose2f) pose, // Buffer for robot pose, written to by TeamCommBufferManager, read by compile callback
  (BallModel)model, // Buffer for ball model, written to by TeamCommBufferManager, read by compile callback
});

// Impl for Compile Callback
void TeamCommBuffer::compileRobotPose(RobotPoseComponent * comp) {
    comp->pose = pose;
}

// Impl for Compile Callback
void TeamCommBuffer::compileBallModel(BallModelComponent* comp) {
  comp->model = model;
}