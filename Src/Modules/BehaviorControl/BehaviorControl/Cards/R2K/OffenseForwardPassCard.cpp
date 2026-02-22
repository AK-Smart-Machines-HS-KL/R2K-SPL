/**
 * @file OffenseForwardPassCard.cpp
 * @author Niklas Schmidts, Adrian Müller
 * @version 1.2
 *
 * OpenPoints status:
 * - preconditions are now deterministic and require a valid receiver
 * - explicit pass intent/abort signaling is implemented via passTarget + R2KLOG events
 */

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Modeling/RobotPose.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Representations/Communication/TeamData.h"
#include "Tools/BehaviorControl/R2KDecisionLog.h"
#include "Tools/BehaviorControl/R2KPassLogic.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include <string>

#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/TeamCommStatus.h"
#include "Representations/Infrastructure/ExtendedGameInfo.h"

CARD(OffenseForwardPassCard,
     {
    ,
    CALLS(Activity),
    CALLS(GoToBallAndKick),
    CALLS(PassTarget),
    REQUIRES(RobotPose),
    REQUIRES(TeamData),
    REQUIRES(TeammateRoles),
    REQUIRES(PlayerRole),
    REQUIRES(RobotInfo),
    REQUIRES(TeamCommStatus),
    REQUIRES(ExtendedGameInfo),
    REQUIRES(FrameInfo),

    DEFINES_PARAMETERS(
    {,
      (int)(1200) minForwardDistanceMm,
      (int)(400) intentRefreshMs,
      (int)(2500) targetLeadMm,
    }),
});

class OffenseForwardPassCard : public OffenseForwardPassCardBase
{
  Vector2f targetAbsolute = Vector2f::Zero();
  int targetMate = -1;
  unsigned lastIntentTs = 0;

  bool preconditions() const override
  {
    if(aBuddyIsClearingOrPassing())
      return false;

    if(!theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive) ||
       !theTeammateRoles.isTacticalOffense(theRobotInfo.number))
      return false;

    if(!(thePlayerRole.supporterIndex() == thePlayerRole.numOfActiveSupporters - 1 ||
         theExtendedGameInfo.timeSincePlayingStarted < 10000))
      return false;

    return selectReceiver(nullptr, nullptr);
  }

  bool postconditions() const override
  {
    return !preconditions();
  }

  bool selectReceiver(int* receiverNumber, Vector2f* receiverTarget) const
  {
    const auto candidate = R2KPassLogic::selectForwardReceiver(theTeamData.teammates,
                                                               theRobotPose.translation,
                                                               static_cast<float>(minForwardDistanceMm));
    if(!candidate.has_value())
      return false;

    if(receiverNumber)
      *receiverNumber = candidate->number;

    if(receiverTarget)
    {
      *receiverTarget = candidate->pose;
      receiverTarget->x() += targetLeadMm;
    }

    return true;
  }

  void execute() override
  {
    int selectedTarget = -1;
    Vector2f selectedTargetPosition = Vector2f::Zero();
    if(!selectReceiver(&selectedTarget, &selectedTargetPosition))
    {
      thePassTargetSkill(-1);
      return;
    }

    targetMate = selectedTarget;
    targetAbsolute = selectedTargetPosition;

    theActivitySkill(BehaviorStatus::offenseForwardPassCard);
    thePassTargetSkill(targetMate, targetAbsolute);

    if(lastIntentTs == 0 || theFrameInfo.getTimeSince(lastIntentTs) > static_cast<unsigned>(intentRefreshMs))
    {
      lastIntentTs = theFrameInfo.time;
      R2KDecisionLog::annotation("pass_intent", {{"card", "OffenseForwardPass"},
                                             {"passer", std::to_string(theRobotInfo.number)},
                                             {"target", std::to_string(targetMate)},
                                             {"ts", std::to_string(static_cast<int>(lastIntentTs))},
                                             {"targetX", std::to_string(static_cast<int>(targetAbsolute.x()))},
                                             {"targetY", std::to_string(static_cast<int>(targetAbsolute.y()))}});
    }

    theGoToBallAndKickSkill(theRobotPose.toRelative(targetAbsolute).angle(), KickInfo::forwardFastLeft);
  }

  void reset() override
  {
    targetAbsolute = Vector2f::Zero();
    targetMate = -1;
    lastIntentTs = 0;
    thePassTargetSkill(-1);
    R2KDecisionLog::annotation("pass_abort", {{"card", "OffenseForwardPass"},
                                           {"passer", std::to_string(theRobotInfo.number)},
                                           {"reason", "card_reset"}});
  }

  bool aBuddyIsClearingOrPassing() const
  {
    for(const auto& buddy : theTeamData.teammates)
    {
      if(buddy.theBehaviorStatus.activity == BehaviorStatus::defenseLongShotCard ||
         buddy.theBehaviorStatus.activity == BehaviorStatus::goalieLongShotCard ||
         buddy.theBehaviorStatus.activity == BehaviorStatus::goalShotCard ||
         buddy.theBehaviorStatus.activity == BehaviorStatus::offenseForwardPassCard)
        return true;
    }
    return false;
  }
};

MAKE_CARD(OffenseForwardPassCard);
