#include "DefaultPoseProvider.h"
#include "Tools/Debugging/DebugDrawings3D.h"



MAKE_MODULE(DefaultPoseProvider, behaviorControl);

void DefaultPoseProvider::update(DefaultPose& defaultPose)
{
	/* 
  opp kick off: theGameInfo.kickingTeam != theOwnTeamInfo.teamNumber && theGameInfo.state == STATE_READY
  own kick off: theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber && theGameInfo.state == STATE_READY 
  normal game: STATE_PLAYING                       3
  */

	// generic assignments according to field dimensi
	defaultPose.goalieDefaultPosition = calcAbsoluteFromMargin(goalieMargin);
	defaultPose.leftDefensePosition   = calcAbsoluteFromMargin(leftDefenseMargin);
	defaultPose.rightDefensePosition  = calcAbsoluteFromMargin(rightDefenseMargin);
	defaultPose.leftOffensePosition   = calcAbsoluteFromMargin(leftOffenseMargin);
	defaultPose.rightOffensePosition  = calcAbsoluteFromMargin(rightOffenseMargin);

	// normal game
	// need to take care that one player is goalie (resp. subsitute)
	if(theGameInfo.state == STATE_PLAYING)
	{	/*
		if(theTeamBehaviorStatus.teammateRoles.roles[theRobotInfo.number - 1] == TeammateRoles::GOALKEEPER)
			defaultPose.defaultPosition = defaultPose.getDefaultPosition(1);  // get goalie position; applies for goalie substitute as well
		else
			defaultPose.defaultPosition = defaultPose.getDefaultPosition(theRobotInfo.number);  //generic mapping, team strategy is ignored yet
		*/

		defaultPose.defaultPosition = defaultPose.getDefaultPosition(theRobotInfo.number);

		const Vector2f strikerPose = { -300.0f, 0.0f };     // standing in front of the ball, player number 5
		const Vector2f supportPos  = { -1200.0f, -400.0f };  // right of (ie lower) from center, player number 4
		
		const Vector2f sup0Pos = { -3500.0f, -800.0f };     // player number 2
		const Vector2f sup1Pos  = { -2000.0f, 600.0f};  // right of (ie lower) from center, palyer number 3
		if(theRobotInfo.number == 5) defaultPose.defaultPosition = strikerPose;
		if(theRobotInfo.number == 4) defaultPose.defaultPosition = supportPos;
		if(theRobotInfo.number == 3) defaultPose.defaultPosition = sup1Pos;
		if(theRobotInfo.number == 2) defaultPose.defaultPosition = sup0Pos;

	}

	// own kick off
	// need to take care of two players, if feasable
	if(theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber && theGameInfo.state == STATE_READY)
	{

		// OUTPUT_TEXT("using own kick off positions");
		defaultPose.defaultPosition = defaultPose.getDefaultPosition(theRobotInfo.number);

		const Vector2f strikerPose = { -300.0f, 0.0f };     // standing in front of the ball, player number 5
		const Vector2f supportPos  = { -1200.0f, -400.0f };  // right of (ie lower) from center, player number 4
		const Vector2f sup0Pos = { -3500.0f, -800.0f };     // player number 2		
		const Vector2f sup1Pos  = { -2200.0f, 600.0f};  // right of (ie lower) from center, palyer number 3

		if(theTeamBehaviorStatus.teamActivity == TeamBehaviorStatus::R2K_NORMAL_GAME 
			|| theTeamBehaviorStatus.teamActivity == TeamBehaviorStatus::R2K_OFFENSIVE_GAME)
		{  // we have 2 offense;
			if(theRobotInfo.number == 5) defaultPose.defaultPosition = strikerPose;
			if(theRobotInfo.number == 4) defaultPose.defaultPosition = supportPos;
			if(theRobotInfo.number == 3) defaultPose.defaultPosition = sup1Pos;
			if(theRobotInfo.number == 2) defaultPose.defaultPosition = sup0Pos;
		}

		// only one bot for kick-off
		if(theTeamBehaviorStatus.teamActivity == TeamBehaviorStatus::R2K_DEFENSIVE_GAME)
		{
			if(theTeamBehaviorStatus.role.ballPlayer) defaultPose.defaultPosition = strikerPose;
		}
	}

  // opp kick off
	// need to take care of one or two OFFENSE players (the number is calculated in R2KTeamCard, according to team strategy
  // ReadyOppKickoffCard.cpp checks wether theTeamBehaviorStatus.isOffense(theRobotInfo.number);
	if(theGameInfo.kickingTeam != theOwnTeamInfo.teamNumber && theGameInfo.state == STATE_READY)
	{
		defaultPose.defaultPosition = defaultPose.getDefaultPosition(theRobotInfo.number);

		const Vector2f strikerPose = { -500.0f, 800.0f };     // standing in front of the ball, player number 5
		const Vector2f supportPos  = { -1500.0f, -600.0f };  // right of (ie lower) from center, player number 4
		
		const Vector2f sup0Pos = { -3500.0f, -800.0f };     // player number 2		
		const Vector2f sup1Pos  = { -2000.0f, 600.0f};  // right of (ie lower) from center, palyer number 3

		if(theTeamBehaviorStatus.teamActivity == TeamBehaviorStatus::R2K_NORMAL_GAME)
		{  // we have 2 offense;
			if(theRobotInfo.number == 5) defaultPose.defaultPosition = strikerPose;
			if(theRobotInfo.number == 4) defaultPose.defaultPosition = supportPos;
			if(theRobotInfo.number == 3) defaultPose.defaultPosition = sup1Pos;
			if(theRobotInfo.number == 2) defaultPose.defaultPosition = sup0Pos;
		}

		// only one bot for kick-off
		if(theTeamBehaviorStatus.teamActivity == TeamBehaviorStatus::R2K_DEFENSIVE_GAME)
		{
			if(theTeamBehaviorStatus.role.ballPlayer) defaultPose.defaultPosition = strikerPose;
		}
	}

  
	// 				Geometry::distance(theRobotPose.translation, theDefaultPose.defaultPosition) < distanceThreshold 
  //        &&std::abs(theRobotPose.rotation - 0.0f) < angleThreshold
  
  defaultPose.isOnDefaultPosition = (Geometry::distance(theRobotPose.translation, defaultPose.defaultPosition) <= 300.0f);
  defaultPose.isOnDefaultPose = defaultPose.isOnDefaultPosition && std::abs(theRobotPose.rotation) < 10_deg;

  drawDefaultPositions();
}

