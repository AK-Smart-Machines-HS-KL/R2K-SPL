#pragma once

namespace R2KDribbleLogic
{
  enum class DribbleMode
  {
    advance,
    cautiousAdvance,
    recover,
  };

  enum class DribbleReason
  {
    stable,
    localizationPoor,
    ballUncertain,
  };

  struct DribbleDecision
  {
    DribbleMode mode = DribbleMode::recover;
    DribbleReason reason = DribbleReason::ballUncertain;
  };

  DribbleDecision decide(bool ballSeenRecently,
                         bool localizationPoor,
                         float ballTravelEstimateMm,
                         float stableBallTravelThresholdMm);

  const char* toString(DribbleMode mode);
  const char* toString(DribbleReason reason);
}
