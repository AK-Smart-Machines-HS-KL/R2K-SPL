/**
 * @file InitialCard.cpp
 *
 * This file specifies the behavior for a robot in the Initial game state.
 *
 * @author Arne Hasselbring
 */

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Communication/GameInfo.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"

#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Modeling/RobotPose.h"
#include "Platform/SystemCall.h"

bool awakeCalled = false;

CARD(InitialCard,
{,
  CALLS(Activity),
  CALLS(LookForward),
  CALLS(PointAt),
  CALLS(PointAtWithArm),
  CALLS(Stand),
  REQUIRES(GameInfo),
  REQUIRES(FieldBall),
  REQUIRES(RobotPose),
});

class InitialCard : public InitialCardBase
{
  bool preconditions() const override
  {
    return theGameInfo.state == STATE_INITIAL;
  }

  bool postconditions() const override
  {
    return theGameInfo.state != STATE_INITIAL;
  }

  void execute() override
  {
    theActivitySkill(BehaviorStatus::initial);
    // theLookAtAnglesSkill(0.f, 0.f, 150_deg);
    theLookForwardSkill();
    theStandSkill(/* high: */ true);

    Vector2f PointPosition = theFieldBall.recentBallPositionOnField();

    Vector2f relativePointPosition = theRobotPose.toRelative(PointPosition);
    Vector3f relativePointCoordinate3D(2 * relativePointPosition.x() , 2* relativePointPosition.y() , 0.f);
    //OUTPUT_TEXT("Point Position (x, y): " << relativePointPosition.x() << ", " << relativePointPosition.y());
    //thePointAtSkill(relativePointCoordinate3D);
    if (!awakeCalled){
        SystemCall::say("I'm wide awake!");
        awakeCalled = true;
        OUTPUT_TEXT("Awake call!");
    }
  }
};

MAKE_CARD(InitialCard);
