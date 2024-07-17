#include "Representations/BehaviorControl/SACCommands.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"

CARD(SACCard,
{,
  CALLS(Activity),
  CALLS(LookForward),
  CALLS(Say),
  CALLS(Stand),
  CALLS(WalkAtRelativeSpeed),
  REQUIRES(SACCommands),
});

class SACCard : public SACCardBase
{
  bool preconditions() const override
  {
    return true;
  }

  bool postconditions() const override
  {
    return true;
  }

  void execute() override
  {
    theActivitySkill(BehaviorStatus::SAC);
    theLookForwardSkill();
    theStandSkill();
    
    // 0 = Standskill, 1 = Forward, 2 = Backward, 3 = Left, 4 = Right, 5 = Leftwards, 6 = Rightwards
    OUTPUT_TEXT("SAC direction" << theSACCommands.direction);
    if(theSACCommands.direction == 0)
      theStandSkill();
    else if(theSACCommands.direction == 1)
      theWalkAtRelativeSpeedSkill(Pose2f(0.f, 1.f, 0.f));
    else if(theSACCommands.direction == 2)
      theWalkAtRelativeSpeedSkill(Pose2f(0.f, -1.f, 0.f));
    else if(theSACCommands.direction == 3)
      theWalkAtRelativeSpeedSkill(Pose2f(0.f, 0.f, 1.f));
    else if(theSACCommands.direction == 4)
      theWalkAtRelativeSpeedSkill(Pose2f(0.f, 0.f, -1.f));
    else if(theSACCommands.direction == 5)
      theWalkAtRelativeSpeedSkill(Pose2f(1.f, 0.f, 0.f));
    else if(theSACCommands.direction == 6)
      theWalkAtRelativeSpeedSkill(Pose2f(-1.f, 0.f, 0.f));
    else
      OUTPUT_TEXT("Invalid direction");
    
  }
};

MAKE_CARD(SACCard);