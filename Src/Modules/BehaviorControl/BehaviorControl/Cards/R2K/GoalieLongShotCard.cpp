/**
 * @file GoalieLongShotCard.cpp
 * @author Nicholas Pfohlmann, Adrian Müller
 * @version 1.4
 *
 * OpenPoints status:
 * - free shot direction and post-kick timeout handling are implemented.
 */

#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Modeling/ObstacleModel.h"

#include "Representations/Modeling/RobotPose.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Tools/Math/BHMath.h"

#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/Communication/RobotInfo.h"
#include "Tools/BehaviorControl/R2KClearShotLogic.h"

CARD(GoalieLongShotCard,
{ ,
  CALLS(Activity),
  CALLS(GoToBallAndKick),
  REQUIRES(FieldBall),
  REQUIRES(FieldDimensions),
  REQUIRES(FrameInfo),
  REQUIRES(ObstacleModel),
  REQUIRES(RobotInfo),
  REQUIRES(RobotPose),
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

class GoalieLongShotCard : public GoalieLongShotCardBase
{
  bool preconditions() const override
  {
    return theTeammateRoles.playsTheBall(theRobotInfo.number) &&
           !theObstacleModel.opponentIsClose(minOpponentDistanceMm) &&
           theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number) &&
           theFieldBall.positionOnField.x() <= theFieldDimensions.xPosOwnPenaltyMark &&
           theFieldBall.positionOnField.x() >= theFieldDimensions.xPosOwnGroundLine;
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
    theActivitySkill(BehaviorStatus::goalieLongShotCard);

    if(!footIsSelected)
    {
      footIsSelected = true;
      kickStartTime = theFrameInfo.time;
      leftFoot = theFieldBall.positionRelative.y() < 0;
    }

    const KickInfo::KickType kickType = leftFoot ? KickInfo::forwardFastLeftLong : KickInfo::forwardFastRightLong;
    const Angle clearAngle = R2KClearShotLogic::chooseClearAngleToGoal(theObstacleModel,
                                                                        theRobotPose,
                                                                        theFieldDimensions,
                                                                        shotDirectionScanAngleDeg);

    switch(theObstacleModel.opponentIsTooClose(theFieldBall.positionRelative))
    {
      case KickInfo::LongShotType::fast: theGoToBallAndKickSkill(clearAngle, kickType, false); break;
      case KickInfo::LongShotType::precise: theGoToBallAndKickSkill(clearAngle, kickType, true); break;
      default: theGoToBallAndKickSkill(clearAngle, kickType); break;
    }
  }
};

MAKE_CARD(GoalieLongShotCard);
