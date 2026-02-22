/**
 * @file ClearOwnHalfGoalieCard.cpp
 * @author Adrian Müller
 * @version 1.5
 *
 * OpenPoints status:
 * - better shot direction and exit timeout are implemented.
 */

#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Modeling/ObstacleModel.h"

#include "Representations/Modeling/RobotPose.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Tools/Math/BHMath.h"

#include "Representations/BehaviorControl/TeamBehaviorStatus.h"
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/TeamCommStatus.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Tools/BehaviorControl/R2KClearShotLogic.h"

CARD(ClearOwnHalfGoalieCard,
  { ,
    CALLS(Activity),
    CALLS(GoToBallAndKick),
    REQUIRES(FieldBall),
    REQUIRES(FieldDimensions),
    REQUIRES(ObstacleModel),
    REQUIRES(FrameInfo),
    REQUIRES(RobotInfo),
    REQUIRES(RobotPose),
    REQUIRES(TeamBehaviorStatus),
    REQUIRES(TeammateRoles),
    REQUIRES(TeamCommStatus),

    DEFINES_PARAMETERS(
    {,
      (float)(500) maxDistanceFromGoalArea,
      (bool)(false) footIsSelected,
      (bool)(true) leftFoot,
      (unsigned)(0) kickStartTime,
      (int)(1200) minOpponentDistanceMm,
      (int)(500) emergencyOpponentDistanceMm,
      (float)(30.f) shotDirectionScanAngleDeg,
      (int)(2500) postKickExitTimeoutMs,
    }),
  });

class ClearOwnHalfGoalieCard : public ClearOwnHalfGoalieCardBase
{
  bool preconditions() const override
  {
    return theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive) &&
           theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number) &&
           theObstacleModel.opponentIsClose(minOpponentDistanceMm) &&
           theFieldBall.positionOnField.x() <= theFieldDimensions.xPosOwnGoalArea + maxDistanceFromGoalArea &&
           theFieldBall.positionOnField.y() <= theFieldDimensions.yPosLeftGoalArea + maxDistanceFromGoalArea &&
           theFieldBall.positionOnField.y() >= theFieldDimensions.yPosRightGoalArea - maxDistanceFromGoalArea &&
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
    theActivitySkill(BehaviorStatus::clearOwnHalfGoalieCard);

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

MAKE_CARD(ClearOwnHalfGoalieCard);
