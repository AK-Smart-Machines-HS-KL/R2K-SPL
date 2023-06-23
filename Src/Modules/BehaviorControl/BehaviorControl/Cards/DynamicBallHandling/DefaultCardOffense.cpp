/**
 * @file DefaultCard.cpp
 * @author Asfiya
 * @brief This card's preconditions are always true. 
 *       
 * @version 0.1
 * @date 2023-06-18
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

CARD(DefaultCardOffense,
     {
        ,
        CALLS(Activity),
        CALLS(LookActive),
        CALLS(WalkToPoint),

        REQUIRES(DefaultPose),
        REQUIRES(RobotPose),

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

class DefaultCardOffense : public DefaultCardOffenseBase
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
    
    Vector2f targetRelative = theRobotPose.toRelative(theDefaultPose.ownDefaultPose.translation);

    theLookActiveSkill(); // Head Motion Request
    theWalkToPointSkill(Pose2f(theDefaultPose.ownDefaultPose.rotation - theRobotPose.rotation, targetRelative), 1.0f, true);     

  }
};

MAKE_CARD(DefaultCardOffense);
