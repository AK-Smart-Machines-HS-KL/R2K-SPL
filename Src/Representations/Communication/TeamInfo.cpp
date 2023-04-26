/**
 * @file TeamInfo.cpp
 * The file implements a struct that encapsulates the structure TeamInfo defined in
 * the file RoboCupGameControlData.h that is provided with the GameController.
 * @author Thomas Röfer
 */

#include "TeamInfo.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Tools/Debugging/DebugDrawings.h"
#include "Tools/Debugging/DebugDrawings3D.h"
#include "Tools/Global.h"
#include "Tools/Math/Eigen.h"
#include "Tools/Module/Blackboard.h"
#include "Tools/Settings.h"
#include <algorithm>
#include <cstring>

/**
 * The struct is a helper to be able to stream the players.
 * The global RobotInfo cannot be used, because it has an additional attribute.
 */
struct PlayerInfo : public RoboCup::RobotInfo
{
private:
  static void reg()
  {
    PUBLISH(reg);
    REG_CLASS(PlayerInfo);
    REG(penalty);
    REG(secsTillUnpenalised);
  }
};

/**
 * Write a player info to a stream.
 * @param stream The stream that is written to.
 * @param playerInfo The data that is written.
 * @return The stream.
 */
Out& operator<<(Out& stream, const PlayerInfo& playerInfo)
{
  STREAM_EXT(stream, playerInfo.penalty);
  STREAM_EXT(stream, playerInfo.secsTillUnpenalised);
  return stream;
}

/**
 * Read a player info from a stream.
 * @param stream The stream that is read from.
 * @param playerInfo The data that is read.
 * @return The stream.
 */
In& operator>>(In& stream, PlayerInfo& playerInfo)
{
  STREAM_EXT(stream, playerInfo.penalty);
  STREAM_EXT(stream, playerInfo.secsTillUnpenalised);
  return stream;
}

TeamInfo::TeamInfo()
{
  memset(static_cast<RoboCup::TeamInfo*>(this), 0, sizeof(RoboCup::TeamInfo));
}

int TeamInfo::getSubstitutedPlayerNumber(int number) const
{
  if(number < 6)
    return number;

  for(unsigned int i = 0; i < 5; i++)
    if(players[i].penalty == PENALTY_SUBSTITUTE)
      return i + 1;

  return number;
}

void TeamInfo::read(In& stream)
{
  PlayerInfo(&players)[MAX_NUM_PLAYERS] = reinterpret_cast<PlayerInfo(&)[MAX_NUM_PLAYERS]>(this->players);

  STREAM(teamNumber); // unique team number
  STREAM(fieldPlayerColour); // TEAM_BLUE, TEAM_RED, TEAM_YELLOW, TEAM_BLACK, ...
  STREAM(goalkeeperColour); // TEAM_BLUE, TEAM_RED, TEAM_YELLOW, TEAM_BLACK, ...
  STREAM(goalkeeper); // which robot is the goal keeper (i.e. allowed to dive)
  STREAM(score); // team's score
  STREAM(players); // the team's players
  STREAM(messageBudget); // the team's remaining messages
}

void TeamInfo::write(Out& stream) const
{
  const PlayerInfo(&players)[MAX_NUM_PLAYERS] = reinterpret_cast<const PlayerInfo(&)[MAX_NUM_PLAYERS]>(this->players);

  STREAM(teamNumber); // unique team number
  STREAM(fieldPlayerColour); // TEAM_BLUE, TEAM_RED, TEAM_YELLOW, TEAM_BLACK, ...
  STREAM(goalkeeperColour); // TEAM_BLUE, TEAM_RED, TEAM_YELLOW, TEAM_BLACK, ...
  STREAM(goalkeeper); // which robot is the goal keeper (i.e. allowed to dive)
  STREAM(score); // team's score
  STREAM(players); // the team's players
  STREAM(messageBudget); // the team's remaining messages
}

