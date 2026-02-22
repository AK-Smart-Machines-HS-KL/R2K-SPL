#include "R2KDecisionLog.h"

#include "Tools/Debugging/Annotation.h"

namespace R2KDecisionLog
{
  void annotation(const char* eventName,
                  const std::vector<std::pair<std::string, std::string>>& fields)
  {
    ANNOTATION("R2KDecision", buildMessage(eventName, fields));
  }
}
