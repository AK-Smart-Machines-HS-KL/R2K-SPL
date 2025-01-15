/**
 * @file ChallangeCard.cpp
 * @author Dennis Fuhrmann
 * @brief Card for R2k Ball Challange 2025
 *        Currently still in its infancy
 * @version 0.2
 * @date 2025-06-01
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
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Modeling/ObstacleModel.h"
#include "Representations/Communication/TeamData.h"
#include "Representations/Modeling/BallModel.h"
#include "Representations/BehaviorControl/Libraries/LibWalk.h"


//#include <filesystem>
#include "Tools/Modeling/BallPhysics.h"
#include "Tools/Math/Eigen.h"
#include "Tools/BehaviorControl/Interception.h"


// Modify this card but don't commit changes to keep it clean for other developers
// Also don't forget to put this card at the top of your Card Stack!
CARD(ChallangeCard,
  {
       ,
       CALLS(Activity),
       CALLS(LookForward),
       CALLS(Stand),
       CALLS(WalkToPoint),
       CALLS(InterceptBall),
       CALLS(LookAtBall),
       CALLS(WalkToKickoffPose),
       CALLS(GoToBallAndKick),
       CALLS(GoToBallAndDribble),
       CALLS(WalkToBallAndKick),
       CALLS(TurnAngle),
       CALLS(Dribble),
       REQUIRES(FieldBall),
       REQUIRES(RobotPose),
       REQUIRES(RobotInfo),
       REQUIRES(FieldDimensions),
       REQUIRES(BallModel),
       REQUIRES(LibWalk),


       DEFINES_PARAMETERS(
      {,
        (bool)(false) footIsSelected,  // freeze the first decision
        (bool)(true) leftFoot,
        (bool)(false) isKicking,
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
      return  !(theFieldBall.timeSinceBallWasSeen < 7000) && false;

    }

    void execute() override
    {
      theActivitySkill(BehaviorStatus::testingBehavior);

      Vector2f intersectionwithownYAxis = theFieldBall.intersectionPositionWithOwnYAxis;
      Vector2f inersectionwithOwnXAxis = Vector2f::Zero();

      //Calculate Distance to Ball for Kick depedndant on current Position of Ball and ints Speed
      float minDistance = 1000;

      if (!footIsSelected) {  // select only once
        footIsSelected = true;
        leftFoot = theFieldBall.positionRelative.y() < 0;
      }

      KickInfo::KickType kickType = leftFoot ? KickInfo::forwardFastRight : KickInfo::forwardFastLeft;

      if (!isKicking) {
        if (calcDisrtacetoBall() <= minDistance) {

           //theGoToBallAndKickSkill(calcAngleToGoal(), kickType, false, 0.5f, true, false, Pose2f(0.5f,0.5f,0.5f), Rangea(20_deg,20_deg));
           //isKicking = true;
          
          
          theGoToBallAndDribbleSkill(calcAngleToGoal(), false, 1.f, false, false);
           //auto obstacleAvoidance = theLibWalk.calcObstacleAvoidance(theRobotPose, /* rough: */ true, /* disableObstacleAvoidance: */ true);
          //theWalkToBallAndKickSkill(calcAngleToGoal(), kickType, false, 0.1f, Pose2f(1.f, 1.f, 1.f), obstacleAvoidance, false, false, Rangea(40_deg, 40_deg));
           //theDribbleSkill(calcAngleToGoal(), Pose2f(1.f, 1.f, 1.f), obstacleAvoidance, false, 0.5f, false, false, Rangea(40_deg, 40_deg));
        }/*
        else if (intersectionwithownYAxis != Vector2f::Zero()) {

          //theWalkToKickoffPoseSkill(Pose2f(calcAngleToGoal(), intersectionwithownYAxis));


        }
        else if (inersectionwithOwnXAxis != Vector2f::Zero()) {

          //Go infronft of Rolling Ball in Preparation to Kick
          //theWalkToKickoffPoseSkill(Pose2f(calcAngleToGoal(), inersectionwithOwnXAxis));

        }
        else if (theFieldBall.isRollingTowardsOwnGoal) {
          //theWalkToKickoffPoseSkill(Pose2f(calcAngleToGoal(), theFieldBall.endPositionRelative));
        }*/
        else {

          theTurnAngleSkill(calcAngleToGoal());
        }
      }
      theLookAtBallSkill();
    }

    Angle calcAngleToGoal() const
    {
      return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
    }

    
    float calcDisrtacetoBall() const
    {
      Vector2f temp1 = theFieldBall.recentBallPositionRelative();
      float temp2 = temp1.x() * temp1.x();
      float temp3 = temp1.y() * temp1.y();
      return std::sqrt(temp2 + temp3);
    }
};

MAKE_CARD(ChallangeCard);
