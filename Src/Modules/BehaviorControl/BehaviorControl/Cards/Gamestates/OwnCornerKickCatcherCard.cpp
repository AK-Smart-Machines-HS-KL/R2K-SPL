/**
 * @file OwnCornerKickCatcherCard.cpp
 * @author Dennis Fuhrmann
 * @brief Card to recieve the Pass from OwnCornerKickCard.cpp and kick it into the Goal
 *          based on ChallangeCard.cpp from Branch 2025-03-IRB 
 *        Second Half of this konzept. 
 *        Performs the Kick into the Goal
 * 
 * @version 1.0 Copy of ChallangeCard.cpp and walk to OpponentPenaltyMark and we don't assume  we already are at the korrekt position and continue walking to the Point
 *          1.1 Preconditions cahnged to actually work in a real game 
 *              Robot angles himself to the corner where Ball is Located   
 * @date 2025-06-03
 * 
 * future Work: Split this Card into two  - first one where the Catcher walks to the OpponentPenaltyArea (OwnCornerKickCatcherWalkInCard.cpp)
 *                                        - second one actually plays the Ball into the goal
 */

 // Skills - Must be included BEFORE Card Base
 #include "Representations/BehaviorControl/Skills.h"

 // Card Base
 #include "Tools/BehaviorControl/Framework/Card/Card.h"
 #include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
 
 
 // Representations
 #include "Representations/Configuration/FieldDimensions.h"
 #include "Representations/BehaviorControl/FieldBall.h"
 #include "Representations/Communication/TeamData.h"
 #include "Representations/Modeling/BallModel.h"
 #include "Representations/Configuration/BallSpecification.h"

 #include "Representations/Communication/GameInfo.h"
 #include "Representations/Communication/TeamInfo.h"
 #include "Representations/Communication/RobotInfo.h"
 #include "Representations/Communication/TeamCommStatus.h"
 #include "Representations/BehaviorControl/PlayerRole.h"
 #include "Representations/BehaviorControl/Libraries/LibTeam.h"
 #include "Representations/Infrastructure/ExtendedGameInfo.h"


 
 //#include <filesystem>
 #include "Tools/Modeling/BallPhysics.h"
 #include "Tools/Math/Eigen.h"
 #include "Tools/Math/Geometry.h"
 
 
 
 CARD(OwnCornerKickCatcherCard,
   {
        ,
        CALLS(Activity),
        CALLS(WalkToPoint),
        CALLS(LookAtBall),
        CALLS(Stand),
        REQUIRES(FieldBall),
        REQUIRES(RobotPose),
        REQUIRES(FieldDimensions),
        REQUIRES(GameInfo),
        REQUIRES(RobotInfo),
        REQUIRES(PlayerRole),
        REQUIRES(OwnTeamInfo),
        REQUIRES(TeammateRoles),
        REQUIRES(BallModel),
        REQUIRES(BallSpecification),
        REQUIRES(TeamCommStatus),
        REQUIRES(LibTeam),
        REQUIRES(ExtendedGameInfo),
        
     
        DEFINES_PARAMETERS(
       {,
          (int)(100) targetOffset, // target behind penalty point
          (float)(7000.f) timeOut, // how long after last Penalty Kick (CornerKick) (6 sec)
          (Vector2f)(Vector2f(200.f,0.f)) interceptPoint, // just a few steps forward 
          (bool)(false) pointIsSelected, // InterceptPoint wird nur einmal berechnet
          (float)(1.2f) interceptFactor, // veringere diesen Wert um den Ball früher in seiner Lufbahn zu intercepten
          (float)(0.9f) minDistanceFactor, // eröhe diesen Wert um den distanz zu erhöhen die der Ball unterschreiten muss damit der Roboter reagiert
          (float)(100.0f) interceptOffset, // sowohl x und y Werte sind betroffen
          (Angle)(Angle(30_deg)) goalOffset, // needed so the Nao actually looks at the goal and not the goal post, Offset is relative to goal post
          (Angle)(Angle(2_deg)) goalPrecision, // (suggested Value) how precice the Nao turn towards the goal 
       }),
       
     });
     
 
 class OwnCornerKickCatcherCard : public OwnCornerKickCatcherCardBase
 {

        // active if last freee Kick was under 5 sec ago  
     bool preconditions() const override
     {
       return  thePlayerRole.supporterIndex() == thePlayerRole.numOfActiveSupporters - 1  //  i am Supporter 
         //&&   (theLibTeam.strikerPose.translation.x() >= theFieldDimensions.xPosOpponentPenaltyMark && (theLibTeam.strikerPose.translation.y() >= theFieldDimensions.yPosLeftPenaltyArea || theLibTeam.strikerPose.translation.y() <= theFieldDimensions.yPosRightPenaltyArea)) // Striker is in Corner  
         // doesnt work how i want it to
         &&    ballIsInCorner()
         &&    theGameInfo.setPlay == SET_PLAY_NONE  // No Penalty
         &&    theExtendedGameInfo.timeSinceLastFreeKickEnded < timeOut; // 3 sec since last penalty
         
     }
 
     bool postconditions() const override
     {
       return !preconditions();
     }
 
     void execute() override
     {
       theActivitySkill(BehaviorStatus::ownFreeKick);
 
      
 
       //Berechnet die Distanz vom Roboter zum Balls 
       float minDistance = calcMinDistance();
       
         if (calcDistanceToBall() <= minDistance) {
           
              
             // InterceptPoint wird nur einmal berechnet
           if (!pointIsSelected) {
             
             interceptPoint = calcInterceptPoint();
             pointIsSelected = true;
           }
               //Die Kick Skills sind nicht schnell genug um einen rollenden Ball zu intercepten stadesssen nutzen wir einen Lauf in den Ball rein um einen Kick zu simulieren
              theWalkToPointSkill(Pose2f(0_deg, interceptPoint), 1.f, true, true, true, true);
              theLookAtBallSkill(); // HeadMotion controll
           
            // we don't assume we already are at the correkt position
              
         }else if (theRobotPose.toRelative(Vector2f(theFieldDimensions.xPosOpponentPenaltyMark - targetOffset, 0)) != Vector2f::Zero())
         {
          if (theFieldBall.positionOnField.x() >= 0){ // if the Ball is above the middle Line turn slightly left
            theWalkToPointSkill(Pose2f(calcAngleToGoal() - goalOffset, theRobotPose.toRelative(Vector2f(theFieldDimensions.xPosOpponentPenaltyMark - targetOffset, 0))));
          }
          else { // else turn slightly right
            theWalkToPointSkill(Pose2f(calcAngleToGoal() + goalOffset, theRobotPose.toRelative(Vector2f(theFieldDimensions.xPosOpponentPenaltyMark - targetOffset, 0))));
          }
           theLookAtBallSkill(); // head Motion Control       
         }else
         {
           
           theStandSkill();
           theLookAtBallSkill(); // head Motion Control
         }
       
     }
     // kopiert aus CornerKick
     Angle calcAngleToGoal() const
     {
       return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
     }
 
     // Altenativ TimetoReachBall benutzen (konnte nicht herausfinden wie)  
     // relativer Abstand nach Pytagoras
     float calcDistanceToBall() const
     {
       Vector2f temp1 = theFieldBall.recentBallPositionRelative();
       float temp2 = temp1.x() * temp1.x();
       float temp3 = temp1.y() * temp1.y();
       return std::sqrt(temp2 + temp3);
     }
 
    
     // ifdef weil es unterschiedliche ergebnisse beim Simulator und im echten Roboter gibt
     Vector2f calcInterceptPoint() const
     {
       Vector2f temp = BallPhysics::propagateBallPosition(theFieldBall.recentBallPositionOnField(), theBallModel.estimate.velocity, interceptFactor, theBallSpecification.friction);
       //Für den Fehler beim echten Roboter (die Werte sind invertiert)
 #ifdef TARGET_ROBOT
       Vector2f result = Vector2f(-(temp.x() + interceptOffset), -(temp.y() + interceptOffset));
 #else
       Vector2f result = Vector2f((temp.x() + interceptOffset), (temp.y() + interceptOffset));
 #endif //Simulator
       return result;
     }
 
 
     //berechnet den Schwellwert für die Distance zum Ball anhand der Geschwnidkeit des Balls
     float calcMinDistance() const
     {
       // Geschwindigkeit des Balls durch Pythagoras
       Vector2f temp1 = theBallModel.estimate.velocity;
       float temp2 = temp1.x() * temp1.x();
       float temp3 = temp1.y() * temp1.y();
       float result = std::sqrt(temp2 + temp3);
       float distance = 0.f;
       // Ball stands still inclusive assumed sensor jitter
         if (result >= 1) {
           distance = Geometry::getDistanceToLine(Geometry::Line(theFieldBall.recentBallPositionRelative(), temp1), Vector2f::Zero());
         }
       return (result + distance) * minDistanceFactor;
     }
    bool ballIsInCorner() const
    {
      return (theFieldBall.positionOnField.x() > theFieldDimensions.xPosOpponentPenaltyArea  // is behind penelty area line
              && (theFieldBall.positionOnField.y() <= theFieldDimensions.yPosLeftPenaltyArea // is Left of penalty area
              || theFieldBall.positionOnField.y() >= theFieldDimensions.yPosRightPenaltyArea)); // is Right of penalty area
    }
 };
 
 MAKE_CARD(OwnCornerKickCatcherCard);
