/**
 * @file DefaultCard.cpp
 * @author Andy Hobelsberger    
 * @brief This card's preconditions are always true. 
 *        Edit it for testing
 * @version 0.1
 * @date 2021-05-23
 * 
 * 
 */

// Skills - Must be included BEFORE Card Base
#include "Representations/BehaviorControl/Skills.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

// Representations
#include "Representations/BehaviorControl/DefaultPose.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/BehaviorControl/FieldBall.h"


//#include <filesystem>

// Modify this card but don't commit changes to keep it clean for other developers
// Also don't forget to put this card at the top of your Card Stack!
CARD(DefaultCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookActive),
        CALLS(WalkToPoint),
        CALLS(GoToBallAndDribble),
        CALLS(LookAtBall),

        REQUIRES(DefaultPose),
        REQUIRES(RobotPose),
        REQUIRES(FieldBall),
        REQUIRES(FieldDimensions),

        DEFINES_PARAMETERS(
             {,
                //Define Params here
             }),

        /*
        //Optionally, Load Config params here. DEFINES and LOADS can not be used together
        LOADS_PARAMETERS(
             {,
                //Load Params here
             }),
             
        */

     });

class DefaultCard : public DefaultCardBase
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
    /*
        Previos Default Behavior

    theActivitySkill(BehaviorStatus::defaultBehavior);
    
    Pose2f targetRelative = theRobotPose.toRelative(theDefaultPose.ownDefaultPose);

    theLookActiveSkill(); // Head Motion Request
    theWalkToPointSkill(targetRelative, 1.0f, true);     
    */

    theActivitySkill(BehaviorStatus::defaultBehavior);

    theLookAtBallSkill();
    theGoToBallAndDribbleSkill(calcAngleToGoal());
  }

  Angle calcAngleToGoal() const
  {
    return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
  }

    Angle calcAngleToBall() const
  {
    return (theRobotPose.inversePose * Vector2f(theFieldBall.endPositionOnField.x(), theFieldBall.endPositionOnField.y())).angle();
  }
};

 

MAKE_CARD(DefaultCard);
