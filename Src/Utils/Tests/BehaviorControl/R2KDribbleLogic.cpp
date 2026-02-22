#include "Tools/BehaviorControl/R2KDribbleLogic.h"

#include "gtest/gtest.h"

GTEST_TEST(R2KDribbleLogic, RecoverWhenBallUnseen)
{
  const auto decision = R2KDribbleLogic::decide(false, false, 0.f, 200.f);
  EXPECT_EQ(decision.mode, R2KDribbleLogic::DribbleMode::recover);
  EXPECT_EQ(decision.reason, R2KDribbleLogic::DribbleReason::ballUncertain);
}

GTEST_TEST(R2KDribbleLogic, CautiousWhenLocalizationPoor)
{
  const auto decision = R2KDribbleLogic::decide(true, true, 10.f, 200.f);
  EXPECT_EQ(decision.mode, R2KDribbleLogic::DribbleMode::cautiousAdvance);
  EXPECT_EQ(decision.reason, R2KDribbleLogic::DribbleReason::localizationPoor);
}

GTEST_TEST(R2KDribbleLogic, CautiousWhenBallTravelEstimateHigh)
{
  const auto decision = R2KDribbleLogic::decide(true, false, 300.f, 200.f);
  EXPECT_EQ(decision.mode, R2KDribbleLogic::DribbleMode::cautiousAdvance);
  EXPECT_EQ(decision.reason, R2KDribbleLogic::DribbleReason::ballUncertain);
}

GTEST_TEST(R2KDribbleLogic, AdvanceWhenStable)
{
  const auto decision = R2KDribbleLogic::decide(true, false, 50.f, 200.f);
  EXPECT_EQ(decision.mode, R2KDribbleLogic::DribbleMode::advance);
  EXPECT_EQ(decision.reason, R2KDribbleLogic::DribbleReason::stable);
}
