#pragma once

namespace R2KAttackLogic
{
  enum class ShotExecutionMode
  {
    preciseKick,
    safeKick,
    hold
  };

  enum class DecisionReason
  {
    ready,
    ballLost,
    localizationPoor,
    farFromGoal
  };

  struct ShotDecision
  {
    ShotExecutionMode mode = ShotExecutionMode::hold;
    DecisionReason reason = DecisionReason::ballLost;
  };

  ShotDecision decideShotExecution(bool ballSeenRecently,
                                   bool localizationPoor,
                                   float distanceToGoal,
                                   float preciseKickDistanceMm);

  const char* toString(DecisionReason reason);
}
