/**
 * @file DefenseCard.cpp
 * @author Benjamin Veit
 * @version 1.0
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
#include "Representations/Communication/TeamData.h"

//#include <filesystem>

CARD(DefenseCard,
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
  CALLS(LookActive),
  REQUIRES(RobotInfo),
  REQUIRES(RobotPose),
  REQUIRES(FieldDimensions),
  REQUIRES(FieldBall),
  REQUIRES(TeamData),
  
  DEFINES_PARAMETERS(
                     {
                       ,
                       // Define Params here
                     }),
  
});

class DefenseCard : public DefenseCardBase
{
  
  bool preconditions() const override
  {
    return theRobotInfo.number == 2 || theRobotInfo.number == 3;
  }
  
  bool postconditions() const override
  {
    return !preconditions();
  }
  
  option
  {
    theActivitySkill(BehaviorStatus::defenseCard);
    
    
    initial_state(identifierTimeToReachBall)
    {
      bool closest = checkDistanze();
      
      transition
      {
        // if the defense is closest
        if (closest == true && (theFieldBall.positionOnField.x() < 0))
        {
          goto walkToBallAndKickIt;
        }
      }

      action
      {
        
        theLookActiveSkill();
        theStandSkill();
      }
    }
    
    state(walkToBallAndKickIt){
      
      //checking if the currentRobot is the closest to the ball
      bool closest = checkDistanze();
      
      
      transition{
        if(theFieldBall.positionOnField.x() > 0) {
          goto identifierTimeToReachBall;
        }
        if (closest == false)
        {
          goto identifierTimeToReachBall;
        }
      }
      
      action{
        theGoToBallAndKickSkill((theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle(), KickInfo::forwardFastRightLong);
      }
      
    }

  }
  bool checkDistanze(){
    float myDist = theFieldBall.positionRelative.norm();
    
    for(auto& t : theTeamData.teammates) {
      if(t.number == 2 || t.number == 3){
        if(myDist > (t.theRobotPose.translation - theFieldBall.positionOnField).norm()) {
          return false;
        }
      }
    }
    return true;
  }
  
};

      
MAKE_CARD(DefenseCard);
