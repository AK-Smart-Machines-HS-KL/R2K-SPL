/**
/**
 * @file DefenseLongShotCard.cpp
 * @author Nicholas Pfohlmann, Adrian Müller 
 * @version: 1.0
 *
 * Functions, values, side effects: this card qualifies for the one DEFENSE player only who is closest to the ball
 * Details: 
 * Purpose of this card is to clear the own field as fast as possible.
 * KISS applies ;-) 
 * Shooting the ball straight (ie keeping the y- axis) to opppent half
 * After kick:
 * - a) card exists if role no longer is playsTheBall(), Problem: opponet might interfere 
 * - b) exit after kick (isDone()) to avoid defenser player chase the ball 
 * 
 * Note: 
 * 
 * 
 * ToDo: 
 * check for free shoot vector and opt. change y-coordinate
 * check whether isDone () works correctly 
 */
#

// B-Human includes
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Modeling/RobotPose.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Tools/Math/BHMath.h"

// this is the R2K specific stuff
#include "Representations/BehaviorControl/TeamBehaviorStatus.h" 
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/Communication/RobotInfo.h"

CARD(DefenseLongShotCard,
  { ,
    CALLS(Activity),
    CALLS(LookForward),
    CALLS(Stand),
    CALLS(WalkToBallAndKick),
    REQUIRES(FieldBall),
    REQUIRES(FieldDimensions),
    REQUIRES(PlayerRole),  // R2K
    REQUIRES(RobotInfo),
    REQUIRES(RobotPose),
    REQUIRES(TeammateRoles),  // R2K
    DEFINES_PARAMETERS(
    {,
      (float)(0.8f) walkSpeed,
    }),
  });

class DefenseLongShotCard : public DefenseLongShotCardBase
{
  bool preconditions() const override
  {
    return
      thePlayerRole.playsTheBall() &&  // I am the striker
      theTeammateRoles.isTacticalDefense(theRobotInfo.number); // my recent role
  }

  bool postconditions() const override
  {
    return !preconditions();
  }

 
  void execute() override
  {

    theActivitySkill(BehaviorStatus::defenseLongShotCard);
 
    // Override these skills with the skills you wish to test
    theLookForwardSkill(); // Head Motion Request
    // theWalkToBallAndKickSkill(0, , MotionRequest::ObstacleAvoidance());
    theStandSkill();
  }
};

MAKE_CARD(DefenseLongShotCard);
