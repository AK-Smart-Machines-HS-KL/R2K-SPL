/**
 * @file GoalieCard.cpp
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
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/BehaviorControl/FieldBall.h"

//#include <filesystem>

// Modify this card but don't commit changes to keep it clean for other developers
// Also don't forget to put this card at the top of your Card Stack!
CARD(GoalieCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookForward),
        CALLS(Stand),
        CALLS(WalkToPoint),
        CALLS(WalkAtRelativeSpeed),
        REQUIRES(RobotInfo),
        REQUIRES(RobotPose),
        REQUIRES(FieldDimensions),
        REQUIRES(FieldBall),


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

class GoalieCard : public GoalieCardBase
{

  //always active
  bool preconditions() const override
  {
    return theRobotInfo.number == 3;
  }

  bool postconditions() const override
  {
    return false;
  }

  void execute() override
  {

    theActivitySkill(BehaviorStatus::testingBehavior);
    Vector2f target((theFieldDimensions.xPosOwnGoalPost + 300.0f), std::max(theFieldDimensions.yPosRightGoal, std::min(theFieldDimensions.yPosLeftGoal, theFieldBall.positionOnField.y())));
    Vector2f targetRelative = theRobotPose.inversePose * target;

    theWalkAtRelativeSpeedSkill(Pose2f(theFieldBall.positionRelative.angle(), targetRelative.normalized()));
    
    

    // Override these skills with the skills you wish to test
    theLookForwardSkill(); // Head Motion Request
    //theStandSkill(); // Standard Motion Request

  }
};

MAKE_CARD(GoalieCard);
