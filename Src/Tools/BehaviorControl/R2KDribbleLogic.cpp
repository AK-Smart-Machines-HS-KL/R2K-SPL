#include "R2KDribbleLogic.h"

namespace R2KDribbleLogic
{
  DribbleDecision decide(const bool ballSeenRecently,
                         const bool localizationPoor,
                         const float ballTravelEstimateMm,
                         const float stableBallTravelThresholdMm)
  {
    if(!ballSeenRecently)
      return {DribbleMode::recover, DribbleReason::ballUncertain};

    if(localizationPoor)
      return {DribbleMode::cautiousAdvance, DribbleReason::localizationPoor};

    if(ballTravelEstimateMm > stableBallTravelThresholdMm)
      return {DribbleMode::cautiousAdvance, DribbleReason::ballUncertain};

    return {DribbleMode::advance, DribbleReason::stable};
  }

  const char* toString(const DribbleMode mode)
  {
    switch(mode)
    {
      case DribbleMode::advance: return "advance";
      case DribbleMode::cautiousAdvance: return "cautiousAdvance";
      case DribbleMode::recover: return "recover";
      default: return "unknown";
    }
  }

  const char* toString(const DribbleReason reason)
  {
    switch(reason)
    {
      case DribbleReason::stable: return "stable";
      case DribbleReason::localizationPoor: return "localizationPoor";
      case DribbleReason::ballUncertain: return "ballUncertain";
      default: return "unknown";
    }
  }
}
