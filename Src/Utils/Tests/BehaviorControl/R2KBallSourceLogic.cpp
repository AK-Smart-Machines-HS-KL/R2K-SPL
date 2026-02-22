#include "Tools/BehaviorControl/R2KBallSourceLogic.h"

#include "gtest/gtest.h"

GTEST_TEST(R2KBallSourceLogic, PrefersOwnBall)
{
  EXPECT_EQ(R2KBallSourceLogic::chooseBallSource(true, true, false), R2KBallSourceLogic::BallSource::own);
}

GTEST_TEST(R2KBallSourceLogic, UsesTeamWhenOwnMissing)
{
  EXPECT_EQ(R2KBallSourceLogic::chooseBallSource(false, true, false), R2KBallSourceLogic::BallSource::team);
}

GTEST_TEST(R2KBallSourceLogic, UnknownWithoutOwnAndTeam)
{
  EXPECT_EQ(R2KBallSourceLogic::chooseBallSource(false, false, false), R2KBallSourceLogic::BallSource::unknown);
}

GTEST_TEST(R2KBallSourceLogic, ToStringStable)
{
  EXPECT_STREQ(R2KBallSourceLogic::toString(R2KBallSourceLogic::BallSource::own), "own");
  EXPECT_STREQ(R2KBallSourceLogic::toString(R2KBallSourceLogic::BallSource::team), "team");
  EXPECT_STREQ(R2KBallSourceLogic::toString(R2KBallSourceLogic::BallSource::forecast), "forecast");
  EXPECT_STREQ(R2KBallSourceLogic::toString(R2KBallSourceLogic::BallSource::unknown), "unknown");
}


GTEST_TEST(R2KBallSourceLogic, ForecastWhenRequested)
{
  EXPECT_EQ(R2KBallSourceLogic::chooseBallSource(true, true, true), R2KBallSourceLogic::BallSource::forecast);
}
