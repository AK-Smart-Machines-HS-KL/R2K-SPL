#include "R2KAttackLogic.h"

namespace R2KAttackLogic
{
  ShotDecision decideShotExecution(const bool ballSeenRecently,
                                   const bool localizationPoor,
                                   const float distanceToGoal,
                                   const float preciseKickDistanceMm)
  {
    if(!ballSeenRecently)
      return {ShotExecutionMode::hold, DecisionReason::ballLost};

    if(localizationPoor)
      return {ShotExecutionMode::safeKick, DecisionReason::localizationPoor};

    if(distanceToGoal <= preciseKickDistanceMm)
      return {ShotExecutionMode::preciseKick, DecisionReason::ready};

    return {ShotExecutionMode::safeKick, DecisionReason::farFromGoal};
  }

  const char* toString(const DecisionReason reason)
  {
    switch(reason)
    {
      case DecisionReason::ready: return "ready";
      case DecisionReason::ballLost: return "ballLost";
      case DecisionReason::localizationPoor: return "localizationPoor";
      case DecisionReason::farFromGoal: return "farFromGoal";
      default: return "unknown";
    }
  }
}
