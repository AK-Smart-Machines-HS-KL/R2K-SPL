/**
 * @file DefenseLongShotCard.cpp
 * @author Nicholas Pfohlmann, Adrian Müller 
 * @version: 1.2
 *
 * Functions, values, side effects: 
 * - this card qualifies for the one DEFENSE player only who is closest to the ball
 * - it will take its time to make a long kick if no opppenent is too close 
 * - it will make a faster, more unprecise version of the kick if opponent is closer
 *
 * Details: 
 * Purpose of this card is to clear the own field by a long shot.
 * - opponent is not to close (otherwise see ClearOwnHalf) and my distance to ball < opponents distance to ball
 * - isPlayBall()
 * - long shot to opponent goal, using correct foot: 
 *   - left or right is computed once, when the card is called for the first time
 *   - reason: while approaching the ball, the relative y-position might vary (for only a few centimeters),
 *             causing the skill to "freeze"
 * - defense will not leave own half, unless we are in OFFENSIVE or SPARSE Mode
 * 
 * 
 * v1.1: added kick variations (precise and unprecise) and now using opponent-ball as distance
 *       and check if I am closer to ball than opponent
 * V1.2: uses new enum LongShotType in KickInfo.h for precise and fast longshot
 * 
 * After kick:
 * - a) card exists if role no longer is playsTheBall()
 * - b) exit after kick (isDone()) to avoid defenser player chase the ball 
 * 
 * Note: 
 * - because this is a long shot, the flag "playsTheBall" typicall is re-set after the shot, as a side effect.
 * - However, if the ball is stuck, the flag may still be set, and the player will follow the ball
 * - This behavior is ok in OFFENSIVE and SPARSE mode
 * 
 *  v.1.3 precond: x < 0 - threshold. 
 *      Activated !aBuddyIsClearingOwnHalf
 *  v.1.4 Added the online & offline role assignment(Asrar)
 * OpenPoints status:
 * - max opponent distance thresholds are now configurable parameters
 * - free shoot-vector optimization and post-kick isDone() tuning remain future work
 */


// B-Human includes
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Modeling/ObstacleModel.h"
#include "Representations/Communication/TeamData.h"
#include "Representations/Infrastructure/FrameInfo.h"

#include "Representations/Modeling/RobotPose.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Tools/Math/BHMath.h"
#include "Tools/BehaviorControl/R2KClearShotLogic.h"

// this is the R2K specific stuff
#include "Representations/BehaviorControl/TeamBehaviorStatus.h" 
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/TeamCommStatus.h"

CARD(DefenseLongShotCard,
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
    REQUIRES(TeamData),
    REQUIRES(TeammateRoles),  // R2K
    REQUIRES(TeamCommStatus),  // wifi on off?

    DEFINES_PARAMETERS(
    {,
      (bool)(false) footIsSelected,  // freeze the first decision
      (bool)(true) leftFoot,
      (int)(-1000) offsetX,
      (int)(1200) minOpponentDistanceMm,
      (int)(500) emergencyOpponentDistanceMm,
      (float)(30.f) shotDirectionScanAngleDeg,
      (int)(2500) postKickExitTimeoutMs,
      (unsigned)(0) kickStartTime,
    }),
  });

class DefenseLongShotCard : public DefenseLongShotCardBase
{
  bool preconditions() const override
  {
    
    return
      theTeammateRoles.playsTheBall(&theRobotInfo , theTeamCommStatus.isWifiCommActive) &&  // I am the striker
      !theObstacleModel.opponentIsClose(minOpponentDistanceMm) &&
      !aBuddyIsClearingOwnHalf() &&
      theTeammateRoles.isTacticalDefense(theRobotInfo.number) && // my recent role

      //don't leave own half, unless we are in OFFENSIVE or SPARSE Mode)
      (
        theTeamBehaviorStatus.teamActivity == TeamBehaviorStatus::R2K_OFFENSIVE_GAME ||
        theTeamBehaviorStatus.teamActivity == TeamBehaviorStatus::R2K_SPARSE_GAME ||
        theFieldBall.endPositionOnField.x() < 0
      );
  }

  bool postconditions() const override
  {
    return !preconditions() ||
           theObstacleModel.opponentIsClose(emergencyOpponentDistanceMm) ||
           !theTeammateRoles.isTacticalDefense(theRobotInfo.number) ||
           !(theFieldBall.endPositionOnField.x() < 200) ||
           theGoToBallAndKickSkill.isDone() ||
           (kickStartTime != 0 && theFrameInfo.getTimeSince(kickStartTime) > static_cast<unsigned>(postKickExitTimeoutMs));
  }

 
  void execute() override
  {

    theActivitySkill(BehaviorStatus::defenseLongShotCard);

    if (!footIsSelected) {  // select only once
      footIsSelected = true;
      kickStartTime = theFrameInfo.time;
      leftFoot = theFieldBall.positionRelative.y() < 0;
    }
    const KickInfo::KickType kickType = leftFoot ? KickInfo::forwardFastLeftLong : KickInfo::forwardFastRightLong;
    const Angle clearAngle = R2KClearShotLogic::chooseClearAngleToGoal(theObstacleModel,
                                                                        theRobotPose,
                                                                        theFieldDimensions,
                                                                        shotDirectionScanAngleDeg);

    switch (theObstacleModel.opponentIsTooClose(theFieldBall.positionRelative))
    {
      case(KickInfo::LongShotType::fast): theGoToBallAndKickSkill(clearAngle, kickType, false); break;
      case(KickInfo::LongShotType::precise): theGoToBallAndKickSkill(clearAngle, kickType, true); break;
      default: theGoToBallAndKickSkill(clearAngle, kickType); break;
    }
  }


  bool aBuddyIsClearingOwnHalf() const
  {
    for (const auto& buddy : theTeamData.teammates)
    {
      if (buddy.theBehaviorStatus.activity == BehaviorStatus::defenseChaseBallCard ||
        buddy.theBehaviorStatus.activity == BehaviorStatus::blocking ||
        buddy.theBehaviorStatus.activity == BehaviorStatus::clearOwnHalfCard ||
        buddy.theBehaviorStatus.activity == BehaviorStatus::clearOwnHalfGoalieCard ||
        buddy.theBehaviorStatus.activity == BehaviorStatus::defenseLongShotCard ||
        buddy.theBehaviorStatus.activity == BehaviorStatus::goalieLongShotCard)
        return true;
    }
    return false;
  }

};

MAKE_CARD(DefenseLongShotCard);
