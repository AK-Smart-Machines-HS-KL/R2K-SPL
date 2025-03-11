/**
 * @file OwnCornerKickCatcherWalkInCard.cpp
 * @author Dennis Fuhrmann
 * @brief Card to recieve the Pass from OwnCornerKickCard.cpp and kick it into the goal
 *        based on ChallangeCard.cpp from Branch 2025-03-IRB 
 *        first half of this Konzept. 
 *        Robot Walks to penalty point = target Location
 *        to be in sync with OwnCornerKickCard.cpp
 *        
 * 
 * @version 1.0 Robot Walks to OpponentPenaltyArea during the Own Corner Kick Phase
 * 
 * @date 2025-06-03
 * 
 */

 // Skills - Must be included BEFORE Card Base
 #include "Representations/BehaviorControl/Skills.h"

 // Card Base
 #include "Tools/BehaviorControl/Framework/Card/Card.h"
 #include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
 
 
 
 #include "Representations/Configuration/FieldDimensions.h"
 #include "Representations/Communication/TeamData.h"

 #include "Representations/Communication/GameInfo.h"
 #include "Representations/Communication/TeamInfo.h"
 #include "Representations/Communication/RobotInfo.h"
 #include "Representations/Communication/TeamCommStatus.h"
 #include "Representations/BehaviorControl/Libraries/LibTeam.h"


 
 
 #include "Tools/Math/Eigen.h"
 #include "Tools/Math/Geometry.h"
 
 
 
 CARD(OwnCornerKickCatcherWalkInCard,
   {
        ,
        CALLS(Activity),
        CALLS(WalkToPoint),
        CALLS(LookAtBall),
        CALLS(TurnAngle),
        REQUIRES(RobotPose),
        REQUIRES(FieldDimensions),
        REQUIRES(GameInfo),
        REQUIRES(RobotInfo),
        REQUIRES(PlayerRole),
        REQUIRES(OwnTeamInfo),
        REQUIRES(TeammateRoles),
        REQUIRES(TeamCommStatus),
        REQUIRES(LibTeam),

        
     
        DEFINES_PARAMETERS(
       {, 
          (int)(0) targetOffset, // target behind penalty point 
          (Angle)(Angle(25_deg)) goalOffset, // needed so the Nao actually looks at the goal and not the goal post, Offset is relative to goal post
          (Angle)(Angle(2_deg)) goalPrecision, // (suggested Value) how precice the Nao turn towards the goal 
       }),
       
     });
     
 
 class OwnCornerKickCatcherWalkInCard : public OwnCornerKickCatcherWalkInCardBase
 {
 
     
     bool preconditions() const override
     {
         return !theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive)  // I am not the striker
            &&  theGameInfo.setPlay == SET_PLAY_CORNER_KICK     
            &&  theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber // ownCornerKick 
            &&  thePlayerRole.supporterIndex() == thePlayerRole.numOfActiveSupporters - 1 ; // but i am Supporter
     }
 
     bool postconditions() const override
     {
       return  !preconditions();  
     }
 
     void execute() override
     {
       theActivitySkill(BehaviorStatus::ownFreeKick);
 
         if (theRobotPose.toRelative(Vector2f(theFieldDimensions.xPosOpponentPenaltyMark - targetOffset, 0)) != Vector2f::Zero())
         {
           Pose2f strikerPose = theLibTeam.strikerPose;
           if (theRobotPose.toRelative(strikerPose).translation.y() >= 0){ // if the striker is above the middle Line turn left
             theWalkToPointSkill(Pose2f(calcAngleToGoal() + goalOffset, theRobotPose.toRelative(Vector2f(theFieldDimensions.xPosOpponentPenaltyMark - targetOffset, 0))));
           }
           else { // else turn right
             theWalkToPointSkill(Pose2f(calcAngleToGoal() - goalOffset, theRobotPose.toRelative(Vector2f(theFieldDimensions.xPosOpponentPenaltyMark - targetOffset, 0))));
           }
            theLookAtBallSkill(); // head Motion Control
         }else
         {
           
           theTurnAngleSkill(calcAngleToGoal(), goalPrecision);
           theLookAtBallSkill(); // head Motion Control
         }
       
       
     }
     // kopiert aus CornerKick
     Angle calcAngleToGoal() const
     {
       return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
     }

 };
 
 MAKE_CARD(OwnCornerKickCatcherWalkInCard);
