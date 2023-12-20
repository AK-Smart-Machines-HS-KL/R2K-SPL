/**
 * @file TestCard.cpp
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
#include "Representations/Communication/GameInfo.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

// Representations
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Modeling/BallModel.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/MotionControl/ArmMotionRequest.h"
#include <Modules/Modeling/OracledWorldModelProvider/OracledWorldModelProvider.h>

//#include <filesystem>

// Modify this card but don't commit changes to keep it clean for other developers
// Also don't forget to put this card at the top of your Card Stack!
CARD(TestCard,
  {
     ,
     CALLS(Activity),
     CALLS(LookForward),
     CALLS(Stand),
     CALLS(PointAt),

     REQUIRES(GameInfo),

     DEFINES_PARAMETERS(
          {,
       //bool ballSeen;
             }),

        /*
        //Optionally, Load Config params here. DEFINES and LOADS can not be used together
        LOADS_PARAMETERS(
             {,
                //Load Params here
             }),
             
        */

     });

class TestCard : public TestCardBase
{

  //always active
  bool preconditions() const override

  {
    return theGameInfo.state == STATE_PLAYING;
    //return  theGameInfo.state == STATE_PLAYING && theGameInfo. ;
  }

  bool postconditions() const override
  {

    return theGameInfo.state != STATE_PLAYING;   // set to true, when used as default card, ie, lowest card on stack
    //return theGameInfo.state != STATE_PLAYING && theGameInfo.gamePhase != ;
  }

  void execute() override
  {

    theActivitySkill(BehaviorStatus::testingBehavior);
    // std::string s = "testingBehavior";
    //bool ballSeen = theFrameInfo.getTimeSince(theBallModel.timeWhenLastSeen) < 250;

    //if (ballSeen)
      OUTPUT_TEXT("PLAYING");
      PointAt;
      //PointAtWithArm, (const Vector3f&) localPoint, (Arms::left));
      //PointAtWithArm, (const Vector3f&)localPoint, (Arms::right));
    // Override these skills with the skills you wish to test
    theLookForwardSkill(); // Head Motion Request
    theStandSkill(); // Standard Motion Request

  }
};

MAKE_CARD(TestCard);
