/**
 * @file LibTeammatesProvider.h
 *
 * Contains information about teammate positions
 *
 * @author Lukas Malte Monnerjahn
 */

#include "Tools/Module/Module.h"
#include "Representations/BehaviorControl/Libraries/LibPosition.h"
#include "Representations/BehaviorControl/Libraries/LibTeammates.h"
#include "Representations/Communication/TeamData.h"
#include "Representations/Modeling/RobotPose.h"

MODULE(LibTeammatesProvider,
{,
  REQUIRES(LibPosition),
  REQUIRES(RobotPose),
  REQUIRES(TeamData),
  PROVIDES(LibTeammates),
  DEFINES_PARAMETERS({,
   (float) (400.f) outsideDistanceThreshold,
   (float) (50.f) insideDistanceThreshold,
   }),
});

class LibTeammatesProvider : public LibTeammatesProviderBase
{
private:
  void update(LibTeammates& libTeammates) override;
};