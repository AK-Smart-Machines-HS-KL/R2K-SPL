/**
 * @file RobotInfo.cpp
 * The file declares a struct that encapsulates the structure RobotInfo defined in
 * the file RoboCupGameControlData.h that is provided with the GameController.
 *
 * @author Thomas Röfer
 */

#include "RobotInfo.h"
#include "Platform/SystemCall.h"
#include "Tools/Global.h"
#include "Tools/Settings.h"
#include "Tools/Debugging/DebugDrawings3D.h"
#include <cstring>

RobotInfo::RobotInfo() :
  number(Global::settingsExist() ? Global::getSettings().playerNumber : 0),
  mode(SystemCall::getMode() == SystemCall::physicalRobot ? unstiff : active)
{
  memset(static_cast<RoboCup::RobotInfo*>(this), 0, sizeof(RoboCup::RobotInfo));
}

void RobotInfo::draw() const
{
  DEBUG_DRAWING3D("representation:RobotInfo", "robot")
  {
    float centerDigit = (number > 1) ? 50.f : 0;
    int num = number;
    ROTATE3D("representation:RobotInfo", 0, 0, pi_2);
    DRAWDIGIT3D("representation:RobotInfo", num, Vector3f(centerDigit, 0.f, 500.f), 80, 5, ColorRGBA::green);
  }
}

std::string RobotInfo::getModeAsString() const
{
  switch(mode)
  {
    case active: return "Active";
    case calibration: return "Calibatrion";
    case unstiff: return "Unstiff";
    case walktest: return "Walk Test";
    default: return "Unknown";
  }
}

std::string RobotInfo::getPenaltyAsString() const
{
  switch(penalty)
  {
    case PENALTY_ILLEGAL_POSITIONING: return "Illegal Position";
    case PENALTY_MOTION_IN_SET: return "Illegal Motion in Set";
    case PENALTY_LOCAL_GAME_STUCK: return "Local Game Stuck";
    case PENALTY_INCAPABLE_ROBOT: return "Incapable Robot";
    case PENALTY_PICK_UP: return "Request for Pickup";
    case PENALTY_BALL_HOLDING: return "Ball Holding";
    case PENALTY_LEAVING_THE_FIELD: return "Leaving the Field";
    case PENALTY_PLAYING_WITH_ARMS_HANDS: return "Playing with Arms/Hands";
    case PENALTY_PUSHING: return "Player Pushing";
    case PENALTY_SENT_OFF: return "Sent Off"; 
    case PENALTY_SUBSTITUTE: return "Substitute";
    case PENALTY_MANUAL: return "Manual";
    default: return "None";
  }
}

bool RobotInfo::isGoalkeeper() const
{
  return number == 1;
}

void RobotInfo::read(In& stream)
{
  STREAM(number); // robot number: 1..11
  STREAM(mode); // one of the modes
  STREAM(penalty); // PENALTY_NONE, PENALTY_ILLEGAL_BALL_CONTACT, ...
  STREAM(secsTillUnpenalised); // estimate of time till unpenalized.
}

void RobotInfo::write(Out& stream) const
{
  STREAM(number); // robot number: 1..11
  STREAM(mode); // one of the modes
  STREAM(penalty); // PENALTY_NONE, PENALTY_ILLEGAL_BALL_CONTACT, ...
  STREAM(secsTillUnpenalised); // estimate of time till unpenalized.
}

void RobotInfo::reg()
{
  PUBLISH(reg);
  REG_CLASS(RobotInfo);
  REG(number);
  REG(mode);
  REG(penalty);
  REG(secsTillUnpenalised);
}
