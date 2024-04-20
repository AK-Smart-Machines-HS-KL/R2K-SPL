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
    WorldData() = default;
    WorldData(std::string, bool isRelative = true);
    void clear();
    void save(std::string path);
    ,

    (std::string)           fileName,             // name of the file on disk
    (std::vector<WorldModel>) models,             // World Models
    (WorldModel)             trigger,
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

  PlaybackAction& setSkill(PlaybackAction::Skills);
  PlaybackAction& setAngle1(const Angle&);
  PlaybackAction& setAngle2(const Angle&);
  PlaybackAction& setPose(const Pose2f&);
  PlaybackAction& setVector(const Vector3f&);
  PlaybackAction& setBool(bool);
  PlaybackAction& setFloat(float);
  PlaybackAction& setInt(int);
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
  (Angle)       angleParam1,  // this and the following members are parameters to use when executing the skill
  (Angle)       angleParam2,  
  (Pose2f)      poseParam,    
  (Vector3f)    vector3Param, 
  (bool)        boolParam,
  (float)       floatParam,
  (int)         intParam,
  (std::string) stringParam,
});

STREAMABLE(PlaybackSequence, 
{
  PlaybackSequence() = default;
  PlaybackSequence(std::string, bool isRelative = true);
  void clear();
  void save(std::string path);
  
  ,
  (std::string)                   fileName,   // Name of the file on disk
  (std::vector<PlaybackAction>)   actions,    // vector of all actions in the file
});