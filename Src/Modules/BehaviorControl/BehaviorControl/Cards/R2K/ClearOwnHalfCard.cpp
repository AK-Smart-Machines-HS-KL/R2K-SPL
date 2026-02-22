/**
 * @file ClearOwnHalfCard.cpp
 * @author Adrian Müller
 * @version 1.5
 *
 * OpenPoints status:
 * - shooting direction and exit behavior are harmonized with shared clear-shot logic.
 */

#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Modeling/ObstacleModel.h"

#include "Representations/Modeling/RobotPose.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Tools/Math/BHMath.h"

#include "Representations/BehaviorControl/TeamBehaviorStatus.h"
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamData.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Tools/BehaviorControl/R2KClearShotLogic.h"

CARD(ClearOwnHalfCard,
  { ,
    CALLS(Activity),
    CALLS(GoToBallAndKick),
    USES(GameInfo),
    REQUIRES(FieldBall),
    REQUIRES(FieldDimensions),
    REQUIRES(FrameInfo),
    REQUIRES(ObstacleModel),
    REQUIRES(RobotInfo),
    REQUIRES(RobotPose),
    REQUIRES(TeamBehaviorStatus),
    REQUIRES(TeammateRoles),

    DEFINES_PARAMETERS(
    {,
      (bool)(false) footIsSelected,
      (bool)(true) leftFoot,
      (unsigned)(0) kickStartTime,
      (int)(1200) minOpponentDistanceMm,
      (int)(500) emergencyOpponentDistanceMm,
      (float)(30.f) shotDirectionScanAngleDeg,
      (int)(2500) postKickExitTimeoutMs,
    }),
  });

class ClearOwnHalfCard : public ClearOwnHalfCardBase
{
  bool preconditions() const override
  {
    return theGameInfo.setPlay == SET_PLAY_NONE &&
           theTeammateRoles.playsTheBall(theRobotInfo.number) &&
           theTeammateRoles.isTacticalDefense(theRobotInfo.number) &&
           theFieldBall.positionOnField.x() < -500 &&
           theObstacleModel.opponentIsClose(minOpponentDistanceMm) &&
           theTeamBehaviorStatus.teamActivity != TeamBehaviorStatus::R2K_SPARSE_GAME;
  }

  bool postconditions() const override
  {
    return !preconditions() ||
           theObstacleModel.opponentIsClose(emergencyOpponentDistanceMm) ||
           theGoToBallAndKickSkill.isDone() ||
           (kickStartTime != 0 && theFrameInfo.getTimeSince(kickStartTime) > static_cast<unsigned>(postKickExitTimeoutMs));
  }

  void execute() override
  {
    theActivitySkill(BehaviorStatus::clearOwnHalfCard);

    if(!footIsSelected)
    {
      footIsSelected = true;
      kickStartTime = theFrameInfo.time;
      leftFoot = theFieldBall.positionRelative.y() < 0;
    }

    const Angle clearAngle = R2KClearShotLogic::chooseClearAngleToGoal(theObstacleModel,
                                                                        theRobotPose,
                                                                        theFieldDimensions,
                                                                        shotDirectionScanAngleDeg);

    theGoToBallAndKickSkill(clearAngle, leftFoot ? KickInfo::walkForwardsLeft : KickInfo::walkForwardsRight);
  }
};

MAKE_CARD(ClearOwnHalfCard);
