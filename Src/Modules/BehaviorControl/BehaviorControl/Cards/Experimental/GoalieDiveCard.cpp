/**
 * @file GoalieDiveCard.cpp
 * @author Asfiya Aazim & Mohammed
 * @brief This card's preconditions are true only when the ball is rolling towards own goal.
 * @Details:
 * - this card qualifies for only robot num1 GOALIE player if the ball is rolling towards own goal,
 * - the groundline + ball position <= -150,
 * - goalie will not go beyond penalty mark
 * - after the dive the goalie will stand.
 
 * @version 1.0
 * @date 2023-04-05
 
 */

 // Skills - Must be included BEFORE Card Base
#include "Representations/BehaviorControl/Skills.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Representations/Configuration/FieldDimensions.h"

// Representations
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Communication/RobotInfo.h"



CARD(GoalieDiveCard,
  {
     ,
     CALLS(Activity),
     CALLS(InterceptBall),
     REQUIRES(FieldBall),
     REQUIRES(RobotInfo),
     REQUIRES(FieldDimensions),

  });

class GoalieDiveCard : public GoalieDiveCardBase
{

  
  bool preconditions() const override
  {
    return theFieldBall.isRollingTowardsOwnGoal && theRobotInfo.number == 1
      && theFieldDimensions.xPosOwnPenaltyMark + theFieldBall.positionOnField.x() <= -150;
   /* OUTPUT_TEXT(s);
       the groundline + ball position <= -150
      -450+300 =-150
      -450+400 =-50 
   */ 
  }

  bool postconditions() const override
  {
    return  !preconditions();  
  }

  void execute() override
  {

    theActivitySkill(BehaviorStatus::dive);
    // std::string s = "dive";
    // OUTPUT_TEXT(s);
    // goalie can dive in both left & right directions
    // after the dive the goalie will stand

    theInterceptBallSkill((unsigned)(bit(Interception::jumpRight) | bit(Interception::jumpLeft) | bit(Interception::stand)));
  }
};

MAKE_CARD(GoalieDiveCard);