/**
 * @file KeyLogger.h
 *
 * This class manages the used Keys of the PS4 Controller.
 * 
 * It takes information from the B-Human routines about which button got pressed to which moment. 
 * This information is shared public and allows the Cardsystem to react on Key-Events and with that it allows an highlevel/symbolic use 
 * of Keyevents because hole Skills can be seadled on the keys now.
 * On the other hand it is able to protocoll all done actions with the gamepad on command. Allowing to create the playback, which is later stored in the teachIn CSV files
 *
 * Information is taken from:
 * ConsoleView.cpp
 * RobotConsole.cpp
 * ConsoleRoboCupCtrl.cpp
 * 
 * Information is provided to:
 * TeachinCard.cpp
 * TechInCard_XXX.csv files
 *
 * @author R-ZWEI KICKERS Felix Mayer, Jannis Schottler
 */

#pragma once

#include <queue>
#include <mutex>
#include <fstream>
#include <chrono>
#include "Tools/Math/Eigen.h"
//#include "Column.h" - ToDo remove
//#include "Tools/RZWEI/RZWEI_Constant.h" - ToDo remove

namespace Keys
{
#if defined(MACOS) || defined(WINDOWS)

  // Controller:
  // Keys:
  constexpr int Cross       = 1;
  constexpr int Square      = 0;
  constexpr int Triangle    = 3;
  constexpr int Circle      = 2;
  constexpr int R1          = 5;
  constexpr int R2          = 7;
  constexpr int R3          = 11;   // stick press
  constexpr int L1          = 4;
  constexpr int L2          = 6;
  constexpr int L3          = 10;   // stick press
  constexpr int Start       = 9;
  constexpr int Share       = 8;
  constexpr int Home        = 12;   // PS Button
  constexpr int TouchButton = 13;
  constexpr int Mute        = 14;   // DualSense Mute

  constexpr int DPadDown    = 36;
  constexpr int DPadLeft    = 38;
  constexpr int DPadUp      = 32;
  constexpr int DPadRight   = 34;
  // Joystick:
  constexpr int LeftCursorUp      = 1;
  constexpr int LeftCursorDown    = -1;
  constexpr int LeftCursorRight   = -1;
  constexpr int LeftCursorleft    = 1;
  constexpr int RightCursorReight = -1;
  constexpr int RightCursorleft   = 1;
  constexpr int NoCursor          = 0;

  // Nothing
  constexpr int NoKey = -1;

#else
  // Controller:
  // Keys:
  constexpr int Cross       = 0;
  constexpr int Circle      = 1;
  constexpr int Triangle    = 2;
  constexpr int Square      = 3;
  constexpr int L1          = 4;
  constexpr int R1          = 5;
  constexpr int L2          = 6;
  constexpr int R2          = 7;
  constexpr int Share       = 8;   
  constexpr int Start       = 9;    // Options-Button ins some Controllers
  constexpr int Home        = 10;   // PS-Button
  constexpr int L3          = 11;   // analog stick press
  constexpr int R3          = 12;   // analog stick press
  constexpr int TouchButton = 13;   // TODO: confirm id -> not confirmed ... no id given on press
  constexpr int Mute        = 14;   // DualSense Mute TODO: confirm id

  constexpr int DPadDown    = 30;   // TODO: confirm ids
  constexpr int DPadLeft    = 30;
  constexpr int DPadUp      = 30;
  constexpr int DPadRight   = 30;
  // Joystick:
  constexpr int LeftCursorUp      = 1;
  constexpr int LeftCursorDown    = -1;
  constexpr int LeftCursorRight   = -1;
  constexpr int LeftCursorleft    = 1;
  constexpr int RightCursorReight = -1;
  constexpr int RightCursorleft   = 1;
  constexpr int NoCursor          = 0;

  // Nothing
  constexpr int NoKey = -1;

#endif
} // namespace Keys

class KeyLogger
{
private:
    static KeyLogger *keyLoggerInstance;

public:
    static KeyLogger *getInstance();

private:
    KeyLogger() = default;
    int mKey = -1;
    std::mutex mtx;
    int lastGamePlayTime = 0;
    Vector2f leftCursor = Vector2f(0, 0);
    Vector2f rightCursor = Vector2f(0, 0);
    bool fastMode = false;
    bool headUp = false;
    bool startGamepadControl = false;
    bool saving = false;
    std::string robotName = "robot-1"; //undefined
    float protocolIntervals=500; // protocol the actions for half second

    std::queue<int> keyEvents;

    int recordingStartTime = 0;

