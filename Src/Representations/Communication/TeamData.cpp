/**
 * @file TeamData.cpp
 */

#include "TeamData.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Tools/Module/Blackboard.h"
#include "Tools/Debugging/DebugDrawings3D.h"
#include "Platform/Time.h"
#include <algorithm>


Vector2f Teammate::getEstimatedPosition(unsigned time) const
{
  const float timeSinceLastPacket = std::max(0, static_cast<int>(time) - static_cast<int>(timeWhenLastPacketReceived)) / 1000.f;
  const float distanceToTarget = theBehaviorStatus.walkingTo.norm(); // Do not overshoot the target.
  const float correctionFactor = 0.5f;
  const Vector2f walkDirection = ((theRobotPose * theBehaviorStatus.walkingTo) - theRobotPose.translation).normalized();
  return theRobotPose.translation + std::min(distanceToTarget, timeSinceLastPacket * correctionFactor * theBehaviorStatus.speed) * walkDirection;
};

void TeamData::draw() const
{
  DECLARE_DEBUG_DRAWING("representation:TeamData", "drawingOnField");
  DECLARE_DEBUG_DRAWING3D("representation:TeamData:3d", "field");
  for(auto const& teammate : teammates)
  {
    ColorRGBA posCol;
    if(teammate.status == Teammate::PLAYING)
      posCol = ColorRGBA::green;
    else if(teammate.status == Teammate::FALLEN)
      posCol = ColorRGBA::yellow;
    else
      posCol = ColorRGBA::red;

    const Vector2f& rPos = teammate.theRobotPose.translation;
    const float radius = std::max(50.f, teammate.theRobotPose.getTranslationalStandardDeviation());
    Vector2f dirPos = teammate.theRobotPose * Vector2f(radius, 0.f);

    // Current estimated position
    if(Blackboard::getInstance().exists("FrameInfo"))
    {
      const FrameInfo& theFrameInfo = static_cast<const FrameInfo&>(Blackboard::getInstance()["FrameInfo"]);
      const Vector2f estimatedPosition = teammate.getEstimatedPosition(theFrameInfo.time);
      CIRCLE("representation:TeamData", estimatedPosition.x(), estimatedPosition.y(), 100, 30, Drawings::solidPen, ColorRGBA::yellow, Drawings::solidBrush, ColorRGBA::red);
      SPHERE3D("representation:TeamData:3d", estimatedPosition.x(), estimatedPosition.y(), 35, 35, ColorRGBA::yellow);
    }

    // Circle around Player
    CIRCLE("representation:TeamData", rPos.x(), rPos.y(), radius, 20, Drawings::solidPen,
           posCol, Drawings::noBrush, ColorRGBA::white);
    // Direction of the Robot
    LINE("representation:TeamData", rPos.x(), rPos.y(), dirPos.x(), dirPos.y(), 20,
         Drawings::solidPen, posCol);
    // Player number
    DRAW_TEXT("representation:TeamData", rPos.x() + 100, rPos.y(), 100, ColorRGBA::black, teammate.number);

    // Line from Robot to WalkTarget
    const Vector2f target = teammate.theRobotPose * teammate.theBehaviorStatus.walkingTo;
    LINE("representation:TeamData", rPos.x(), rPos.y(), target.x(), target.y(),
         10, Drawings::dashedPen, ColorRGBA::magenta);

    // Ball position
    const Vector2f ballPos = teammate.theRobotPose * teammate.theBallModel.estimate.position;
    CIRCLE("representation:TeamData", ballPos.x(), ballPos.y(), 50, 20, Drawings::solidPen, ColorRGBA::yellow, Drawings::solidBrush, ColorRGBA::yellow);
  }
}

TeamData::TeamData() {
  RobotPoseComponent::priority = 1;
  BehaviorStatusComponent::priority = 1;
  BallModelComponent::priority = 1;
}

Teammate& TeamData::getBMate(int number)
{
  for(auto& teammate : teammates) {
    if(teammate.number == number) {
      return teammate;
    }
  }
  teammates.emplace_back();
  teammates.back().number = number;
  return teammates.back();
}

void TeamData::rcvMessage(RobotMessageHeader & header) {
  auto& bmate = getBMate(header.senderID);

  bmate.timeWhenLastPacketReceived = Time::getCurrentSystemTime();
}

void TeamData::rcvRobotPose(RobotPoseComponent * comp, RobotMessageHeader & header) {
  auto& bmate = getBMate(header.senderID);
  bmate.theRobotPose = comp->pose;
}

void TeamData::rcvBehaviorStatus(BehaviorStatusComponent * comp, RobotMessageHeader & header) {
  auto& bmate = getBMate(header.senderID);
  bmate.theBehaviorStatus.activity = static_cast<BehaviorStatus::Activity>(comp->activity);
}


void TeamData::rcvBallModel(BallModelComponent* comp, RobotMessageHeader& header) {
  auto& bmate = getBMate(header.senderID);
  bmate.theBallModel = comp->model;
}

