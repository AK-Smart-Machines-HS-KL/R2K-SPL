/**
 * @file GoalieCard.cpp
 * @author Benjamin Veit
 * @version 0.1
 * @date 2022-06-20
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

CARD(GoalieCard,
     {
         ,
         CALLS(Activity),
         CALLS(LookForward),
         CALLS(Stand),
         CALLS(WalkToPoint),
         CALLS(WalkAtRelativeSpeed),
         CALLS(TurnAngle),
         CALLS(GoToBallAndKick),
         CALLS(LookAtBall),
         REQUIRES(RobotInfo),
         REQUIRES(RobotPose),
         REQUIRES(FieldDimensions),
         REQUIRES(FieldBall),

         DEFINES_PARAMETERS(
             {
                 ,
                 // Define Params here
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

  // always active
  bool preconditions() const override
  {
    return theRobotInfo.number == 1;
  }

  bool postconditions() const override
  {
    return false;
  }

  option
  {
    theActivitySkill(BehaviorStatus::codeReleaseKickAtGoal);

    initial_state(moveToGoal)
    {
      Vector2f target((theFieldDimensions.xPosOwnGoalPost + 300.0f), std::max(theFieldDimensions.yPosRightGoal, std::min(theFieldDimensions.yPosLeftGoal, theFieldBall.positionOnField.y())));
      transition
      {

        //if ball is on the other field --> Goalie moving to the firstGoalieLine
        if (theFieldBall.positionOnField.x() > 600.0f)
        {
          goto goToFirstGoalieLine;
        }
        /*else if (theFieldBall.positionRelative.norm() < 150.0f)
        {
          goto kickBallIfToClose;
        }*/
      }

      action
      {
        // define x and y move cord for the goalie
        Vector2f targetRelative = theRobotPose.inversePose * target;

        // if the robot isnt at his target position --> he move to his target
        // if he is already at his position --> he will stand there and turn his head to the ball
        if ((target - theRobotPose.translation).norm() > 150)
        {
          theWalkAtRelativeSpeedSkill(Pose2f(theFieldBall.positionRelative.angle(), targetRelative.normalized()));
        }
        else
        {
          theTurnAngleSkill(theFieldBall.positionRelative.angle());
        }

        // turn the head to the ball
        theLookAtBallSkill();
      }
    }

    state(goToFirstGoalieLine)
    {
      // define x and y move cord for the goalie
      Vector2f target((theFieldDimensions.xPosOwnGoalPost + 1000.0f), std::max(theFieldDimensions.yPosRightGoal, std::min(theFieldDimensions.yPosLeftGoal, theFieldBall.positionOnField.y())));
      transition
      {
        // if the ball is closer then 0.6m to the middlecircle --> move to the goal position --> so he can block a shoot from the other field site
        if (theFieldBall.positionOnField.x() < 600.0f)
        {
          goto moveToGoal;
        }
      }

      action
      {

        Vector2f targetRelative = theRobotPose.inversePose * target;

        // if the robot isnt at the target position --> he move to this point
        // if he is already at his position --> he will stand there
        if ((target - theRobotPose.translation).norm() > 150)
        {
          theWalkAtRelativeSpeedSkill(Pose2f(theFieldBall.positionRelative.angle(), targetRelative.normalized()));
        }
        else
        {
          theTurnAngleSkill(theFieldBall.positionRelative.angle());
        }

        // move head to the ball
        theLookAtBallSkill();
      }
    }

    /*state(kickBallIfToClose)
    {
      transition
      {
        // if the ball is closer then 1meter --> move to the goal position
        if (theFieldBall.positionRelative.norm() > 150.0f)
        {
          goto moveToGoal;
        }
      }

      action
      {
        // if the robot isnt at the target position --> he move to this point
        // if he is already at his position --> he will stand there
        theGoToBallAndKickSkill();

        // Override these skills with the skills you wish to test
        theLookForwardSkill(); // Head Motion Request
      }
    }*/
  }
};

MAKE_CARD(GoalieCard);
