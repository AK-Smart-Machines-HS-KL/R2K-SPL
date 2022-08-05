#include "DefaultPoseProvider.h"
#include "Tools/Debugging/DebugDrawings3D.h"

MAKE_MODULE(DefaultPoseProvider, behaviorControl);

void DefaultPoseProvider::update(DefaultPose& defaultPose)
{
  static Vector2f scaleFactor(1.0f ,1.0f);

  // Only runs once
  if (!isInit) {

    // Calulate Scaling factor
    if (autoScale)
    {
      // calulate scaling factor based on standard field
      // Dimensions provided in FieldDimensions are half of field size, thus 4.5m and 3m
      scaleFactor.x() = theFieldDimensions.xPosOpponentGroundLine / 4500.0f;
      scaleFactor.y() = theFieldDimensions.yPosLeftSideline / 3000.f;
    }
    isInit = true;
  }
  
  
  
  for (size_t i = 0; i < defaultPose.teamDefaultPoses.size(); i++)
  {
    // Assign default poses by role
    switch (theTeammateRoles.roles[i])
    {
      case TeammateRoles::GOALKEEPER: defaultPose.teamDefaultPoses[i] = goaliePose; break;
      case TeammateRoles::DEFENSE: defaultPose.teamDefaultPoses[i] = defenseMidPose; break;
      case TeammateRoles::OFFENSE: defaultPose.teamDefaultPoses[i] = offenseMidPose; break;

      // 
      //case TeammateRoles::GOALKEEPER: defaultPose.teamDefaultPoses[i] = goaliePose; break;
      //case TeammateRoles::GOALKEEPER: defaultPose.teamDefaultPoses[i] = goaliePose; break; 
    }

    // Apply offset by team Activity
    if (theTeammateRoles.roles[i] != TeammateRoles::GOALKEEPER) // Do not apply offset to Goalie
    {
      float offset = 0; // local offset var for readability
      switch (theTeamBehaviorStatus.teamActivity)
      {
        case TeamBehaviorStatus::R2K_DEFENSIVE_GAME : offset = offsetDefense;
        break;
        case TeamBehaviorStatus::R2K_OFFENSIVE_GAME : offset = offsetDefense;
        break;
      }
      
      defaultPose.teamDefaultPoses[i].translation.x() = defaultPose.teamDefaultPoses[i].translation.x() + offset;
    }
    
    // Scale Translation by scaleFactor
    if (autoScale)
    {
      // This is an Elementwise multiplication of two vectors
      defaultPose.teamDefaultPoses[i].translation = defaultPose.teamDefaultPoses[i].translation.array() * scaleFactor.array();
    }
  }

  // Set Own Pos
  defaultPose.ownDefaultPose = defaultPose.teamDefaultPoses[theRobotInfo.number - 1];

  defaultPose.isOnDefaultPosition = (defaultPose.ownDefaultPose.translation - theRobotPose.translation).norm() < PositionTolerance;

  defaultPose.isOnDefaultPose = 
    defaultPose.isOnDefaultPosition 
    && abs(defaultPose.ownDefaultPose.rotation - theRobotPose.rotation)  < AngleTolerance;
}
