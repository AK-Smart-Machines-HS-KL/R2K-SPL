/**
 * @file Skills.h
 *
 * This file declares all skills that are used by the current behavior.
 *
 * @author Probably many people who will not add themselves to this declaration. <- Yes (Check .cpp files)
 * 
 * Annotations: 
 * XX SKILL -     Labelled Skills are usable in a Card / other Skills
 * ACTION SKILL - A Skill that does something like move the body or the head
 * DEBUG SKILL -  Usable for printing information / or relaying information to the user
 * COM SKILL -    Sets Information for communication with other robots (i.e. Behavior status etc.)
 */

#pragma once

#include "Modules/BehaviorControl/BehaviorControl/Skills/Head/HeadOrientation.h"
#include "Representations/BehaviorControl/BehaviorStatus.h"
#include "Representations/Configuration/CalibrationRequest.h"
#include "Representations/Configuration/KickInfo.h"
#include "Representations/MotionControl/ArmKeyFrameRequest.h"
#include "Representations/MotionControl/HeadMotionRequest.h"
#include "Representations/MotionControl/KeyframeMotionRequest.h"
#include "Representations/MotionControl/MotionRequest.h"
#include "Tools/BehaviorControl/Framework/Skill/Skill.h"
#include "Tools/BehaviorControl/Interception.h"
#include "Tools/Math/Angle.h"
#include "Tools/Math/Eigen.h"
#include "Tools/Range.h"
#include "Tools/Math/Pose2f.h"
#include "Tools/RobotParts/Arms.h"
#include <limits>
#include <string>

namespace Skills
{
  /** This skill turns off the joints of the robot. */
  SKILL_INTERFACE(PlayDead);

  /**
   * ACTION SKILL
   * This skill makes the robot stand.
   * @param high Whether the knees should be stretched
   */
  SKILL_INTERFACE(Stand, (bool)(false) high);

  /**
   * ACTION SKILL
   * This skill walks with a specified speed.
   * @param speed The walking speed in radians/s for the rotation and mm/s for the translation
   */
  SKILL_INTERFACE(WalkAtAbsoluteSpeed, (const Pose2f&) speed);

  /**
   * ACTION SKILL
   * This skill walks with a specified speed relative to the configured maximum. 
   * Maximums set in Config/Robots/Default/walkingEngine.cfg 
   * @param speed The walking speed as ratio of the maximum speed in [0, 1]
   */
  SKILL_INTERFACE(WalkAtRelativeSpeed, (const Pose2f&) speed);

  /**
   * ACTION SKILL
   * This skill walks to a (relative) target.
   * Note: No Obstacle Avoidance, Use GoTo Skills for obstacle avoidance
   * @param target The target pose in robot-relative coordinates
   * @param speed The walking speed as ratio of the maximum speed in [0, 1]
   * @param obstacleAvoidance The obstacle avoidance request
   * @param keepTargetRotation Whether the target rotation should be headed for all the time (instead of allowing motion to plan it)
   */
  SKILL_INTERFACE(WalkToPose, (const Pose2f&) target, (const Pose2f&) speed, (const MotionRequest::ObstacleAvoidance&) obstacleAvoidance, (bool) keepTargetRotation);

  /**
   * This skill walks to the ball and kicks it.
   * Note: use GoToBallAndKick
   * @param targetDirection The (robot-relative) direction in which the ball should go
   * @param kickType The type of kick that should be used
   * @param alignPrecisely Whether the robot should align more precisely than usual
   * @param kickPower The amount of power (in [0, 1]) that the kick should use
   * @param speed The walking speed as ratio of the maximum speed in [0, 1]
   * @param obstacleAvoidance The obstacle avoidance request
   * @param preStepAllowed Is a prestep for the InWalkKick allowed?
   * @param turnKickAllowed Does the forward kick not need to align with the kick direction?
   * @param directionPrecision The allowed deviation of the direction in which the ball should go. If default, the WalkToBallAndKickEngine uses its own precision.
   */
  SKILL_INTERFACE(WalkToBallAndKick, (Angle) targetDirection, (KickInfo::KickType) kickType, (bool) alignPrecisely, (float) kickPower, (const Pose2f&) speed, (const MotionRequest::ObstacleAvoidance&) obstacleAvoidance, (bool)(true) preStepAllowed, (bool)(true) turnKickAllowed, (const Rangea&)(Rangea(0_deg, 0_deg)) directionPrecision);

