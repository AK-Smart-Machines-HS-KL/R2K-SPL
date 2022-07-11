/**
 * @file DefaultPoseProvider.h
 * @author Jonas Lambing (R2K), Adrian Müller (R2K)
 * @brief Module for providing default scenario poses
 * 
 * This module loads 5 margins (one for each bot) from defaultPoseProvider.cfg.
 * These margins are 2d vectors that contain a fraction between 0 and 1 in both x and y.
 * The margins will then be converted to absolute positions based on the playing fields
 * dimensions (eg. { 0.5, 0.5 } will be the center of the field).
 * 
 *    0          x         1  (entire field)
 * 0  +--------------------+
 *    |         |          |
 * y  |         O          |
 *    |         |          |
 * 1  +--------------------+
 * 
 * The defaultPose representation gets updated on a per robot basis, that means, that
 * the update method of this module gets called before the card using it gets executed
 * by the robot. 
 * 
 * 
 * v1.1. (Adrian): added fixc: returning defaultPose.getDefaultPosition(1); - the  goalie position - if 
 *    if(theTeamBehaviorStatus.teammateRoles.roles[theRobotInfo.number - 1] == TeammateRoles::GOALKEEPER)
 * @version 1.1
 * @date 2022-03-16
 * 
 * 
 * v.1.2 (Adrian) default position is now adjusted according to game state adn team strategy
 * a) normal game, R2K_NORMAL_GAME: v1.1. applies
 * b) opp kick off,R2K_NORMAL_GAME: two player close to the mid circle, R2K_DEFENSIVE_GAME only one  
   c) own kick off:  two player close to the mid circle both cases)


   v.1.3 (Thomas)
  adapt Walk-In Positions for own and opponent kickoff situations
 */

#pragma once
#include "Tools/Module/Module.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"  
#include "Representations/BehaviorControl/DefaultPose.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Configuration/GlobalOptions.h"
#include "Representations/Modeling/RobotPose.h"





MODULE(DefaultPoseProvider,
{,
  PROVIDES(DefaultPose),
 
  REQUIRES(FieldDimensions),
  REQUIRES(GameInfo),
  REQUIRES(GlobalOptions),
  REQUIRES(RobotPose),
  REQUIRES(RobotInfo),
  REQUIRES(OpponentTeamInfo),
  REQUIRES(OwnTeamInfo),
  REQUIRES(TeamBehaviorStatus),

  LOADS_PARAMETERS(
  {,
    (Vector2f) goalieMargin,
    (Vector2f) leftDefenseMargin,
    (Vector2f) rightDefenseMargin,
    (Vector2f) leftOffenseMargin,
    (Vector2f) rightOffenseMargin,
  }),
});

class DefaultPoseProvider : public DefaultPoseProviderBase
{
  /**
   * @brief This function gets triggered whenever a card gets executed that uses it
   * 
   * @param defaultPose The default-pose that we are going to update
   */
  void update(DefaultPose& defaultPose) override;
  
  /**
   * @brief This function calculates an absolute position from a given margin using the field dimensions
   * 
   * @param margin A 2d vector containing a set of margins in a range of 0-1
   * @return Vector2f The calculated absolute position in field coordinates
   */
  Vector2f calcAbsoluteFromMargin(const Vector2f &margin);

  /**
   * @brief This function draws the default positions inside the simulator (dr debugDrawing3d:test_defaultPos)
   * 
   */
  void drawDefaultPositions();
};