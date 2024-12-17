
/**
 * @file TeamData.h
 *
 * @author <A href="mailto:jesse@tzi.de">Jesse Richter-Klug</A>
 * @author <a href="mailto:tlaue@uni-bremen.de">Tim Laue</a>
 */

#pragma once

#include "Representations/BehaviorControl/BehaviorStatus.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Infrastructure/RobotHealth.h"
#include "Representations/Modeling/FieldCoverage.h"
#include "Representations/Modeling/ObstacleModel.h"
#include "Representations/Modeling/BallModel.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Modeling/Whistle.h"

#include "Tools/Communication/SPLStandardMessageBuffer.h"
#include "Tools/MessageQueue/InMessage.h"
#include "Tools/Streams/AutoStreamable.h"
#include "Tools/Function.h"
#include "Tools/Streams/Enum.h"

#include "Tools/Communication/BNTP.h"
#include "Tools/Communication/MessageComponents/RobotPose.h"
#include "Tools/Communication/MessageComponents/BallModel.h"
#include "Tools/Communication/MessageComponents/BehaviorStatus.h"

STREAMABLE(Teammate,
{
  const SynchronizationMeasurementsBuffer* bSMB = nullptr;

  unsigned toLocalTimestamp(unsigned remoteTimestamp) const
  {
    if(bSMB)
      return bSMB->getRemoteTimeInLocalTime(remoteTimestamp);
    else
      return 0u;
  };

  Vector2f getEstimatedPosition(unsigned time) const;

  ENUM(Status,
  {,
    PENALIZED,                        /** OK   : I receive packets, but robot is penalized */
    FALLEN,                           /** GOOD : Robot is playing but has fallen or currently no ground contact */
    PLAYING,                          /** BEST : Teammate is standing/walking and has ground contact :-) */
  });

  FieldCoverage theFieldCoverage; /**< Do not log this huge representation! */
  
  ,
  (int)(-1) number,
  (bool)(false) isGoalkeeper,             /**< This is for a teammate what \c theRobotInfo.isGoalkeeper() is for the player itself. */
  (bool)(false) isPenalized,              // Derived in TeamMessageHandler
  (bool)(true) isUpright,                 // NOT SYNCED 

  (unsigned)(0) timeWhenLastPacketSent,
  (unsigned)(0) timeWhenLastPacketReceived,
  (Status)(PENALIZED) status,             // NOT SYNCED - Partially derived in TeamMessageHandler
  (unsigned)(0) timeWhenStatusChanged,    // Derived in TeamMessageHandler

  (RobotPose) theRobotPose,               // SYNCED
  (BallModel) theBallModel,               // NOT SYNCED
  (ObstacleModel) theObstacleModel,       // NOT SYNCED
  (BehaviorStatus) theBehaviorStatus,     // SYNCED
});

/**
 * @struct TeammateData
 * Collection of teammate information
 */
STREAMABLE(TeamData,
{
  TeamData();

  void draw() const;

  Teammate& getBMate(int number);

  void rcvRobotPose(RobotPoseComponent *, RobotMessageHeader &);
  RobotPoseComponent::Callback robotPoseCallbackRef = RobotPoseComponent::onRecieve.add(std::bind(&TeamData::rcvRobotPose, this, _1, _2));

  void rcvBallModel(BallModelComponent *, RobotMessageHeader&);
  BallModelComponent::Callback ballModelCallbackRef = BallModelComponent::onRecieve.add(std::bind(&TeamData::rcvBallModel, this, _1, _2));

  void rcvBehaviorStatus(BehaviorStatusComponent *, RobotMessageHeader &);
  BehaviorStatusComponent::Callback behaviorStatusCallbackRef = BehaviorStatusComponent::onRecieve.add(std::bind(&TeamData::rcvBehaviorStatus, this, _1, _2));

  void rcvMessage(RobotMessageHeader &);
  CallbackRegistry<std::function<void(RobotMessageHeader&)>>::Callback msgRecieveCallbackRef = RobotMessage::onRecieve.add(std::bind(&TeamData::rcvMessage, this, _1));
  ,

  (std::vector<Teammate>) teammates, /**< An unordered(!) list of all teammates that are currently communicating with me */
  (int)(0) numberOfActiveTeammates,  /**< The number of teammates (in the list) that are at not PENALIZED */
  (unsigned)(0) receivedMessages,    /**< The number of received (not self) team messages */
});
