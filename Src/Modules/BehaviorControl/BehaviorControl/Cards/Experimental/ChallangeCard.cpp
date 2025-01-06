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



//#include <filesystem>
#include "Tools/Modeling/BallPhysics.h"
#include "Tools/Math/Eigen.h"
#include "Tools/BehaviorControl/Interception.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h" 
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/TeamCommStatus.h"

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
       REQUIRES(FieldBall),
       REQUIRES(RobotPose),
       REQUIRES(RobotInfo),
       REQUIRES(FieldDimensions),
       REQUIERS(BallModel),


       DEFINES_PARAMETERS(
      {,
        (bool)(false) footIsSelected,  // freeze the first decision
        (bool)(true) leftFoot,
      }),
    });

class ChallangeCard : public ChallangeCardBase
{

    //always active
    bool preconditions() const override
    {
        return true;
    }

    bool postconditions() const override
    {
      return false;

    }

    void execute() override
    {
      theActivitySkill(BehaviorStatus::testingBehavior);

      Vector2f intersectionwithownYAxis = theFieldBall.intersectionPositionWithOwnYAxis;
      Vector2f inersectionwithOwnXAxis = Vector2f::Zero();

      //Calculate Distance to Ball for Kick depedndant on current Position of Ball and ints Speed
      float minDistance = theBallModel.estimate.velocity.x();

      if (!footIsSelected) {  // select only once
        footIsSelected = true;
        leftFoot = theFieldBall.positionRelative.y() < 0;
      }

      KickInfo::KickType kickType = leftFoot ? KickInfo::forwardFastLeft : KickInfo::forwardFastRight;


      if(RobotInfo::getTimeToReachBall() <= minDistance){

        theGoToBallAndKickSkill(calcAngleToGoal(), kickType);

      }else if (intersectionwithownYAxis != Vector2f::Zero()) {

        //theWalkToPointSkill(Pose2f(0_deg, ballpos), 0.7f, true, true, false, true);

        //theInterceptBallSkill((unsigned) bit(Interception::walk), false, false);

        theWalkToKickoffPoseSkill(Pose2f(0_deg, intersectionwithownYAxis));


      }
      else if (inersectionwithOwnXAxis != Vector2f::Zero()) {

        theWalkToKickoffPoseSkill(Pose2f(0_deg, inersectionwithOwnXAxis));

      }else if(theFieldBall.isRollingTowardsOwnGoal){
        theWalkToKickoffPoseSkill(Pose2f(0_deg, theFieldBall.endPositionRelative));
      }else{

        theLookAtBallSkill();
        theStandSkill();
      }

    }

    Angle calcAngleToGoal() const
    {
      return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
    }
};

MAKE_CARD(ChallangeCard);
