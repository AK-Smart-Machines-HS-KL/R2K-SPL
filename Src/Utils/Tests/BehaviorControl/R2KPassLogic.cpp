#include "Tools/BehaviorControl/R2KPassLogic.h"

#include "gtest/gtest.h"
#include <cmath>

GTEST_TEST(R2KPassLogic, SelectForwardReceiverUsesLowestNumberAsTieBreak)
{
  std::vector<Teammate> mates(2);
  mates[0].number = 4;
  mates[0].isPenalized = false;
  mates[0].isUpright = true;
  mates[0].theRobotPose.translation = Vector2f(1000.f, 0.f);

  mates[1].number = 2;
  mates[1].isPenalized = false;
  mates[1].isUpright = true;
  mates[1].theRobotPose.translation = Vector2f(1020.f, 100.f);

  const auto receiver = R2KPassLogic::selectForwardReceiver(mates, Vector2f::Zero(), 500.f);
  ASSERT_TRUE(receiver.has_value());
  EXPECT_EQ(receiver->number, 2);
}

GTEST_TEST(R2KPassLogic, FindFreshPassIntentRejectsStaleIntent)
{
  Teammate mate;
  mate.number = 3;
  mate.theBehaviorStatus.activity = BehaviorStatus::offenseForwardPassCard;
  mate.theBehaviorStatus.passTarget = 5;
  mate.timeWhenLastPacketReceived = 1000;

  const std::vector<Teammate> mates = {mate};
  EXPECT_EQ(R2KPassLogic::findFreshPassIntent(mates, 5, 4000, 1500), nullptr);
}

GTEST_TEST(R2KPassLogic, FindFreshPassIntentAcceptsMatchingIntent)
{
  Teammate mate;
  mate.number = 3;
  mate.theBehaviorStatus.activity = BehaviorStatus::offenseForwardPassCard;
  mate.theBehaviorStatus.passTarget = 5;
  mate.timeWhenLastPacketReceived = 3000;

  const std::vector<Teammate> mates = {mate};
  const Teammate* result = R2KPassLogic::findFreshPassIntent(mates, 5, 4000, 1500);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->number, 3);
}

GTEST_TEST(R2KPassLogic, FindFreshPassIntentRejectsFutureTimestamp)
{
  Teammate mate;
  mate.number = 3;
  mate.theBehaviorStatus.activity = BehaviorStatus::offenseForwardPassCard;
  mate.theBehaviorStatus.passTarget = 5;
  mate.timeWhenLastPacketReceived = 5000;

  const std::vector<Teammate> mates = {mate};
  EXPECT_EQ(R2KPassLogic::findFreshPassIntent(mates, 5, 4000, 1500), nullptr);
}
