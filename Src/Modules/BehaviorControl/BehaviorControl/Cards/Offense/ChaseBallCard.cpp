/**
 * @file ChaseBallCard.cpp
 * @author Niklas Schmidts   
 * @brief Allows Offenseplayer to chase the Ball and kick to goal
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
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/PlayerRole.h"

//#include <filesystem>

CARD(ChaseBallCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookForward),
        CALLS(Stand),
        CALLS(GoToBallAndKick),
        CALLS(WalkAtRelativeSpeed),
        REQUIRES(RobotPose),
        REQUIRES(FieldDimensions),
        REQUIRES(RobotInfo),
        REQUIRES(FieldBall),
        REQUIRES(PlayerRole),

        DEFINES_PARAMETERS(
             {,
                //Define Params here
                (float)(0.8f) walkSpeed,
                (int)(1000) initialWaitTime,
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
    Vector2f pos = theRobotPose.translation;
    float x_pos = pos.x();

    //Vergleich ob die Spielerposition in der Opponentside liegt
    //mit einem threshold damit Stürmer noch teils ins eigene Feld darf
    bool opField = x_pos > 0 - threshold;

    return thePlayerRole.playsTheBall()&&opField;
  }

  bool postconditions() const override
  {
    return !preconditions();
  }

  option
  {
    theActivitySkill(BehaviorStatus::codeReleaseKickAtGoal);

    initial_state(start)
    {
      transition
      {
        if(state_time > initialWaitTime)
          goto goToBallAndKick;
      }

      action
      {
        theLookForwardSkill();
        theStandSkill();
      }
    }

    state(goToBallAndKick)
    {
      transition
      {
        if(!theFieldBall.ballWasSeen(ballNotSeenTimeout))
          goto searchForBall;
      }

      action
      {
        theGoToBallAndKickSkill(calcAngleToGoal(), KickInfo::walkForwardsLeft);
      }
    }

    state(searchForBall)
    {
      transition
      {
        if(theFieldBall.ballWasSeen())
          goto goToBallAndKick;
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
};

MAKE_CARD(ChaseBallCard);
