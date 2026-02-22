#pragma once

namespace R2KBallSourceLogic
{
  enum class BallSource
  {
    own,
    team,
    forecast,
    unknown,
  };

  BallSource chooseBallSource(bool ownBallSeenRecently, bool teamCommActive, bool useForecast = false);

  const char* toString(BallSource source);
}