  /**
   * This skill dribbles the ball.
   * Note: Use GoToBallAndDribble
   * @param targetDirection The (robot-relative) direction in which the ball should go
   * @param speed The walking speed as ratio of the maximum speed in [0, 1]
   * @param obstacleAvoidance The obstacle avoidance request
   * @param alignPrecisely Whether the robot should align more precisely than usual
   * @param kickPower The desired strength of the dribble kick
   * @param preStepAllowed Is a prestep for the InWalkKick allowed?
   * @param turnKickAllowed Does the forward kick not need to align with the kick direction?
   * @param directionPrecision The allowed deviation of the direction in which the ball should go. If default, the WalkToBallAndKickEngine uses its own precision.
   */
  SKILL_INTERFACE(Dribble, (Angle) targetDirection, (const Pose2f&) speed, (const MotionRequest::ObstacleAvoidance&) obstacleAvoidance, (bool)(false) alignPrecisely, (float)(1.f) kickPower, (bool)(true) preStepAllowed, (bool)(true) turnKickAllowed, (const Rangea&)(Rangea(0_deg, 0_deg)) directionPrecision);

  /** This skill executes the get up engine. */
  SKILL_INTERFACE(GetUpEngine);

  /**
   * This skill lets the robot execute a keyframe motion.
   * @param id The ID of the motion
   * @param mirror Whether the motion should be mirrored
   */
  SKILL_INTERFACE(KeyframeMotion, (KeyframeMotionRequest::KeyframeMotionID) id, (bool)(false) mirror);

  /**
   * This skill executes a key frame motion with both arms.
   * @param motion The motion that the arm should execute
   * @param fast Whether states should not be interpolated
   */
  SKILL_INTERFACE(KeyFrameArms, (ArmKeyFrameRequest::ArmKeyFrameId) motion, (bool)(false) fast);

  /**
   * This skill executes a key frame motion with a single arm.
   * @param motion The motion that the arm should execute
   * @param arm The arm that should execute the motion
   * @param fast Whether states should not be interpolated
   */
  SKILL_INTERFACE(KeyFrameSingleArm, (ArmKeyFrameRequest::ArmKeyFrameId) motion, (Arms::Arm) arm, (bool)(false) fast);

  /**
   * ACTION SKILL
   * This skill lets one arm point at some point.
   * @param localPoint The point in robot-relative coordinates
   */
  SKILL_INTERFACE(PointAt, (const Vector3f&) localPoint);

  /**
   * ACTION SKILL
   * This skill lets a specific arm point at some point.
   * @param localPoint The point in robot-relative coordinates
   * @param arm The arm that shall be used for pointing
   */
  SKILL_INTERFACE(PointAtWithArm, (const Vector3f&) localPoint, (Arms::Arm) arm);

  /** This skill reacts to arm contact by taking the arm away. */
  SKILL_INTERFACE(ArmContact);

  /**
   * This skill reacts to arm contact by taking the arm away (but only for one arm).
   * @param arm The arm for which the contact reaction shall be done
   */
  SKILL_INTERFACE(ArmContactSingleArm, (Arms::Arm) arm);

  /** This skill takes arms back to avoid obstacles. */
  SKILL_INTERFACE(ArmObstacleAvoidance);

  /**
   * This skill takes one arm back to avoid obstacles.
   * @param arm The arm for which the obstacle avoidance should be done
   */
  SKILL_INTERFACE(ArmObstacleAvoidanceSingleArm, (Arms::Arm) arm);

  /**
   * ACTION SKILL
   * This skill moves the head to look at interesting points
   * Note: No idea how this works, but it does well
   * @param withBall Whether the ball must be in the image
   * @param ignoreBall Whether the ball should be completely ignored
   * @param onlyOwnBall Whether to use only the own ball model and not the team ball model
   * @param fixTilt Whether to use a fix tilt or to allow the head control to calculate it
   */
  SKILL_INTERFACE(LookActive, (bool)(false) withBall, (bool)(false) ignoreBall, (bool)(false) onlyOwnBall, (bool)(false) fixTilt);

