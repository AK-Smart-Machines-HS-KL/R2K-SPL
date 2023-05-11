/**
 * @file TeamCommSentinel.h
 * @author Andy Hobelsberger
 */

#pragma once

#include "Tools/Module/Module.h"
#include "Representations/Communication/TeamCommStatus.h"

MODULE(TeamCommSentinel,
{,
  PROVIDES_WITHOUT_MODIFY(TeamCommStatus),

  DEFINES_PARAMETERS(
  {,
  }),
});

class TeamCommSentinel : public TeamCommSentinelBase
{
public:
  void update(TeamCommStatus& status) override {}
};