    int64_t r1_lastHit;
    int64_t r2_lastHit;
    int64_t r3_lastHit;
    int64_t l1_lastHit;
    int64_t l2_lastHit;
    int64_t l3_lastHit;
    int64_t cross_lastHit;
    int64_t square_lastHit;
    int64_t circle_lastHit;
    int64_t triangle_lastHit;
    int64_t start_lastHit;
    int64_t share_lastHit;
    int64_t home_lastHit;

    /**
     * Returns the last time the specififc kex got hitted
     * @param key the specific key which last hit time should be returned
     * @return Returns the last moment where the key got pressed
     */

    /**
     * Calculates the current time in ms
     *  @return Returns the time in ms
     */
    int64_t now() const;

    /**
     * Returns the last time the specififc kex got hitted
     * @param key the specific key which last hit time should be returned
     * @return Returns the last moment where the key got pressed
     */
    int64_t getKeyHitTime(int key) const;

    /**
     * Returns the last time the specififc kex got hitted
     * @param key the specific key which last hit time should be updated
     * @param time the last moment where the key got pressed
     */
    void setKeyHitTime(int key, int64_t time);

public:
    KeyLogger(KeyLogger const &) = delete;
    void operator=(KeyLogger const &) = delete;

    /**
     * returns the pressed key
     *  @return the last pressed key
     */
    int getKey() const;

    bool hasEvent() const;
    
    int nextEvent();
    
    void clearEvents();

    /**
     * get the left cursor values
     *  @return the x and y values of the left cursor
     */
    Vector2f getLeftCursor() const;

    /**
     * get the right cursor values
     *  @return the x and y values of the right cursor
     */
    Vector2f getRightCursor() const;

    /**
     * get the speed modes
     *  @return true, if the robot should be in fast mode, else false
     */
    bool isFastMode() const;

    /**
     * toggles between max und default speed (default is specified in RZWEI_const namespace)
     * @param usedKey the key which is binded to this action
     */
    void toggleFastMode(int usedKey);

    /**
     * get the height of the head
     *  @return true, if the head shoud be looking upwards, else false
     */
    bool isHeadUp() const;

    /**
     * get the speed modes.True, if the robot should be in fast mode, else false
     * @param usedKey the key which is binded to this action
     */
    void toggleHead();

    /**
     * checks if the start/option button got pressed
     *  @return true, if the start/option button got pressed, else false
     */
    bool isStartPressed() const;

    /**
     * toggles between the start/option button modes.
     * First time it got pressed or if it got pressed a second time
     */
    void toggleStart();

    /**
    * toggles between the start/option button modes.
     * First time it got pressed or if it got pressed a second time. 
     * This action shoudn't be used on keys! because it would trigger several times and not only 1 time per button hit. 
     * It is used to force a switch of the start button mode inside a codeblock and allows it to use it without the gamepad.
     * For gamepad usage see "toggleStart()"
     */
    void forceToToggleStart();

    /**
     * Checks if the specififc key just got pressed
     * @param usedKey the specific key to check if it just got pressed
     * @param timePeriod the past time in which the key got pressed in ms
     * @return true, if the key got pressed less than timePeriod ms ago, else false
     */
    bool keyPressedJustNow(int usedKey, int timePeriod);

    /**
     * set the key once it got pressed
     * @param key which got got pressed
     */
    void setKey(int key);

    /**
     * set the information about the left jyostick
     * @param x the x axis of the joystick
     * @param y the y axis of the joystick
     */
    void setLeftCursor(float x, float y);

    /**
     * set the information about the left jyostick
     * @param left the vector representing the x and y axis of the joystick
     */
    void setLeftCursor(Vector2f left);

    /**
     * set the information about the right jyostick
     * @param x the x axis of the joystick
     * @param y the y axis of the joystick
     */
    void setRightCursor(float x, float y);

    /**
     * set the information about the right jyostick
     * @param right the vector representing the x and y axis of the joystick
     */
    void setRightCursor(Vector2f right);

    /**
     * protocolls the robot name of the currently selected
     * @param name the name of the robot
     */
    void setRobotName(std::string name);

    /**
     * returns the name of the currently selected robot
     * @return the name of the selected robot
     */
    std::string getRobotName() const;

    /**
     * returns the number of the currently selected robot
     * @return the number of the selected robot
     */
    int getRobotNumber() const;

    /**
     * returns the amount of time to pass to protocol an acton
     *  @return the time in ms
     */
    float getProtocolInterval() const;

    /**
     * protocolls the actions of the controller to the playback storage
     * @param playbackAction struct encapsulating all relevant playback data 
     */
		void protocol(struct PlaybackAction* playbackAction);

    /**
     * the playback to .csv file format using rapidCSV
     */
    void saveProtocol();

    /**
     * get the speed modes.True, if the robot should be in fast mode, else false
     * @param usedKey the key which is binded to this action
     */
    void toggleSaving();
};