void TeamInfo::reg()
{
  PUBLISH(reg);
  REG_CLASS(TeamInfo);
  REG(teamNumber);
  REG(fieldPlayerColour);
  REG(goalkeeperColour); 
  REG(goalkeeper); 
  REG(score);
  REG(PlayerInfo(&)[MAX_NUM_PLAYERS], players);
  REG(messageBudget);
}

static void drawDigit(int digit, const Vector3f& pos, float size, int fieldPlayerColour)
{
  static const Vector3f points[8] =
  {
    Vector3f(1, 0, 1),
    Vector3f(1, 0, 0),
    Vector3f(0, 0, 0),
    Vector3f(0, 0, 1),
    Vector3f(0, 0, 2),
    Vector3f(1, 0, 2),
    Vector3f(1, 0, 1),
    Vector3f(0, 0, 1)
  };
  static const unsigned char digits[10] =
  {
    0x3f,
    0x0c,
    0x76,
    0x5e,
    0x4d,
    0x5b,
    0x7b,
    0x0e,
    0x7f,
    0x5f
  };

  digit = digits[std::abs(digit)];
  for(int i = 0; i < 7; ++i)
    if(digit & (1 << i))
    {
      Vector3f from = pos - points[i] * size;
      Vector3f to = pos - points[i + 1] * size;
      LINE3D("representation:TeamInfo", from.x(), from.y(), from.z(), to.x(), to.y(), to.z(), 2, ColorRGBA::fromTeamColor(fieldPlayerColour));
    }
}

void TeamInfo::draw() const
{
  DECLARE_DEBUG_DRAWING3D("representation:TeamInfo", "field");
  {
    float yPosLeftSideline = 3000.f;
    if(Blackboard::getInstance().exists("FieldDimensions"))
      yPosLeftSideline = static_cast<const FieldDimensions&>(Blackboard::getInstance()["FieldDimensions"]).yPosLeftSideline;
    yPosLeftSideline += 500.f;
    const float x = teamNumber == 1 ? -1535.f : 1465.f;
    const int score = std::min(static_cast<int>(this->score), 99);
    drawDigit(score / 10, Vector3f(x, yPosLeftSideline, 1000), 200, fieldPlayerColour);
    drawDigit(score % 10, Vector3f(x + 270, yPosLeftSideline, 1000), 200, fieldPlayerColour);
  };
}

OwnTeamInfo::OwnTeamInfo()
{
  fieldPlayerColour = Global::settingsExist() ? Global::getSettings().fieldPlayerColour : TEAM_RED;
  goalkeeperColour = Global::settingsExist() ? Global::getSettings().goalKeeperColour : TEAM_BLACK;
  goalkeeper = 1;
}

void OwnTeamInfo::draw() const
{
  //do base struct drawing first.
  TeamInfo::draw();

  DEBUG_DRAWING("representation:OwnTeamInfo", "drawingOnField")
  {
    float xPosOwnFieldBorder = -5200.f;
    float yPosRightFieldBorder = -3700;
    if(Blackboard::getInstance().exists("FieldDimensions"))
    {
      const FieldDimensions& theFieldDimensions = static_cast<const FieldDimensions&>(Blackboard::getInstance()["FieldDimensions"]);
      xPosOwnFieldBorder = theFieldDimensions.xPosOwnFieldBorder;
      yPosRightFieldBorder = theFieldDimensions.yPosRightFieldBorder;
    }
    DRAW_TEXT("representation:OwnTeamInfo", xPosOwnFieldBorder + 200, yPosRightFieldBorder - 100, (xPosOwnFieldBorder / -5200.f) * 140, ColorRGBA::red, "Team color: " << TypeRegistry::getEnumName((Settings::TeamColor) fieldPlayerColour));
  }
}

OpponentTeamInfo::OpponentTeamInfo()
{
  fieldPlayerColour = ((Global::settingsExist() ? Global::getSettings().fieldPlayerColour : TEAM_RED) + 5) % TEAM_GRAY;
  goalkeeperColour = ((Global::settingsExist() ? Global::getSettings().goalKeeperColour : TEAM_BLACK) + 5) % TEAM_GRAY;
  goalkeeper = 1;
}
