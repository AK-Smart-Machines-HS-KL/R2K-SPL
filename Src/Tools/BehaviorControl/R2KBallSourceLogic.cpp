#include "R2KBallSourceLogic.h"

namespace R2KBallSourceLogic
{
  BallSource chooseBallSource(const bool ownBallSeenRecently, const bool teamCommActive, const bool useForecast)
  {
    if(useForecast)
      return BallSource::forecast;
    if(ownBallSeenRecently)
      return BallSource::own;
    if(teamCommActive)
      return BallSource::team;
    return BallSource::unknown;
  }

  const char* toString(const BallSource source)
  {
    switch(source)
    {
      case BallSource::own: return "own";
      case BallSource::team: return "team";
      case BallSource::forecast: return "forecast";
      case BallSource::unknown: return "unknown";
      default: return "unknown";
    }
  }
}
