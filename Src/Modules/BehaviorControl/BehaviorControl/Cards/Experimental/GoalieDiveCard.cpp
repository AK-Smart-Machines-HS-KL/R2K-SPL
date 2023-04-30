/**
 * @file GoalieDiveCard.cpp
 * @author Asfiya Aazim & Mohammed
 * @brief This card's preconditions are true only when the ball is rolling towards own goal.
 * @Details:
 * - this card qualifies for only robot num1 GOALIE player if the ball is rolling towards own goal,
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
      && theFieldDimensions.xPosOwnPenaltyMark - 150 < theFieldBall.positionOnField.x();
 
  }

  bool postconditions() const override
  {
    return  !theFieldBall.isRollingTowardsOwnGoal;  //Only stop the skill when the ball is not rolling towards own goal
  }

  void execute() override
  {

    theActivitySkill(BehaviorStatus::dive);
    // std::string s = "dive";
    // OUTPUT_TEXT(s);
    // goalie can dive in both left & right directions
    // after dive the goalie will stand

    theInterceptBallSkill((unsigned)(bit(Interception::jumpRight) | bit(Interception::jumpLeft) | bit(Interception::walk) | bit(Interception::stand) | bit(Interception::genuflectStand)));
  }
};

MAKE_CARD(GoalieDiveCard);