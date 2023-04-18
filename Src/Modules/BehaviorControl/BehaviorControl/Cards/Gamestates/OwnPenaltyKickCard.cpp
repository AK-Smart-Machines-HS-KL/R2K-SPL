/**
 * @file OwnPenaltyKickCard.cpp
 * @author Asfiya Aazim
 * @brief Covers the Penalty Kick: Own Team has Penalty Kick
 * @version 1.1
 * @date 2023-04-18
 *
 * Notes:
 *  - Currently Triggers for all Robots. Use this Card as a template for preconditions
 *  - Currently only calls stand
 *
 * 
 */


#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"


#include "Representations/BehaviorControl/Skills.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"

#include "Representations/Configuration/FieldDimensions.h"

#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Communication/RobotInfo.h"

#include "Representations/Modeling/RobotPose.h"

#include "Tools/Math/Geometry.h"


CARD(OwnPenaltykKickCard,
    { ,
      CALLS(Stand),
      CALLS(Activity),
      CALLS(LookForward),
      CALLS(GoToBallAndKick),
      CALLS(GoToBallHeadControl),

      REQUIRES(FieldBall),
      REQUIRES(RobotPose),
      REQUIRES(RobotInfo),
      REQUIRES(FieldDimensions),
      REQUIRES(OwnTeamInfo),
      REQUIRES(GameInfo),
      REQUIRES(TeamBehaviorStatus),
      REQUIRES(TeammateRoles),

    });

class OwnPenaltykKickCard : public OwnPenaltykKickCardBase
{
    KickInfo::KickType kickType;
    /**
     * @brief The condition that needs to be met to execute this card
     */
    bool preconditions() const override
    {
        return theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber
            && theGameInfo.setPlay == SET_PLAY_PENALTY_KICK
            && theTeammateRoles.isTacticalDefense(theRobotInfo.number);

    }

    /**
     * @brief The condition that needs to be met to exit the this card
     */
    bool postconditions() const override
    {
        return !preconditions();
    }

    option
    {
      theActivitySkill(BehaviorStatus::ownPenaltyKick);

      initial_state(init)
      {

        transition
        {
           bool leftFoot = theFieldBall.positionRelative.y() < 0;
          kickType = leftFoot ? KickInfo::forwardFastLeft : KickInfo::forwardFastRight;
          goto active;
        }

        action
        {
          theLookForwardSkill();
          theStandSkill();
          theGoToBallHeadControlSkill(1.0,true, Vector2f::Zero());

        }
      }

      state(active)
      {
        transition
        {

        }

        action
        {
          theGoToBallAndKickSkill(calcAngleToGoal(), kickType);
        }
      }

      state(standby)
      {
        transition
        {

        }

        action
        {

        }
      }
    }
        Angle calcAngleToGoal() const
    {
        return (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundLine, 0.f)).angle();
    }
};

MAKE_CARD(OwnPenaltykKickCard);