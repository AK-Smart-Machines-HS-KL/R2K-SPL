#include "Tools/BehaviorControl/R2KTeamLogic.h"

#include "gtest/gtest.h"

GTEST_TEST(R2KTeamLogic, CountActivePlayers5v5UsesFirstFive)
{
  const std::vector<bool> penalized = {false, true, false, false, true, false, false};
  EXPECT_EQ(R2KTeamLogic::countActivePlayers5v5(penalized), 3);
}

GTEST_TEST(R2KTeamLogic, SparseModeTriggersByOpponentThreshold)
{
  EXPECT_TRUE(R2KTeamLogic::shouldUseSparseMode(5, 2, 3, 3));
  EXPECT_FALSE(R2KTeamLogic::shouldUseSparseMode(5, 4, 3, 3));
}

GTEST_TEST(R2KTeamLogic, SparseModeTriggersByTotalActiveThreshold)
{
  EXPECT_TRUE(R2KTeamLogic::shouldUseSparseMode(2, 1, 0, 3));
  EXPECT_FALSE(R2KTeamLogic::shouldUseSparseMode(2, 2, 0, 3));
}

GTEST_TEST(R2KTeamLogic, CountPlayersReturningSoonUsesFirstFive)
{
  const std::vector<int> secsTillUnpenalised = {0, 6, 0, 19, 21, 4, 3};
  EXPECT_EQ(R2KTeamLogic::countPlayersReturningSoon5v5(secsTillUnpenalised, 20), 2);
}

GTEST_TEST(R2KTeamLogic, CountPlayersReturningSoonIgnoresZeroAndLongTimeout)
{
  const std::vector<int> secsTillUnpenalised = {0, 0, 45, 22, 0};
  EXPECT_EQ(R2KTeamLogic::countPlayersReturningSoon5v5(secsTillUnpenalised, 20), 0);
}

GTEST_TEST(R2KTeamLogic, CaptainStaysDuringHoldTime)
{
  const std::vector<std::pair<int, int>> mates = {{2, 800}, {3, 700}};
  EXPECT_EQ(R2KTeamLogic::chooseCaptainWithHysteresis(2, 4, 600, mates, 200, 1000, 250), 2);
}

GTEST_TEST(R2KTeamLogic, CaptainStaysBecauseOfHysteresis)
{
  const std::vector<std::pair<int, int>> mates = {{2, 850}, {3, 700}};
  EXPECT_EQ(R2KTeamLogic::chooseCaptainWithHysteresis(2, 4, 600, mates, 1200, 1000, 300), 2);
}

GTEST_TEST(R2KTeamLogic, CaptainSwitchesWhenClearlyBetterOptionExists)
{
  const std::vector<std::pair<int, int>> mates = {{2, 1300}, {3, 650}};
  EXPECT_EQ(R2KTeamLogic::chooseCaptainWithHysteresis(2, 4, 900, mates, 2000, 1000, 150), 3);
}

GTEST_TEST(R2KTeamLogic, CaptainSwitchesToSelfWhenSelfIsBest)
{
  const std::vector<std::pair<int, int>> mates = {{2, 1300}, {3, 1250}};
  EXPECT_EQ(R2KTeamLogic::chooseCaptainWithHysteresis(2, 4, 700, mates, 2000, 1000, 150), 4);
}
