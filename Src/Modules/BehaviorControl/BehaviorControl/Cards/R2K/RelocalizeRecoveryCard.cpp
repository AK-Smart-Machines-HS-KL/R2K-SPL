/**
 * @file RelocalizeRecoveryCard.cpp
 * @brief Recovery card for unstable self-localization.
 */

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Modeling/RobotPose.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Tools/BehaviorControl/R2KDecisionLog.h"
#include <string>

CARD(RelocalizeRecoveryCard,
     {,
      CALLS(Activity),
      CALLS(LookActive),
      CALLS(Stand),
      CALLS(WalkAtRelativeSpeed),
      REQUIRES(GameInfo),
      REQUIRES(TeammateRoles),
      REQUIRES(RobotInfo),
      REQUIRES(RobotPose),
      DEFINES_PARAMETERS(
      {,
       (int)(1200) standScanDuration,
       (int)(1500) turnDuration,
      }),
     });

class RelocalizeRecoveryCard : public RelocalizeRecoveryCardBase
{
  bool preconditions() const override
  {
    return theGameInfo.state == STATE_PLAYING &&
           theRobotPose.quality == RobotPose::poor &&
           !theTeammateRoles.isTacticalGoalKeeper(theRobotInfo.number);
  }

  bool postconditions() const override
  {
    return !preconditions();
  }

  option
  {
    theActivitySkill(BehaviorStatus::searchForBall);

    initial_state(scan)
    {
      transition
      {
        if(state_time > standScanDuration)
          goto turn;
      }

      action
      {
        if(state_time == 0)
          R2KDecisionLog::annotation("relocalize_state", {{"card", "RelocalizeRecovery"}, {"state", "scan"}, {"player", std::to_string(theRobotInfo.number)}});
        theLookActiveSkill(false, true, false, false);
        theStandSkill();
      }
    }

    state(turn)
    {
      transition
      {
        if(state_time > turnDuration)
          goto scan;
      }

      action
      {
        if(state_time == 0)
          R2KDecisionLog::annotation("relocalize_state", {{"card", "RelocalizeRecovery"}, {"state", "turn"}, {"player", std::to_string(theRobotInfo.number)}});
        theLookActiveSkill(false, true, false, false);
        const float turnDirection = theRobotPose.translation.y() >= 0.f ? -1.4f : 1.4f;
        theWalkAtRelativeSpeedSkill(Pose2f(turnDirection, 0.f, 0.f));
      }
    }
  }
};

MAKE_CARD(RelocalizeRecoveryCard);
