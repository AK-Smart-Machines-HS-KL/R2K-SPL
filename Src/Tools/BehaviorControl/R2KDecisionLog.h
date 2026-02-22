#pragma once

#include <string>
#include <vector>

namespace R2KDecisionLog
{
  inline std::string buildMessage(const char* eventName,
                                  const std::vector<std::pair<std::string, std::string>>& fields)
  {
    std::string message = "R2KLOG|event=";
    message += eventName;
    for(const auto& field : fields)
    {
      message += "|";
      message += field.first;
      message += "=";
      message += field.second;
    }
    return message;
  }

  void annotation(const char* eventName,
                  const std::vector<std::pair<std::string, std::string>>& fields);
}
