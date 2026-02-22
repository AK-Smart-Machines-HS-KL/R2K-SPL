/**
 * @file OffenseReceivePassCard.cpp
 * @author Adrian Müller
 * @version 1.1
 *
 * OpenPoints status:
 * - receiver activation now checks explicit passTarget and freshness timeout.
 */

#include "Representations/BehaviorControl/Skills.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Representations/Communication/TeamData.h"
#include "Tools/BehaviorControl/R2KDecisionLog.h"
#include "Tools/BehaviorControl/R2KPassLogic.h"
#include <string>

#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/TeamCommStatus.h"
#include "Representations/Infrastructure/FrameInfo.h"

CARD(OffenseReceivePassCard,
     {
    ,
    CALLS(Activity),
    CALLS(LookActive),
    CALLS(Stand),
    REQUIRES(TeamData),
    REQUIRES(TeammateRoles),
    REQUIRES(PlayerRole),
    REQUIRES(RobotInfo),
    REQUIRES(TeamCommStatus),
    REQUIRES(FrameInfo),

    DEFINES_PARAMETERS(
    {,
      (int)(1500) intentTimeoutMs,
    }),
});

class OffenseReceivePassCard : public OffenseReceivePassCardBase
{
  bool preconditions() const override
  {
    return thePlayerRole.supporterIndex() == thePlayerRole.numOfActiveSupporters &&
           findValidPasser() != nullptr &&
           !theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive) &&
           theTeammateRoles.isTacticalOffense(theRobotInfo.number);
  }

  bool postconditions() const override
  {
    return !preconditions();
  }

  const Teammate* findValidPasser() const
  {
    return R2KPassLogic::findFreshPassIntent(theTeamData.teammates,
                                             theRobotInfo.number,
                                             theFrameInfo.time,
                                             static_cast<unsigned>(intentTimeoutMs));
  }

  void execute() override
  {
    const Teammate* passer = findValidPasser();
    if(!passer)
    {
      R2KDecisionLog::annotation("pass_abort", {{"card", "OffenseReceivePass"},
                                             {"receiver", std::to_string(theRobotInfo.number)},
                                             {"reason", "intent_timeout_or_missing"}});
      return;
    }

    R2KDecisionLog::annotation("pass_ack", {{"card", "OffenseReceivePass"},
                                        {"receiver", std::to_string(theRobotInfo.number)},
                                        {"passer", std::to_string(passer->number)},
                                        {"intentAgeMs", std::to_string(static_cast<int>(theFrameInfo.getTimeSince(passer->timeWhenLastPacketReceived)))}});

    theActivitySkill(BehaviorStatus::offenseReceivePassCard);
    theLookActiveSkill();
    theStandSkill();
  }
};

MAKE_CARD(OffenseReceivePassCard);