Vector2f DefaultPoseProvider::calcAbsoluteFromMargin(const Vector2f &margin)
{
  ASSERT(margin.x() >= 0.0f && margin.x() <= 1.0f);
  ASSERT(margin.y() >= 0.0f && margin.y() <= 1.0f);

  const float width = std::abs(theFieldDimensions.xPosOpponentGroundLine - theFieldDimensions.xPosOwnGroundLine);
  const float height = std::abs(theFieldDimensions.yPosRightSideline - theFieldDimensions.yPosLeftSideline);

  const float x = (theFieldDimensions.xPosOwnGroundLine + width * margin.x());
  const float y = (theFieldDimensions.yPosLeftSideline - height * margin.y());

  return Vector2f(x, y);
}

void DefaultPoseProvider::drawDefaultPositions()
{
  DEBUG_DRAWING3D("test_defaultPos", "field")
  {
    const auto goal = calcAbsoluteFromMargin(goalieMargin);
    const auto ldef = calcAbsoluteFromMargin(leftDefenseMargin);
    const auto rdef = calcAbsoluteFromMargin(rightDefenseMargin);
    const auto loff = calcAbsoluteFromMargin(leftOffenseMargin);
    const auto roff = calcAbsoluteFromMargin(rightOffenseMargin);

    SPHERE3D("test_defaultPos", goal.x(), goal.y(), 5, 75, ColorRGBA::red);
    SPHERE3D("test_defaultPos", ldef.x(), ldef.y(), 5, 75, ColorRGBA::yellow);
    SPHERE3D("test_defaultPos", rdef.x(), rdef.y(), 5, 75, ColorRGBA::blue);
    SPHERE3D("test_defaultPos", loff.x(), loff.y(), 5, 75, ColorRGBA::cyan);
    SPHERE3D("test_defaultPos", roff.x(), roff.y(), 5, 75, ColorRGBA::orange);
  }  
}