/**
 * @file DefaultPoseProvider.h
 * @author Jonas Lambing (R2K), Adrian Müller (R2K), Andy Hobelsberger (R2)
 * @brief Module for providing default scenario poses
 * 
 * This Module Loads Default Poses from defaultPoseProvider.cfg and Assigns them to robots based on their Roles.
 * Default poses are translated forward and backward based on team strategy. See offsetOffense / offsetDefense.
 * 
 * If autoScale is enabled, the module attempts to automatically scale the coordinates 
 * to the field size set by the Location, assuming that the Poses are given in standard 6m x 9m field coordinates 
 * 
 * The defaultPose representation gets updated on a per robot basis, that means, that
 * the update method of this module gets called before the card using it gets executed
 * by the robot. 
 * 
 * @version 2.1
 * @date 2022-09-14
 * - in 2.1: added percentageBallAdaption = 0.33; This is the percentage for the y-axis, i.e., the default y positions changes a little,
 * - adaption dynamically to the FieldBalls y-positions. Set to 0 in the .cfg, if you prefer static positions. 
 *
 */

#pragma once
#include "Tools/Module/Module.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"  
#include "Representations/BehaviorControl/DefaultPose.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Configuration/GlobalOptions.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/TeamData.h"

#include "Representations/BehaviorControl/FieldBall.h"  // dynamic adjustment 

MODULE(DefaultPoseProvider,
{,
  PROVIDES(DefaultPose),
 
  REQUIRES(FieldBall),
  REQUIRES(FieldDimensions),
  REQUIRES(TeammateRoles),
  REQUIRES(GlobalOptions),
  REQUIRES(RobotPose),
  REQUIRES(RobotInfo),
  REQUIRES(TeamData),
  REQUIRES(TeamBehaviorStatus),

  LOADS_PARAMETERS(
  {,
    (bool) (false) autoScale, // wether or not to scale the values to the current field dimensions
    
    // Strategy Offsets
    (float) offsetOffense,
    (float) offsetDefense,
    (float) percentageBallAdaption,

    // Poses
    // #Goalie
    (Pose2f) goaliePose,
    (Pose2f) goalieForwardPose,
    // #Offense
    (Pose2f) offenseMidPose,
    (Pose2f) offenseLeftPose,
    (Pose2f) offenseRightPose,
    // #Defense
    (Pose2f) defenseMidPose,
    (Pose2f) defenseLeftPose,
    (Pose2f) defenseRightPose, 

    (float) PositionTolerance, // How far a Robot can be from it's default position to count as being on it
    (Angle) AngleTolerance,    // How far a robot can be rotated it's default pose look direction to count as being on it 
  }),
});

class DefaultPoseProvider : public DefaultPoseProviderBase
{

  bool isInit = false;
  /**
   * @brief This function gets triggered whenever a card gets executed that uses it
   * 
   * @param defaultPose The default-pose that we are going to update
   */
  void update(DefaultPose& defaultPose) override;
};