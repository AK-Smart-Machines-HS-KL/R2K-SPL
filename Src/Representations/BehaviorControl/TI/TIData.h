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

/**
 * @brief Skill Parameter Mapping Guide for CSV Authors
 * 
 * This section documents which parameters are used by each skill.
 * When authoring CSV files, only populate the columns relevant to your skill.
 * 
 * ============================================================================
 * SUPPORTED SKILLS & PARAMETER USAGE
 * ============================================================================
 * 
 * 1. Stand
 *    Purpose: Robot stands in place
 *    Parameters used: None
 *    maxTime: Recommended 100-500ms (brief stance hold)
 *    Example: "Stand, 100, , , , , , , , "
 * 
 * 2. WalkAtRelativeSpeed
 *    Purpose: Walk at a given relative speed (forward/backward/sideways)
 *    Parameters used: floatParam (speed), angleParam1 (direction)
 *    maxTime: Duration of walk (1000-5000ms typical)
 *    - floatParam: Speed in mm/s (range: -500 to +500, negative = backward)
 *    - angleParam1: Walk direction in radians (range: -π to +π)
 *                   0 = forward, π/2 = left, -π/2 = right
 *    Example: "WalkAtRelativeSpeed, 2000, 1.57, , , , 250.0, , , "
 * 
 * 3. WalkToPoint
 *    Purpose: Walk to a specific point on the field relative to robot pose
 *    Parameters used: poseParam (target position and rotation)
 *    maxTime: Duration allowed to reach target (3000-8000ms typical)
 *    - poseParam.translation.x(): Target X position in mm (range: -10000 to +10000)
 *    - poseParam.translation.y(): Target Y position in mm (range: -10000 to +10000)
 *    - poseParam.rotation: Target rotation in radians (range: -π to +π)
 *    Example: "WalkToPoint, 5000, , , 1500.0|500.0|0.0, , , , , "
 *    Note: Format in CSV is "x|y|rotation"
 * 
 * 4. KickAtGoal
 *    Purpose: Kick the ball toward the opponent goal
 *    Parameters used: floatParam (kick power/strength)
 *    maxTime: Duration of kick execution (500-1500ms typical)
 *    - floatParam: Kick power percentage (range: 0.0 to 1.0, where 1.0 = full power)
 *                  Recommended: 0.5-0.8 for reliable kicks, <0.2 for gentle kicks
 *    Example: "KickAtGoal, 800, , , , , 0.7, , , "
 * 
 * 5. WalkToBall
 *    Purpose: Walk to the ball and align for kicking
 *    Parameters used: angleParam1 (kick angle target)
 *    maxTime: Duration allowed to reach ball (4000-8000ms typical)
 *    - angleParam1: Target kick angle in radians (range: -π to +π)
 *                   0 = forward kick, π/2 = left kick, -π/2 = right kick
 *    Example: "WalkToBall, 6000, 0.0, , , , , , , "
 * 
 * 6. Dribble
 *    Purpose: Dribble the ball in a controlled manner toward a direction
 *    Parameters used: angleParam1 (dribble direction) ONLY
 *    maxTime: Duration of dribble (2000-4000ms typical)
 *    - angleParam1: Direction to dribble in radians (range: -π to +π)
 *                   0 = forward, π/2 = left, -π/2 = right
 *    - floatParam: IGNORED - dribble always executes at maximum speed
 *                  (kept for potential future enhancements, currently unused)
 *    Example: "Dribble, 3000, 0.785, , , , , , , "
 *    Note: Dribble always uses full speed regardless of floatParam value.
 *          Direction is controlled solely by angleParam1.
 *    Note: Tactical repositioning skill for moving the ball short distances with control
 * 
 * ============================================================================
 * PARAMETER FORMAT REFERENCE
 * ============================================================================
 * 
 * Angle Parameters (angleParam1, angleParam2):
 *   - Unit: Radians
 *   - Range: -π (-3.14159) to +π (+3.14159)
 *   - Commonly used: 0, π/4 (0.785), π/2 (1.571), π, -π/2 (-1.571)
 * 
 * Pose Parameter (poseParam):
 *   - Format in CSV: "x|y|rotation" (pipe-separated)
 *   - x, y: Position in millimeters (range: ±10000 typical)
 *   - rotation: Angle in radians (range: -π to +π)
 *   - Example: "1500.0|500.0|0.0" means position (1500mm, 500mm) facing forward
 * 
 * Vector3 Parameter (vector3Param):
 *   - Format in CSV: "x|y|z" (pipe-separated)
 *   - Unit: Millimeters (for positional vectors)
 *   - Range: ±10000 typical per component
 * 
 * Float Parameter (floatParam):
 *   - Unit: Varies by skill (see skill documentation above)
 *   - Range: Typically 0.0 to 1.0 (normalized) or -500 to +500 (speed)
 * 
 * Integer Parameter (intParam):
 *   - Unit: Depends on skill context
 *   - Range: -2147483648 to +2147483647 (32-bit signed int)
 * 
 * Boolean Parameter (boolParam):
 *   - Use "true" or "false" or "1" or "0" in CSV
 * 
 * String Parameter (stringParam):
 *   - Text value (no special formatting required)
 * 
 * maxTime:
 *   - Unit: Milliseconds (ms)
 *   - Purpose: Maximum duration skill is allowed to execute
 *   - Range: 0-999999 typical
 *   - Important: Set appropriately to prevent runaway skill execution
 * 
 * ============================================================================
 * CSV AUTHORING GUIDE
 * ============================================================================
 * 
 * CSV Column Order (in both playing_*.csv and worldmodel_*.csv):
 *   1. Skill/Field name (e.g., "Stand", "WalkAtRelativeSpeed")
 *   2. maxTime (milliseconds)
 *   3. angleParam1 (radians)
 *   4. angleParam2 (radians)
 *   5. poseParam (format: "x|y|rotation")
 *   6. vector3Param (format: "x|y|z")
 *   7. boolParam (true/false)
 *   8. floatParam (depends on skill)
 *   9. intParam (integer)
 *  10. stringParam (text)
 * 
 * Guidelines:
 *   - Use empty cells for unused parameters
 *   - Keep maxTime realistic (add buffer for processing: typically +500ms)
 *   - Use consistent decimal format (e.g., always 1500.0 not 1500)
 *   - Test values incrementally (start with lower power/speed, increase as needed)
 *   - Coordinate system: X = forward, Y = left, origin = robot position at record time
 * 
 * Example CSV Row (WalkToPoint then Stand):
 *   WalkToPoint,5000,,,"1000.0|500.0|0.0",,,,,,
 *   Stand,200,,,,,,,,,
 * 
 * ============================================================================
 */

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
   * @note See Skill Parameter Mapping Guide (above) for parameter requirements per skill
   */
  PlaybackAction& setSkill(PlaybackAction::Skills);
  
  /** @brief Set first angle parameter (chainable)
   * @param angle The angle value in radians (range: -π to +π)
   * @return Reference to this PlaybackAction
   * @note Used by: WalkAtRelativeSpeed (walk direction), WalkToBall (kick angle)
   * @note Coordinate system: 0 = forward, π/2 = left, -π/2 = right
   */
  PlaybackAction& setAngle1(const Angle&);
  
  /** @brief Set second angle parameter (chainable)
   * @param angle The angle value in radians (range: -π to +π)
   * @return Reference to this PlaybackAction
   * @note Reserved for skills requiring dual angle parameters
   * @note Currently used by: (reserved for future skills)
   */
  PlaybackAction& setAngle2(const Angle&);
  
  /** @brief Set pose parameter (chainable)
   * @param pose Target pose with translation (x, y in mm) and rotation (radians)
   * @return Reference to this PlaybackAction
   * @note Used by: WalkToPoint
   * @note Units: x, y in millimeters (range: ±10000); rotation in radians (range: -π to +π)
   * @note Coordinate system: relative to robot's starting pose at sequence trigger
   */
  PlaybackAction& setPose(const Pose2f&);
  
  /** @brief Set 3D vector parameter (chainable)
   * @param vec Vector with x, y, z components in millimeters
   * @return Reference to this PlaybackAction
   * @note Currently unused; reserved for future skills requiring 3D positioning
   * @note Example use case: Ball projection in 3D space for advanced kicks
   */
  PlaybackAction& setVector(const Vector3f&);
  
  /** @brief Set boolean parameter (chainable)
   * @param val Boolean value (true/false)
   * @return Reference to this PlaybackAction
   * @note Used by: (reserved for conditional skill behavior)
   * @note Example: enable head tracking, request feedback, etc.
   */
  PlaybackAction& setBool(bool);
  
  /** @brief Set floating point parameter (chainable)
   * @param val Float value, unit depends on skill context
   * @return Reference to this PlaybackAction
   * @note Used by: WalkAtRelativeSpeed (speed in mm/s), KickAtGoal (power 0.0-1.0)
   * @note Typical range: 0.0-1.0 (normalized) or -500 to +500 (speed)
   */
  PlaybackAction& setFloat(float);
  
  /** @brief Set integer parameter (chainable)
   * @param val Integer value
   * @return Reference to this PlaybackAction
   * @note Used by: (reserved for discrete skill parameters, e.g., repetition count)
   * @note Range: -2147483648 to +2147483647 (32-bit signed int)
   */
  PlaybackAction& setInt(int);
  
  /** @brief Set string parameter (chainable)
   * @param val String value (e.g., target name, reference label)
   * @return Reference to this PlaybackAction
   * @note Used by: (reserved for named targets or reference strings)
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
      Dribble,
    }),

  (PlaybackAction::Skills) skill,    // Enum value of Skill to execute
  (int)         maxTime,      // maximum milliseconds allowed for skill execution (recommended: +500ms buffer)
  (Angle)       angleParam1,  // rotation/angular parameter 1 (radians, range: -π to +π)
  (Angle)       angleParam2,  // rotation/angular parameter 2 (radians, range: -π to +π)
  (Pose2f)      poseParam,    // pose parameter: position (mm) + rotation (radians)
  (Vector3f)    vector3Param, // 3D vector parameter (x, y, z in mm; reserved for future)
  (bool)        boolParam,    // boolean parameter (reserved for conditional behavior)
  (float)       floatParam,   // floating point parameter (unit varies: speed mm/s, power 0.0-1.0, etc.)
  (int)         intParam,     // integer parameter (reserved for discrete values, e.g., counts)
  (std::string) stringParam,  // string parameter (reserved for named targets or labels)
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