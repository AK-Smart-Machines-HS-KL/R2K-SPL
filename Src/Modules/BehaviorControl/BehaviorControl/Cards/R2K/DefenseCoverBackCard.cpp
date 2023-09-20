/**
 * @file DefenseCoverBackCard.cpp
 * @author Niklas Schmidts, Adrian Müller   
 * @brief Allows Offenseplayer to chase the Ball and kick to goal
 * @version 1.2
 * @date 2023-01-06
 * 
 * Functions, values, side effects: 
 * OffensePlayer tries to catch the ball (ie ´walks in this direction) if
 * - ball is nearer to opponent goal as his x postion (minus threshold)
 * - player is in range from middle line - threshold, or closer to opp. goal
 * - ignores thePlayerRole.playsTheBall()
 * 
 * 
 * Details
 * if bot is closest to ball (playsTheBall()) card ShootAtGoalCard will take over
 * * 
 * 
 * v1.1. avoid that our  offense bots struggle for ball. loop over buddies -> 
 *      if BehaviorStatus::DefenseCoverBackCard or clearOwnHalfCard or clearOwnHalfCardGoalie exit this card
 * 
 * v.1.2 card now checks wether there is an passing event active (OffenseForwardPassCard, OffenseReceivePassCard)
 * v 1.3: (Asrar) "theTeammateRoles.playsTheBall(&theRobotInfo, theTeamCommStatus.isWifiCommActive)"
          this is for online and offline role assignment
    
 * - Check: GoalShot has higher priority and takes over close to opp.goal
 * v 1.3 DEFENSE only x < 0 - threshold
 */

// Skills - Must be included BEFORE Card Base
#include "Representations/BehaviorControl/Skills.h"

// Card Base
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"

// Representations
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/Communication/TeamData.h"
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamCommStatus.h"



CARD(DefenseCoverBackCard,
     {
        ,
        CALLS(Activity),
        CALLS(LookForward),
        CALLS(GoToBallAndDribble),
        CALLS(WalkToPoint),
        CALLS(WalkAtRelativeSpeed),
        CALLS(LookAtBall),
        CALLS(Stand),
        USES(GameInfo),
        REQUIRES(ObstacleModel),
        REQUIRES(TeamBehaviorStatus),
        REQUIRES(RobotPose),
        REQUIRES(RobotInfo),
        REQUIRES(FieldBall),
        REQUIRES(FieldDimensions),
        REQUIRES(TeamData),   // check behavior
        REQUIRES(TeammateRoles),
        REQUIRES(TeamCommStatus),  // wifi on off?

        DEFINES_PARAMETERS(
             {,
                //Define Params here
                (float)(0.8f) walkSpeed,
                (int)(5000) ballNotSeenTimeout,
                (int)(1000) threshold,
             }),

     });

class DefenseCoverBackCard : public DefenseCoverBackCardBase
{

  bool preconditions() const override
  {  
    //Abfragen Spielerposition
   
    //Vergleich ob die Spielerposition in der Opponentside liegt
    //mit einem threshold damit Stürmer noch teils ins eigene Feld darf
    Vector2f ownGoal = Vector2f(theFieldDimensions.xPosOwnGroundLine, 0);
    float distToGoal = (ownGoal - theFieldBall.positionOnField).norm();

    return
      distToGoal > 1000 &&
      theGameInfo.setPlay == SET_PLAY_NONE &&
      aBuddyIsChasingOrClearing() &&
      theTeammateRoles.isTacticalDefense(theRobotInfo.number); // my recent role
      
  }

  bool postconditions() const override
  {
    return !preconditions();
  }

  void execute() override {
    Vector2f ownGoal = theRobotPose.toRelative(Vector2f(theFieldDimensions.xPosOwnGroundLine, 0));

    Vector2f ballToGoal = ownGoal - theFieldBall.positionRelative;
    Vector2f ballToGoalDirection = ballToGoal.normalized();

    Pose2f target = Pose2f(theFieldBall.positionRelative.angle(), theFieldBall.positionRelative + ballToGoalDirection * 600);

    theActivitySkill(BehaviorStatus::blocking);
    theLookAtBallSkill();
    
    if (target.translation.norm() > 600) {  // I am far
      theWalkToPointSkill(target); 
    } else if(target.translation.norm() > 100 || target.rotation > 10_deg) {    // I am close
      Pose2f normedTargetDirection = Pose2f(std::clamp((float) target.rotation, -1.0f, 1.0f) , target.translation.normalized());
      theWalkAtRelativeSpeedSkill(normedTargetDirection);
    } else { // I have arrived
      theStandSkill();
    }

  }

    Angle calcAngleToGoal() const
  {
    return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
  }

    Angle calcAngleToBall() const
  {
    return (theRobotPose.inversePose * Vector2f(theFieldBall.endPositionOnField.x(), theFieldBall.endPositionOnField.y())).angle();
  }

    bool aBuddyIsChasingOrClearing() const
    {
      for (const auto& buddy : theTeamData.teammates) 
      {
        if (buddy.theBehaviorStatus.activity == BehaviorStatus::defenseChaseBallCard ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::clearOwnHalfCard ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::clearOwnHalfCardGoalie ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::defenseLongShotCard ||
          buddy.theBehaviorStatus.activity == BehaviorStatus::goalieLongShotCard 
          )
          return true;
      }
      return false;
    }
};

MAKE_CARD(DefenseCoverBackCard);
