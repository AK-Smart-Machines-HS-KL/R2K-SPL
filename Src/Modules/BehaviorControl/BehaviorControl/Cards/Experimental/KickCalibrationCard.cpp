/**
 * @file KickCalibrationCard.cpp
 * @author Andy Hobelsberger    
 * @brief This Card is used to Collect data on kick Alignments 
 * @version 1.0
 * @date 2022-10-26
 * 
 * 
 */

#include "Representations/BehaviorControl/Skills.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

// Representations
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Modeling/RobotPose.h"

CARD(KickCalibrationCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookForward),
        CALLS(Stand),
        CALLS(GoToBallAndKick),
        CALLS(WalkToPoint),
        CALLS(LookActive),
        REQUIRES(RobotPose),
        REQUIRES(FieldBall),

        DEFINES_PARAMETERS(
             {,
                (Vector2f) (Vector2f(4000,0)) target,
                (Pose2f) (Pose2f(0_deg, -2000,0))setupPose,
                (KickInfo::KickType) (KickInfo::forwardFastRightLong) kick,
             }),
     });

class KickCalibrationCard : public KickCalibrationCardBase
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

    theActivitySkill(BehaviorStatus::testingBehavior);

    if (theFieldBall.positionRelative.norm() < 1000 && theFieldBall.ballWasSeen())
    {
      Angle targetDir = theRobotPose.toRelative(target).angle();
      theGoToBallAndKickSkill(targetDir, kick);
    } else {
      theWalkToPointSkill(theRobotPose.toRelative(setupPose));
      theLookActiveSkill();
    }
  }
};

MAKE_CARD(KickCalibrationCard);