  /**
   * ACTION SKILL
   * This skill moves the head so that a camera looks at specified angles. 
   * Note: Is not dynamic. If you want the camera to be dynamic, you must checnge the input angles  
   * @param pan The target pan angle
   * @param tilt The target tilt angle
   * @param speed The speed with which to move the head, in radians/sec
   * @param camera The camera which should have the specified angles
   * @param calibrationMode Whether to set the mode to calibrationMode instead of panAndTiltMode, which disables clipping and interpolation of angles.
   */
  SKILL_INTERFACE(LookAtAngles, (Angle) pan, (Angle) tilt, (float)(180_deg) speed, (HeadMotionRequest::CameraControlMode)(HeadMotionRequest::autoCamera) camera, (bool)(false) calibrationMode);

  /**
   * ACTION SKILL
   * This skill moves the head so that the ball is focused by one camera.
   * @param mirrored Whether to look at the mirrored (about the center of the field) ball position
   * @param forceOwnEstimate Whether to use only the own ball model and not the team ball model
   */
  SKILL_INTERFACE(LookAtBall, (bool)(false) mirrored, (bool)(false) forceOwnEstimate);

  /**
   * ACTION SKILL
   * This skill moves the head so that the team ball is focused by one camera.
   * @param mirrored Whether to look at the mirrored (about the center of the field) ball position
   */
  SKILL_INTERFACE(LookAtGlobalBall, (bool)(false) mirrored);

  /**
   * ACTION SKILL
   * This skill moves the head such that a specified (robot-relative) point is focused by one camera.
   * @param target The point to look at in robot-relative coordinates
   * @param camera The camera which should look at the point
   * @param speed The speed with which to move the head, in radians/sec
   */
  SKILL_INTERFACE(LookAtPoint, (const Vector3f&) target, (HeadMotionRequest::CameraControlMode)(HeadMotionRequest::autoCamera) camera, (Angle)(180_deg) speed);

  /** 
   * ACTION SKILL
   * This skill moves the head so that it looks forward.  
   */
  SKILL_INTERFACE(LookForward);

  /**
   * ACTION SKILL
   * This skill moves the head alternately to the left and to the right
   * @param startLeft Whether to look left first (otherwise right)
   * @param maxPan The maximum pan angle
   * @param tilt The constant tilt angle during the motion
   * @param speed The speed with which to move the head
   */
  SKILL_INTERFACE(LookLeftAndRight, (bool)(true) startLeft, (Angle)(50_deg) maxPan, (Angle)(23_deg) tilt, (Angle)(100_deg) speed);

  /**
   * BROKEN SKILL?
   * HeadOrientation Class does not have a good public, non-constexpr constructor
   * This skill moves the head in steps between the maximum pan and tilt, relative to the current orientation.
   * Note: Pan = Horizontal | Tilt = vertical
   * @param original starting orientation 
   * @param maximum maximum deviation from starting orientation (Construct with HeadOrientation(pan, tilt)
   * @param panStep Pan step angle
   * @param tiltStep Tilt step angle
   * @param waitInPosition Time to wait in each position, in ms
   */
  SKILL_INTERFACE(PanAndTiltGrid, (const HeadOrientation&) original, (const HeadOrientation&) maximum, (const Angle&) panStep, (const Angle&) tiltStep, (int) waitInPosition, (Angle)(100_deg) speed);

  /**
   * COM SKILL
   * This skill sets the passTarget member of the BehaviorStatus.
   * Caution: If the passTarget is set to -1, the ballTarget Vector is often used to communicate good angles instead of a position
   * @param passTarget The passTarget to set
   * @param ballTarget Optionally, the position where the ball should end up in robot-relative coordinates
   */
  SKILL_INTERFACE(PassTarget, (int) passTarget, (const Vector2f&)(Vector2f::Zero()) ballTarget);

