#include "Tools/BehaviorControl/R2KAttackLogic.h"

#include "gtest/gtest.h"

GTEST_TEST(R2KAttackLogic, HoldWhenBallWasNotSeen)
{
  const auto result = R2KAttackLogic::decideShotExecution(false, false, 500.f, 2500.f);
  EXPECT_EQ(result.mode, R2KAttackLogic::ShotExecutionMode::hold);
  EXPECT_EQ(result.reason, R2KAttackLogic::DecisionReason::ballLost);
}

GTEST_TEST(R2KAttackLogic, SafeKickWhenLocalizationPoor)
{
  const auto result = R2KAttackLogic::decideShotExecution(true, true, 300.f, 2500.f);
  EXPECT_EQ(result.mode, R2KAttackLogic::ShotExecutionMode::safeKick);
  EXPECT_EQ(result.reason, R2KAttackLogic::DecisionReason::localizationPoor);
}

GTEST_TEST(R2KAttackLogic, PreciseKickWhenReadyAndNearGoal)
{
  const auto result = R2KAttackLogic::decideShotExecution(true, false, 800.f, 2500.f);
  EXPECT_EQ(result.mode, R2KAttackLogic::ShotExecutionMode::preciseKick);
  EXPECT_EQ(result.reason, R2KAttackLogic::DecisionReason::ready);
}

GTEST_TEST(R2KAttackLogic, SafeKickWhenFarFromGoal)
{
  const auto result = R2KAttackLogic::decideShotExecution(true, false, 3500.f, 2500.f);
  EXPECT_EQ(result.mode, R2KAttackLogic::ShotExecutionMode::safeKick);
  EXPECT_EQ(result.reason, R2KAttackLogic::DecisionReason::farFromGoal);
}

GTEST_TEST(R2KAttackLogic, ReasonToStringIsStable)
{
  EXPECT_STREQ(R2KAttackLogic::toString(R2KAttackLogic::DecisionReason::ready), "ready");
  EXPECT_STREQ(R2KAttackLogic::toString(R2KAttackLogic::DecisionReason::ballLost), "ballLost");
  EXPECT_STREQ(R2KAttackLogic::toString(R2KAttackLogic::DecisionReason::localizationPoor), "localizationPoor");
  EXPECT_STREQ(R2KAttackLogic::toString(R2KAttackLogic::DecisionReason::farFromGoal), "farFromGoal");
}

GTEST_TEST(R2KAttackLogic, LocalizationPoorHasPriorityOverDistance)
{
  const auto result = R2KAttackLogic::decideShotExecution(true, true, 500.f, 2500.f);
  EXPECT_EQ(result.mode, R2KAttackLogic::ShotExecutionMode::safeKick);
  EXPECT_EQ(result.reason, R2KAttackLogic::DecisionReason::localizationPoor);
}
