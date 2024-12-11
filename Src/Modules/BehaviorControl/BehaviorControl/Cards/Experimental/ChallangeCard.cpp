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

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Representations/Configuration/FieldDimensions.h"

// Representations
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Communication/RobotInfo.h"

//#include <filesystem>

// Modify this card but don't commit changes to keep it clean for other developers
// Also don't forget to put this card at the top of your Card Stack!
CARD(ChallangeCard,
    {
       ,
       CALLS(Activity),
       CALLS(LookForward),
       CALLS(Stand),
       REQUIRES(FieldBall),
    });

class ChallangeCard : public ChallangeCardBase
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



    }
};

MAKE_CARD(ChallangeCard);
