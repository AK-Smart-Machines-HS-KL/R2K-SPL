/**
 * @file ChallangeCard.cpp
 * @author Dennis Fuhrmann
 * @brief Card for R2k Ball Challange 2025
 *        Final approach: instead of a regular kick, we use "stumbling", 
 *        i.e. a walk into the ball is performed to "fake" a kick forward.
 *        
 * 
 * @version 1.1 calcInterceptpoint() berechnet den relativen Punkt den der Roboter anlaufen muss, um den Ball zu "kicken".
 *              Dieser Punkt wird  noch um den Wert interceptOffset erhöht damit der Punkt hinter dem Ball liegt und ein Lauf in den Ball
 *              mehr mach einem Kick aussieht.
 * @version 1.2 Diskrepanz zwichen Simulator und echten Nao erfordern spiegelung der x,y-Koordinaten des InterceptPoint
 * @date 2025-04-03
 *
 * 
 * Future work (as of 3/3´/25)
 * - to migrate this card into a two player corner kick we need to add theGameInfo.setPlay != SET_PLAY_CORNER_KICK to pre-cond, and 
 *   verify, there is enough time for the kick to be excuted
 * - clean up: theActivitySkill(BehaviorStatus::ownFreeKick);
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


//#include <filesystem>
#include "Tools/Modeling/BallPhysics.h"
#include "Tools/Math/Eigen.h"
#include "Tools/Math/Geometry.h"



CARD(ChallangeCard,
  {
       ,
       CALLS(Activity),
       CALLS(WalkToPoint),
       CALLS(LookAtBall),
       CALLS(TurnAngle),
       REQUIRES(FieldBall),
       REQUIRES(RobotPose),
       REQUIRES(FieldDimensions),
       REQUIRES(BallModel),
       REQUIRES(BallSpecification),
       
    
       DEFINES_PARAMETERS(
      {,
         (int)(7000) timeOut, // controls pre and postcondition, how long ago the Ball had to be visible  
         (Vector2f)(Vector2f(200.f,0.f)) interceptPoint, // just a few steps forward 
         (bool)(false) pointIsSelected, // InterceptPoint wird nur einmal berechnet
         (float)(1.2f) interceptFactor, // veringere diesen Wert um den Ball früher in seiner Lufbahn zu intercepten
         (float)(1.0f) minDistanceFactor, // eröhe diesen Wert um den distanz zu erhöhen die der Ball unterschreiten muss damit der Roboter reagiert
         (float)(100.0f) interceptOffset, // sowohl x und y Werte sind betroffen
         (Angle)(Angle(30_deg)) goalOffset, // needed so the Nao actually looks at the goal and not the goal post, Offset is relative to goal post
         (Angle)(Angle(2_deg)) goalPrecision, // (suggested Value) how precice the Nao turn towards the goal 
      }),
      
    });
    

class ChallangeCard : public ChallangeCardBase
{

    //always active
    bool preconditions() const override
    {
        return theFieldBall.timeSinceBallWasSeen < timeOut;
    }

    bool postconditions() const override
    {
      return  !preconditions();

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
          


        }else
        {
          
          theTurnAngleSkill(calcAngleToGoal() + goalOffset, goalPrecision);
          theLookAtBallSkill();
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
};

MAKE_CARD(ChallangeCard);