  /**
   * COM SKILL
   * DEBUG SKILL
   * This skill sets the activity member of the BehaviorStatus.
   * @param activity The activity to set
   */
  SKILL_INTERFACE(Activity, (BehaviorStatus::Activity) activity);

  /**
   * DEBUG SKILL
   * Note: Great for Debugging
   * This skill adds an annotation if it differs from the one that has been added in the last frame.
   * The Annotation is printed in the behavior stack section of SimRobot
   * @param annotation The annotation message
   */
  SKILL_INTERFACE(Annotation, (const std::string&) annotation);

  /**
   * DEBUG SKILL
   * This skill plays a sound file if it differs from the one that has been played in the last frame.
   * @param name The name of the sound file
   */
  SKILL_INTERFACE(PlaySound, (const std::string&) name);

  /**
   * DEBUG SKILL
   * This skill makes the Nao say something if it differs from what was said in the last frame.
   * @param name The text to be synthesized and pronounced
   * @param speed Use speed < 1 to talk slower and speed > 1 to talk faster.
   */
  SKILL_INTERFACE(Say, (const std::string&) text, (float)(1.f) speed);

  /**
   * DEBUG SKILL
   * Skill used by GameplayCard System card
   * Says the seconds remaining in the Half
   */
  SKILL_INTERFACE(CountDownHalfTime);

  /**
   * DEBUG SKILL
   * This skill makes the whole team say something at the same time
   * if it differs from what was said in the last frame.
   * @param name The text to be synthesized and pronounced
   */
  SKILL_INTERFACE(TeamSpeaker);

  /**
   * ACTION SKILL
   * This skill controls the head for a robot that walks to the ball.
   * @param distanceToTarget The distance to the kick pose (from which the ball can be kicked).
   * @param lookAtKickTarget Whether the robot is allowed to look at the kick target.
   * @param kickTargetRelative The kick target (only needed if the previous parameter is true).
   */
  SKILL_INTERFACE(GoToBallHeadControl, (float) distanceToTarget, (bool)(false) lookAtKickTarget, (Vector2f)(Vector2f::Zero()) kickTargetRelative);

  /**
   * This skill replays walk phases.
   */
  SKILL_INTERFACE(ReplayWalk);

  /**
   * ACTION SKILL
   * This skill walks to a target using intelligent path planning.
   * @param target The target pose in robot-relative coordinates
   * @param speed The walking speed as ratio of the maximum speed in [0, 1]
   * @param rough Ignore obstacles if they prevent the robot from reaching its target
   * @param disableObstacleAvoidance Disables avoiding dynamic obstacles like robots and referees (keeps goal posts, own penalty area and field border avoidance enabled)
   * @param disableAligning Disables rotation to the walk target
   * @param disableStanding Disables standing at the target (i.e. if set to true, the robot will walk on the target spot)
   */
  SKILL_INTERFACE(WalkToPoint, (const Pose2f&) target, (float)(1.f) speed,
                  (bool)(false) rough, (bool)(false) disableObstacleAvoidance, (bool)(false) disableAligning,
                  (bool)(false) disableStanding);

  /**
   * ACTION SKILL
   * This skill walks to a kickoff pose (i.e. in the ready state).
   * Has Precise Alignment and ensures standing still before Kickoff
   * @param target The target pose in absolute field coordinates
   */
  SKILL_INTERFACE(WalkToKickoffPose, (const Pose2f&) target);

  /**
   * ACTION SKILL
   * This skill turns the robot on the spot by a specified angle.
   * @param angle The angle relative to the rotation that the robot had when the skill started
   * @param margin The tolerance for the skill to be done
   */
  SKILL_INTERFACE(TurnAngle, (Angle) angle, (Angle)(5_deg) margin);

  /**
   * ACTION SKILL
   * This skill turns the robot to look forward at a target.
   * Does not move head
   * @param target The position in robot relative coordinates to turn to
   * @param margin The tolerance for the skill to be done
   */
  SKILL_INTERFACE(TurnToPoint, (const Vector2f&) target, (Angle)(5_deg) margin);

