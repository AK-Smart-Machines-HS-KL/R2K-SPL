/**
 * @file ChallangeCard.cpp
 * @author Dennis Fuhrmann
 * @brief Card for R2k Ball Challange 2025
 *        In this Card the Ball is not aktually kicked,
 *        instead a walk into the Ball is performed to "fake" a kick forward.
 *        TODO Live Test with Real robots
 * 
 *        version 1.1 The Interceptpoint can no longer be behind the Robot
 * @version 1.2
 * @date 2025-29-01
 *
 *
 */

 // Skills - Must be included BEFORE Card Base
#include "Representations/BehaviorControl/Skills.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"


// Representations
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Modeling/ObstacleModel.h"
#include "Representations/Communication/TeamData.h"
#include "Representations/Modeling/BallModel.h"
#include "Representations/Configuration/BallSpecification.h"
#include "Representations/BehaviorControl/Shots.h"


//#include <filesystem>
#include "Tools/Modeling/BallPhysics.h"
#include "Tools/Math/Eigen.h"
#include "Tools/Math/Geometry.h"
#include "Tools/BehaviorControl/Interception.h"



CARD(ChallangeCard,
  {
       ,
       CALLS(Activity),
       CALLS(WalkToPoint),
       CALLS(LookAtBall),
       CALLS(WalkToKickoffPose),
       CALLS(TurnAngle),
       CALLS(GoToBallAndKick),
       CALLS(Say),
       REQUIRES(Shots),
       REQUIRES(FieldBall),
       REQUIRES(RobotPose),
       REQUIRES(FieldDimensions),
       REQUIRES(BallModel),
       REQUIRES(BallSpecification),
       
       
       DEFINES_PARAMETERS(
      {,
         (Vector2f)(Vector2f(200.f,0.f)) interceptPoint, // just a few steps forward 
         (bool)(false) pointIsSelected, // InterceptPoint wird nur einmal berechnet
         (float)(1.2f) interceptFactor, // veringere diesen Wert um den Ball früher in seiner Lufbahn zu intercepten
         (float)(1.0f) minDistanceFactor, // eröhe diesen Wert um den distanz zu erhöhen die der Ball unterschreiten muss damit der Roboter reagiert
         (bool)(false) done,        
         (bool)(false) walking,     // Kümmert sich um die Beendung der Card
      }),
      
    });
    

class ChallangeCard : public ChallangeCardBase
{

    //always active
    bool preconditions() const override
    {
        return theFieldBall.timeSinceBallWasSeen < 7000;
    }

    bool postconditions() const override
    {
      return  !(theFieldBall.timeSinceBallWasSeen < 7000) || !done;

    }

    void execute() override
    {
     // theActivitySkill(BehaviorStatus::testingBehavior);

     

      //Berechnet die Distanz vom Roboter zum Balls 
      float minDistance = calcMinDistance();
      
        if (calcDistanceToBall() <= minDistance) {
          
             
            // InterceptPoint wird nur einmal berechnet
          if (!pointIsSelected) {
            theActivitySkill(BehaviorStatus::teachin);
            interceptPoint = calcInterceptPoint();
            pointIsSelected = true;
            OUTPUT_TEXT("x = " << std::to_string(interceptPoint.x()) << ", y = " << std::to_string(interceptPoint.y()));
          }
              //Die Kick Skills sind nicht schnell genug um einen rollenden Ball zu intercepten stadesssen nutzen wir einen Lauf in den Ball rein um einen Kick zu simulieren
             theWalkToPointSkill(Pose2f(0_deg, interceptPoint), 1.f, true, true, true, true);
             theLookAtBallSkill(); // HeadMotion controll
             walking = true;
          


        }else
        {
          theActivitySkill(BehaviorStatus::goalShot);
          if (walking = true)
          {
            done = true;
          }
          theTurnAngleSkill(calcAngleToGoal() + 30_deg, 2_deg);
          theLookAtBallSkill();
        }
      
      
    }

    Angle calcAngleToGoal() const
    {
      return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
    }

    //Altenativ TimetoReachBall benutzen (konnte nicht herausfinden wie)  
    float calcDistanceToBall() const
    {
      Vector2f temp1 = theFieldBall.recentBallPositionRelative();
      float temp2 = temp1.x() * temp1.x();
      float temp3 = temp1.y() * temp1.y();
      return std::sqrt(temp2 + temp3);
    }

    //relative InterceptPoint wird berechnet durch propagateBallPosition und einem Festen Offset für einen besseren Schritt in den Ball, ifdef weil es unterschiedliche ergebnisse beim Simulator und im echten Roboter gibt
    Vector2f calcInterceptPoint() const
    {
      Vector2f temp = BallPhysics::propagateBallPosition(theFieldBall.recentBallPositionOnField(), theBallModel.estimate.velocity, interceptFactor, theBallSpecification.friction);
      Vector2f result = Vector2f::Zero();
      //Für den Fehler beim echten Roboter (die Werte sind invertiert)
#ifdef NAO
      result = Vector2f(-(temp.x() + 100.f), -(temp.y() + 100.f));
#else
      result = Vector2f((temp.x() + 100.f), (temp.y() + 100.f));
#endif //Simulator
      return result;
    }


    //berechnet den Schwellwert für die Distance zum Ball anhand der Geschwnidkeit des Balls
    float calcMinDistance() const
    {
      Vector2f temp1 = theBallModel.estimate.velocity;
      float temp2 = temp1.x() * temp1.x();
      float temp3 = temp1.y() * temp1.y();
      float result = std::sqrt(temp2 + temp3);
      float distance = 0.f;
        if (result >= 1) {
          distance = Geometry::getDistanceToLine(Geometry::Line(theFieldBall.recentBallPositionRelative(), temp1), Vector2f::Zero());
        }
      return (result + distance) * minDistanceFactor;
    }
};

MAKE_CARD(ChallangeCard);
