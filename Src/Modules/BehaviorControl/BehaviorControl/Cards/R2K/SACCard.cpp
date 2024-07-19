
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Communication/TeamData.h"
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
  CALLS(GoToBallAndKick),
  CALLS(GoToBallAndDribble),
  CALLS(GetUpEngine),
  REQUIRES(TeamData),
  REQUIRES(RobotPose),
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
    
    // 0 = Standskill, 1 = Forward, 2 = Backward, 3 = Left, 4 = Right, 5 = Leftwards, 6 = Rightwards, 7 = Kick, 8 = Dribble
    if(theSACCommands.direction == 0)
      theStandSkill();
    
    else if(theSACCommands.direction == 1)
    {
      theWalkAtRelativeSpeedSkill(Pose2f(0.f, 1.f, 0.f));
    }
    else if(theSACCommands.direction == 2)
    {
      theWalkAtRelativeSpeedSkill(Pose2f(0.f, -1.f, 0.f));
    }
    else if(theSACCommands.direction == 3)
    {
      theWalkAtRelativeSpeedSkill(Pose2f(0.f, 0.f, 1.f));
    }
    else if(theSACCommands.direction == 4)
    {
      theWalkAtRelativeSpeedSkill(Pose2f(0.f, 0.f, -1.f));
    }
    else if(theSACCommands.direction == 5)
    {
      theWalkAtRelativeSpeedSkill(Pose2f(1.f, 0.f, 0.f));
    }
    else if(theSACCommands.direction == 6)
    {
      theWalkAtRelativeSpeedSkill(Pose2f(-1.f, 0.f, 0.f));
    }
    else if(theSACCommands.direction == 7)
    {
      for (const auto& buddy : theTeamData.teammates)
      {
        Vector2f target = Vector2f::Zero();
        target = buddy.theRobotPose.translation; 
        int buddyDist = (int)Geometry::distance(target, theRobotPose.translation);
                 
        theGoToBallAndKickSkill(0, KickInfo::forwardFastLeft, true, buddyDist);
      }
    }
    else if(theSACCommands.direction == 8)
      theGoToBallAndDribbleSkill(0);
    else if (theSACCommands.direction == 9)
      theGetUpEngineSkill();   
  }
};

MAKE_CARD(SACCard);
