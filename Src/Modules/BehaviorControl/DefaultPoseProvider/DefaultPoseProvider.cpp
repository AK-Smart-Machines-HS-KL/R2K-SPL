#include "DefaultPoseProvider.h"
#include "Tools/Debugging/DebugDrawings3D.h"

MAKE_MODULE(DefaultPoseProvider, behaviorControl);

void DefaultPoseProvider::update(DefaultPose& defaultPose)
{
  // Calulate Scaling factor
  Vector2f scaleFactor(1.0f ,1.0f);
  if (autoScale)
  {
    // calulate scaling factor based on standard field
    // Dimensions provided in FieldDimensions are half of field size, thus 4.5m and 3m
    scaleFactor.x() = theFieldDimensions.xPosOpponentGroundLine / 4500.0f;
    scaleFactor.y() = theFieldDimensions.yPosLeftSideline / 3000.f;
  }
  
  
  for (size_t i = 0; i < defaultPose.teamDefaultPoses.size(); i++)
  {
    // Assign default poses by role
    switch (theTeammateRoles.roles[i])
    {
      case TeammateRoles::GOALKEEPER: defaultPose.teamDefaultPoses[i] = goaliePose; break;
      case TeammateRoles::DEFENSE: defaultPose.teamDefaultPoses[i] = defenseMidPose; break;
      case TeammateRoles::OFFENSE: defaultPose.teamDefaultPoses[i] = offenseMidPose; break;
      //case TeammateRoles::GOALKEEPER: defaultPose.teamDefaultPoses[i] = goaliePose; break;
      //case TeammateRoles::GOALKEEPER: defaultPose.teamDefaultPoses[i] = goaliePose; break; 
    }

    // Apply offset by team Activity
    switch (theTeamBehaviorStatus.teamActivity)
    {
      case TeamBehaviorStatus::R2K_DEFENSIVE_GAME :
        defaultPose.teamDefaultPoses[i].translation.x() = defaultPose.teamDefaultPoses[i].translation.x() + offsetDefense;
        break;
      
      case TeamBehaviorStatus::R2K_OFFENSIVE_GAME :
        defaultPose.teamDefaultPoses[i].translation.x() = defaultPose.teamDefaultPoses[i].translation.x() + offsetOffense;
        break;
    }
    
    // Scale Translation by 
    if (autoScale)
    {
      defaultPose.teamDefaultPoses[i].translation = defaultPose.teamDefaultPoses[i].translation.array() * scaleFactor.array();
    }
    
  }

  defaultPose.ownDefaultPose = defaultPose.teamDefaultPoses[theRobotInfo.number - 1];

  drawDefaultPositions();
}

void DefaultPoseProvider::drawDefaultPositions()
{
  DEBUG_DRAWING3D("test_defaultPos", "field")
  {
    // const auto goal = calcAbsoluteFromMargin(goalieMargin);
    // const auto ldef = calcAbsoluteFromMargin(leftDefenseMargin);
    // const auto rdef = calcAbsoluteFromMargin(rightDefenseMargin);
    // const auto loff = calcAbsoluteFromMargin(leftOffenseMargin);
    // const auto roff = calcAbsoluteFromMargin(rightOffenseMargin);

    // SPHERE3D("test_defaultPos", goal.x(), goal.y(), 5, 75, ColorRGBA::red);
    // SPHERE3D("test_defaultPos", ldef.x(), ldef.y(), 5, 75, ColorRGBA::yellow);
    // SPHERE3D("test_defaultPos", rdef.x(), rdef.y(), 5, 75, ColorRGBA::blue);
    // SPHERE3D("test_defaultPos", loff.x(), loff.y(), 5, 75, ColorRGBA::cyan);
    // SPHERE3D("test_defaultPos", roff.x(), roff.y(), 5, 75, ColorRGBA::orange);
  }  
}