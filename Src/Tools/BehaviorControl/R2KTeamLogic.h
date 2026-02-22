#pragma once

#include <utility>
#include <vector>

namespace R2KTeamLogic
{
  int countActivePlayers5v5(const std::vector<bool>& penalizedFlags);

  bool shouldUseSparseMode(int ownActivePlayers,
                           int oppActivePlayers,
                           int sparseModeOpponentActiveThreshold,
                           int sparseModeTotalActiveThreshold);

  int countPlayersReturningSoon5v5(const std::vector<int>& secsTillUnpenalised,
                                   int lookaheadSecs);

  int chooseCaptainWithHysteresis(int lastCaptain,
                                  int selfNumber,
                                  int selfDistance,
                                  const std::vector<std::pair<int, int>>& teammateNumberAndDistance,
                                  int elapsedSinceCaptainChangeMs,
                                  int holdTimeMs,
                                  int switchHysteresisMm);
}
