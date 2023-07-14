/**
 * @file BehaviorControl.h
 *
 * This file declares the new BehaviorControl in Season 2019.
 *
 * @author Jesse Richter-Klug
 */

#pragma once

#include "Representations/BehaviorControl/ActivationGraph.h"
#include "Representations/BehaviorControl/BehaviorStatus.h"
#include "Representations/BehaviorControl/Libraries/LibCheck.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Configuration/CalibrationRequest.h"
#include "Representations/Infrastructure/CameraStatus.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Infrastructure/RobotHealth.h"
#include "Representations/Infrastructure/SensorData/KeyStates.h"
#include "Representations/Modeling/BallModel.h"
#include "Representations/MotionControl/ArmMotionRequest.h"
#include "Representations/MotionControl/HeadMotionRequest.h"
#include "Representations/MotionControl/MotionRequest.h"
#include "Representations/MotionControl/OdometryData.h"
#include "Representations/Infrastructure/SensorData/JointSensorData.h"
#include "Tools/Module/Module.h"
#include "Tools/Streams/Enum.h"
#include <string>
#include "Tools/Communication/MessageComponents/BehaviorStatus.h"

MODULE(BehaviorControl,
{,
  REQUIRES(CameraStatus),
  REQUIRES(EnhancedKeyStates),
  REQUIRES(RobotHealth),
  REQUIRES(RobotInfo),

  REQUIRES(BallModel),
  REQUIRES(FrameInfo),
  REQUIRES(JointSensorData),
  REQUIRES(LibCheck),
  REQUIRES(OdometryData),

  PROVIDES(ActivationGraph),
  REQUIRES(ActivationGraph),

  PROVIDES(ArmMotionRequest),
  PROVIDES(BehaviorStatus),
  PROVIDES(CalibrationRequest),
  PROVIDES(HeadMotionRequest),
  PROVIDES(MotionRequest),

  LOADS_PARAMETERS(
  {,
    (std::string) rootCard, /**< The card that is executed directly by this module. */
  }),
});

// Include here so macros do not dismantle themself
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Skill/Skill.h"

class BehaviorControl : public BehaviorControlBase
{
public:
  /** Constructor. */
  BehaviorControl();

  void compileBehaviorControl(BehaviorStatusComponent*);
  BehaviorStatusComponent::Compiler messageCompilerRef = BehaviorStatusComponent::onCompile.add(std::bind(&BehaviorControl::compileBehaviorControl, this, _1));

  /**
   * Creates extended module info (union of this module's info and requirements of all behavior parts (cards and skills)).
   * @return The extended module info.
   */
  static std::vector<ModuleBase::Info> getExtModuleInfo();

private:
  /**
   * Updates the activation graph.
   * @param activationGraph The provided activation graph.
   */
  void update(ActivationGraph& activationGraph) override;

  /** Executes the top-level state machine of the behavior. */
  void execute();

  /**
   * Updates the arm motion request.
   * @param armMotionRequest The provided arm motion request.
   */
  void update(ArmMotionRequest& armMotionRequest) override { armMotionRequest = theArmMotionRequest; }

  /**
   * Updates the behavior status.
   * @param behaviorStatus The provided behavior status.
   */
  void update(BehaviorStatus& behaviorStatus) override { behaviorStatus = theBehaviorStatus; }

  /**
   * Updates the camera calibration request.
   * @param CalibrationRequest The provided camera calibration request.
   */
  void update(CalibrationRequest& CalibrationRequest) override { CalibrationRequest = theCalibrationRequest; }

  /**
   * Updates the head motion request.
   * @param headMotionRequest The provided head motion request.
   */
  void update(HeadMotionRequest& headMotionRequest) override { headMotionRequest = theHeadMotionRequest; }

  /**
   * Updates the motion request.
   * @param motionRequest The provided motion request.
   */
  void update(MotionRequest& motionRequest) override { motionRequest = theMotionRequest; }

  ENUM(State,
  {,
    inactive,
    sittingDown,
    gettingUp,
    cameraStatusFAILED,
    lowBattery,
    penalized,
    playing,
  });

  State status = inactive;

  ArmMotionRequest theArmMotionRequest; /**< The arm motion request that is modified by the behavior. */
  BehaviorStatus theBehaviorStatus; /**< The behavior status that is modified by the behavior. */
  CalibrationRequest theCalibrationRequest; /**< The camera calibration request that is modified by the behavior. */
  HeadMotionRequest theHeadMotionRequest; /**< The head motion request that is modified by the behavior. */
  MotionRequest theMotionRequest; /**< The motion request that is modified by the behavior. */

  SkillRegistry theSkillRegistry; /**< The manager of all skills. */
  CardRegistry theCardRegistry; /**< The manager of all cards. */
};
