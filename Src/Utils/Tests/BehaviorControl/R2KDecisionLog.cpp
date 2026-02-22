#include "Tools/BehaviorControl/R2KDecisionLog.h"

#include "gtest/gtest.h"

GTEST_TEST(R2KDecisionLog, BuildMessageWithFields)
{
  const std::string message = R2KDecisionLog::buildMessage("captain_change",
                                                            {{"from", "2"}, {"to", "4"}, {"player", "3"}});
  EXPECT_EQ(message, "R2KLOG|event=captain_change|from=2|to=4|player=3");
}

GTEST_TEST(R2KDecisionLog, BuildMessageWithoutFields)
{
  const std::string message = R2KDecisionLog::buildMessage("relocalize_state", {});
  EXPECT_EQ(message, "R2KLOG|event=relocalize_state");
}
