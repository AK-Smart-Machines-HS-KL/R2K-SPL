#include "R2KPassLogic.h"

#include <cmath>

namespace R2KPassLogic
{
  std::optional<ReceiverCandidate> selectForwardReceiver(const std::vector<Teammate>& teammates,
                                                         const Vector2f& selfTranslation,
                                                         const float minForwardDistanceMm)
  {
    std::optional<ReceiverCandidate> best = std::nullopt;
    float bestX = -100000.f;

    for(const auto& mate : teammates)
    {
      if(mate.isPenalized || !mate.isUpright)
        continue;

      const float xDelta = mate.theRobotPose.translation.x() - selfTranslation.x();
      if(xDelta < minForwardDistanceMm)
        continue;

      const bool betterX = !best.has_value() || mate.theRobotPose.translation.x() > bestX;
      const bool tieBreakByNumber = best.has_value() &&
                                    std::abs(mate.theRobotPose.translation.x() - bestX) < 50.f &&
                                    mate.number < best->number;
      if(betterX || tieBreakByNumber)
      {
        best = ReceiverCandidate{mate.number, mate.theRobotPose.translation};
        bestX = mate.theRobotPose.translation.x();
      }
    }

    return best;
  }

  const Teammate* findFreshPassIntent(const std::vector<Teammate>& teammates,
                                      const int receiverNumber,
                                      const unsigned now,
                                      const unsigned timeoutMs)
  {
    for(const auto& mate : teammates)
    {
      if(mate.theBehaviorStatus.activity != BehaviorStatus::offenseForwardPassCard)
        continue;
      if(mate.theBehaviorStatus.passTarget != receiverNumber)
        continue;
      if(now < mate.timeWhenLastPacketReceived ||
         now - mate.timeWhenLastPacketReceived > timeoutMs)
        continue;
      return &mate;
    }

    return nullptr;
  }
}