  /**
   * ACTION SKILL
   * This skill walks to the ball and dribbles it from there.
   * @param targetDirection The direction to which the ball should be dribbled in robot-relative coordinates
   * @param alignPrecisely Whether the robot should align more precisely than usual
   * @param kickPower The desired strength of the dribble kick
   * @param preStepAllowed Is a prestep for the InWalkKick allowed?
   * @param turnKickAllowed Does the forward kick not need to align with the kick direction?
   * @param directionPrecision The allowed deviation of the direction in which the ball should go. If default, the WalkToBallAndKickEngine uses its own precision.
   */
  SKILL_INTERFACE(GoToBallAndDribble, (Angle) targetDirection, (bool)(false) alignPrecisely, (float)(1.f) kickPower, (bool)(true) preStepAllowed, (bool)(true) turnKickAllowed, (const Rangea&)(Rangea(0_deg, 0_deg)) directionPrecision);

  /**
   * ACTION SKILL
   * This skill walks to the ball and executes a kick there.
   * Can be used to execute more Powerful or Custom Kicks
   * @param targetDirection The direction to which the ball should be kicked in robot-relative coordinates
   * @param kickType The kick type that should be executed there
   * @param alignPrecisely Whether the robot should align more precisely than usual
   * @param length The desired length of the kick (works only for certain types of kicks)
   * @param preStepAllowed Is a prestep for the InWalkKick allowed?
   * @param turnKickAllowed Does the forward kick not need to align with the kick direction?
   * @param speed The walking speed
   * @param directionPrecision The allowed deviation of the direction in which the ball should go. If default, the WalkToBallAndKickEngine uses its own precision.
   */
  SKILL_INTERFACE(GoToBallAndKick, (Angle) targetDirection, (KickInfo::KickType) kickType, (bool)(true) alignPrecisely, (float)(std::numeric_limits<float>::max()) length, (bool)(true) preStepAllowed, (bool)(true) turnKickAllowed, (const Pose2f&)(Pose2f(1.f, 1.f, 1.f)) speed, (const Rangea&)(Rangea(0_deg, 0_deg)) directionPrecision);

  /**
   * ACTION SKILL
   * This skill walks very carefully to a kick pose and executes a kick there.
   * Specifically made for penalty kicks
   * @param kickPose The pose at which the kick should be executed in robot-relative coordinates
   * @param kickType The kick type that should be executed there
   * @param walkSpeed The walking speed as ratio of the maximum speed in [0, 1]
   */
  SKILL_INTERFACE(PenaltyStrikerGoToBallAndKick, (const Pose2f&) kickPose, (KickInfo::KickType) kickType, (float) walkSpeed);

  /**
   * This skill intercepts a rolling ball with the goal that it does not pass the y axis of this robot.
   * @param interceptionMethods A bit set of methods that may be used to intercept the ball (from Interception::Method).
   * @param allowGetUp Whether the robot is allowed to get up afterwards.
   * @param allowDive Whether the robot is allowed to actually dive. Otherwise, only sounds are played to indicate what it would do.
   */
  SKILL_INTERFACE(InterceptBall, (unsigned) interceptionMethods, (bool)(true) allowGetUp, (bool)(true) allowDive);

  /**
   * This skill continues a ball interception which involves diving.
   * @param allowGetUp Whether the robot is allowed to get up afterwards. Note: If false, this skill will never be done.
   */
  SKILL_INTERFACE(AfterInterceptBall, (bool)(true) allowGetUp);

  /**
   * COM SKILL
   * DEBUG SKILL
   * This skill records the walkingTo target and desired speed in the BehaviorStatus.
   * Can be used to draw Walk target on World State
   * @param target The target the robot is walking towards.
   * @param speed The desired relative speed.
   */
  SKILL_INTERFACE(RecordTargetAndSpeed, (const Vector2f) target, (float)(1.f) speed);

  /**
   * This skill can be used to calibrate the robot.
   */
  SKILL_INTERFACE(CalibrateRobot, (const CalibrationRequest&) request);

  /**
   * This skill can be used to request a reload of the jointCalibration.cfg.
   */
  SKILL_INTERFACE(ReloadJointCalibration);
}
