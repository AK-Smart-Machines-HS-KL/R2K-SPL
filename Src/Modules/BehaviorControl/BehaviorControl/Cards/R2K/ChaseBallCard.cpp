/**
 * @file ChaseBallCard.cpp
 * @author Niklas Schmidts, Adrian Müller   
 * @brief Allows Offenseplayer to chase the Ball and kick to goal
 * @version 1.0
 * @date 2022-09-25
 * 
 * Functions, values, side effects: 
 * OffensePlayer tries to catch the ball (ie ´walks in this direction) if
 * - ball is nearer to opponent goal as his x postion
 * - player is in range from middle line - threshold, or closer to opp. goal
 * - ignores thePlayerRole.playsTheBall()
 * 
 * 
 * Details
 * if bot is closest to ball (playsTheBall()) card ShootAtGoal will take over
 * * 
 * 
 * ToDo
 * - How to avoid that our two offense struggle for ball. Idee: loop over buddies -> exit this card 
 * - Check: ShootAtGoal has higher priority and takes over close to opp.goal
 */

// Skills - Must be included BEFORE Card Base
#include "Representations/BehaviorControl/Skills.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

// Representations
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/BehaviorControl/TeammateRoles.h"

//#include <filesystem>

CARD(ChaseBallCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookForward),
        CALLS(GoToBallAndDribble),
        CALLS(WalkAtRelativeSpeed),
        REQUIRES(RobotPose),
        REQUIRES(RobotInfo),
        REQUIRES(FieldBall),
        REQUIRES(FieldDimensions),
        REQUIRES(TeammateRoles),

        DEFINES_PARAMETERS(
             {,
                //Define Params here
                (float)(0.8f) walkSpeed,
                (int)(7000) ballNotSeenTimeout,
                (int)(1000) threshold,
             }),

        /*
        //Optionally, Load Config params here. DEFINES and LOADS can not be used together
        LOADS_PARAMETERS(
             {,
                //Load Params here
             }),
             
        */

     });

class ChaseBallCard : public ChaseBallCardBase
{

  bool preconditions() const override
  {  
    //Abfragen Spielerposition
   
    //Vergleich ob die Spielerposition in der Opponentside liegt
    //mit einem threshold damit Stürmer noch teils ins eigene Feld darf
  
    return 
      theTeammateRoles.isTacticalOffense(theRobotInfo.number) && // OFFENSE_RIGHT, OFFENSE_MIDDLE, OFFENSE_LEFT
      theRobotPose.translation.x() > (0 - threshold) &&
      theFieldBall.positionOnField.x() >= theRobotPose.translation.x();
  }

  bool postconditions() const override
  {
    return !preconditions();
  }

  option
  {
    theActivitySkill(BehaviorStatus::chaseBallCard);

   initial_state(goToBallAndDribble)
    {
      transition
      {
        if(!theFieldBall.ballWasSeen(ballNotSeenTimeout))
          goto searchForBall;
      }

        action
      {
        // theGoToBallAndKickSkill(calcAngleToGoal(), KickInfo::walkForwardsLeft);
        // SKILL_INTERFACE(GoToBallAndDribble, (Angle) targetDirection, (bool)(false) alignPrecisely, (float)(1.f) kickPower, (bool)(true) preStepAllowed, (bool)(true) turnKickAllowed, (const Rangea&)(Rangea(0_deg, 0_deg)) directionPrecision);

        theGoToBallAndDribbleSkill(calcAngleToGoal(),true);
      }
    }

    state(searchForBall)
    {
      transition
      {
        if(theFieldBall.ballWasSeen())
          goto goToBallAndDribble;
      }

      action
      {
        theLookForwardSkill();
        theWalkAtRelativeSpeedSkill(Pose2f(walkSpeed, 0.f, 0.f));
      }
    }
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

MAKE_CARD(ChaseBallCard);
