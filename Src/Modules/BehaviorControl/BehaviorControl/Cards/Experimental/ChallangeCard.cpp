/**
 * @file ChallangeCard.cpp
 * @author Dennis Fuhrmann
 * @brief Card for R2k Ball Challange 2025
 *        Currently still in its infancy
 * @version 0.1
 * @date 2024-16-12
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
       REQUIRES(FieldBall),
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

      Vector2f ballpos =  theFieldBall.recentBallPositionRelative();

      Vector2f intersectionwithownYAxis = theFieldBall.intersectionPositionWithOwnYAxis;

      if (intersectionwithownYAxis != Vector2f::Zero()) {

        //theWalkToPointSkill(Pose2f(0_deg, ballpos), 0.7f, true, true, false, true);

        theInterceptBallSkill((unsigned) bit(Interception::walk), false, false);


      }else{

        //theWalkToPointSkill(Pose2f(45_deg, ballpos), 0.7f, true, true, false, true);

        theLookAtBallSkill();
        theStandSkill();

      }

    }
};

MAKE_CARD(ChallangeCard);
