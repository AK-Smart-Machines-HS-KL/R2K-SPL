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


#include "Representations/BehaviorControl/FieldBall.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

// Representations
#include "Representations/BehaviorControl/DefaultPose.h"
#include "Representations/Modeling/RobotPose.h"

//#include <filesystem>

// Modify this card but don't commit changes to keep it clean for other developers
// Also don't forget to put this card at the top of your Card Stack!
CARD(DefaultCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookActive),
        CALLS(WalkToPoint),
        CALLS(GoToBallAndKick),
        REQUIRES(FieldBall),

        REQUIRES(DefaultPose),
        REQUIRES(RobotPose),

        DEFINES_PARAMETERS(
             {,
                //Define Params here
                (bool)(false) footIsSelected,  // freeze the first decision
                (bool)(true) leftFoot,
                (bool)(true) shootAngleIsZero,
                (int) (-1000) offsetX,
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
    theActivitySkill(BehaviorStatus::defaultBehavior);
    /*
    int shootAngle = 0;
    if (!footIsSelected) {  // select only once
      footIsSelected = true;
      leftFoot = theFieldBall.positionRelative.y() < 0; //TODO: Das muss bereits vom gewünschten winkel abhängen
    }
    if(leftFoot) {
        theGoToBallAndKickSkill(shootAngle, KickInfo::diagonalFastLeft);  
    }
    else {
        theGoToBallAndKickSkill(shootAngle, KickInfo::diagonalFastRight);
    }
    //-----------------------------------------------------

    
    */
    Pose2f targetRelative = theRobotPose.toRelative(theDefaultPose.ownDefaultPose);

    theLookActiveSkill(); // Head Motion Request
    theWalkToPointSkill(targetRelative, 1.0f, true);
  }
};

MAKE_CARD(DefaultCard);
