/**
 * @file TIData.h
 * @author Andreas Hobelsberger
 * @brief Defines Classes used by the TeachIn Process
 * 
 * Primary Classes:
 * - WorldModel: Contains data about the state of the world. this is a single image of the World at a given point in time.
 * - WorldData: Contains multiple World Models, and a copy to the last one in the list being the trigger point.
 * 
 * - PlaybackAction: A single action of the playback system, such as walkToBall and it's parameters, if needed
 * - PlaybackSequence: A series of playback actions that constitute a playback recording 
 * 
 * @version 1.0
 * @date 2022-01-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include "Tools/Math/Eigen.h"
#include "Tools/Math/Angle.h"
#include "Tools/Math/Pose2f.h"
#include "Tools/Streams/AutoStreamable.h"
#include "Tools/Streams/Enum.h"
#include "Platform/File.h"

const std::string TI_Directory = ((std::string) File::getBHDir()) + "/Config/TeachIn/"; 

STREAMABLE(WorldModel,
{,
  (int)                   gameState,            // what state the game was in while recording (see RoboCupGameControlData.h)
  (int)                   setPlay,              // the Set Play (see RoboCupGameControlData.h)
  (uint32_t)              timeLeft,             // how much time is left in the match
  (int)                   robotNumber,          // the number of the robot that recorded
  (Pose2f)                robotPose,            // the pose of the robot that recorded
  (float)                 distanceToGoal,       // the distance between the goal and the bot
  (bool)                  ballIsNear,           // was the ball near the bot at the time of recording
  (Vector2f)              ballPosition,         // the ball-position
  (float)                 ballDistanceToBot,    // the distance between the ball and the bot
  (float)                 ballDistanceToGoal,   // the distance between the ball and the goal
  (std::vector<Vector2f>) teamList,             // a list of teammates (5 max, according to game, missing bots will be represented with 0)
  (std::vector<Vector2f>) opponentList,         // a list of opponents (5 max, according to game, missing bots will be represented with 0)
});

STREAMABLE(WorldData,
{
    /** @brief Default constructor */
    WorldData() = default;
    
    /** @brief Load worldmodel data from CSV file
     * @param file Path to the worldmodel CSV file
     * @param isRelative If true, path is relative to TeachIn directory; if false, path is absolute
     */
    WorldData(std::string, bool isRelative = true);
    
    /** @brief Clear all loaded worldmodel data */
    void clear();
    
    /** @brief Save worldmodel data to CSV file
     * @param path Path where the file should be saved (uses fileName if empty)
     */
    void save(std::string path);
    ,

    (std::string)           fileName,             // name of the file on disk
    (std::vector<WorldModel>) models,             // World Models (one per frame during recording)
    (WorldModel)             trigger,              // trigger point (last model in the sequence)
});

// Design playback columns: per datatype, assuming most skills will only use few params
// Pro: csv has a homogenous column signature
// If more params are needed -> append new column
STREAMABLE(PlaybackAction,
{
  /** Supported TI Skills
   * If you add a new Skill to this Enum, add it's mapping in Execute.cpp 
  */
  enum Skills : unsigned char;

  /** @brief Set skill type (chainable)
   * @param skill The skill enum value to execute
   * @return Reference to this PlaybackAction (for method chaining)
   */
  PlaybackAction& setSkill(PlaybackAction::Skills);
  
  /** @brief Set first angle parameter (chainable)
   * @param angle The angle value (typically in radians)
   * @return Reference to this PlaybackAction
   */
  PlaybackAction& setAngle1(const Angle&);
  
  /** @brief Set second angle parameter (chainable)
   * @param angle The angle value (typically in radians)
   * @return Reference to this PlaybackAction
   */
  PlaybackAction& setAngle2(const Angle&);
  
  /** @brief Set pose parameter (chainable)
   * @param pose Target pose with translation (x, y) and rotation
   * @return Reference to this PlaybackAction
   */
  PlaybackAction& setPose(const Pose2f&);
  
  /** @brief Set 3D vector parameter (chainable)
   * @param vec Vector with x, y, z components
   * @return Reference to this PlaybackAction
   */
  PlaybackAction& setVector(const Vector3f&);
  
  /** @brief Set boolean parameter (chainable)
   * @param val Boolean value
   * @return Reference to this PlaybackAction
   */
  PlaybackAction& setBool(bool);
  
  /** @brief Set floating point parameter (chainable)
   * @param val Float value
   * @return Reference to this PlaybackAction
   */
  PlaybackAction& setFloat(float);
  
  /** @brief Set integer parameter (chainable)
   * @param val Integer value
   * @return Reference to this PlaybackAction
   */
  PlaybackAction& setInt(int);
  
  /** @brief Set string parameter (chainable)
   * @param val String value
   * @return Reference to this PlaybackAction
   */
  PlaybackAction& setString(const std::string&);

  ENUM(Skills, 
    {,
      Default,
      Stand,
      WalkAtRelativeSpeed,
      WalkToPoint,
      KickAtGoal,
      WalkToBall,
    }),

  (PlaybackAction::Skills) skill,    // Enum value of Skill to execute
  (int)         maxTime,      // how long the skill is allowed to execute for
  (Angle)       angleParam1,  // rotation/angular parameter 1
  (Angle)       angleParam2,  // rotation/angular parameter 2
  (Pose2f)      poseParam,    // pose parameter (position x,y and rotation)
  (Vector3f)    vector3Param, // 3D vector parameter (x, y, z components)
  (bool)        boolParam,    // boolean parameter
  (float)       floatParam,   // floating point parameter
  (int)         intParam,     // integer parameter
  (std::string) stringParam,  // string parameter
});

STREAMABLE(PlaybackSequence, 
{
  /** @brief Default constructor */
  PlaybackSequence() = default;
  
  /** @brief Load playback sequence from CSV file
   * @param file Path to the playback CSV file
   * @param isRelative If true, path is relative to TeachIn directory; if false, path is absolute
   */
  PlaybackSequence(std::string, bool isRelative = true);
  
  /** @brief Clear all loaded actions from this sequence */
  void clear();
  
  /** @brief Save playback sequence to CSV file
   * @param path Path where the file should be saved (uses fileName if empty)
   */
  void save(std::string path);
  
  ,
  (std::string)                   fileName,   // Name of the file on disk
  (std::vector<PlaybackAction>)   actions,    // vector of all actions in the file
});