#include "R2KTeamLogic.h"

#include <algorithm>
#include <limits>

namespace R2KTeamLogic
{
  int countActivePlayers5v5(const std::vector<bool>& penalizedFlags)
  {
    int activePlayers = 0;
    for(size_t i = 0; i < penalizedFlags.size() && i < 5; ++i)
      if(!penalizedFlags[i])
        ++activePlayers;
    return activePlayers;
  }

  bool shouldUseSparseMode(const int ownActivePlayers,
                           const int oppActivePlayers,
                           const int sparseModeOpponentActiveThreshold,
                           const int sparseModeTotalActiveThreshold)
  {
    return oppActivePlayers <= sparseModeOpponentActiveThreshold ||
           (ownActivePlayers + oppActivePlayers) <= sparseModeTotalActiveThreshold;
  }

  int countPlayersReturningSoon5v5(const std::vector<int>& secsTillUnpenalised,
                                   const int lookaheadSecs)
  {
    int returningSoon = 0;
    for(size_t i = 0; i < secsTillUnpenalised.size() && i < 5; ++i)
      if(secsTillUnpenalised[i] > 0 && secsTillUnpenalised[i] <= lookaheadSecs)
        ++returningSoon;
    return returningSoon;
  }

  int chooseCaptainWithHysteresis(const int lastCaptain,
                                  const int selfNumber,
                                  const int selfDistance,
                                  const std::vector<std::pair<int, int>>& teammateNumberAndDistance,
                                  const int elapsedSinceCaptainChangeMs,
                                  const int holdTimeMs,
                                  const int switchHysteresisMm)
  {
    int minDistance = selfDistance;
    int bestCaptain = selfNumber;
    int previousCaptainDistance = std::numeric_limits<int>::max();

    if(lastCaptain == selfNumber)
      previousCaptainDistance = selfDistance;

    for(const auto& teammate : teammateNumberAndDistance)
    {
      const int number = teammate.first;
      const int distance = teammate.second;
      if(distance < minDistance)
      {
        minDistance = distance;
        bestCaptain = number;
      }
      if(number == lastCaptain)
        previousCaptainDistance = distance;
    }

    if(elapsedSinceCaptainChangeMs < holdTimeMs)
      return lastCaptain;

    if(lastCaptain > 0 && previousCaptainDistance <= minDistance + switchHysteresisMm)
      return lastCaptain;

    return bestCaptain;
  }
}
