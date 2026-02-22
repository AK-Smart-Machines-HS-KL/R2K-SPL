#pragma once

#include "Representations/Communication/TeamData.h"

#include <optional>

namespace R2KPassLogic
{
  struct ReceiverCandidate
  {
    int number = -1;
    Vector2f pose = Vector2f::Zero();
  };

  std::optional<ReceiverCandidate> selectForwardReceiver(const std::vector<Teammate>& teammates,
                                                         const Vector2f& selfTranslation,
                                                         float minForwardDistanceMm);

  const Teammate* findFreshPassIntent(const std::vector<Teammate>& teammates,
                                      int receiverNumber,
                                      unsigned now,
                                      unsigned timeoutMs);
}
