/**
 * @file R2K_TeamCard.cpp
 *
 * 
 *
 * @author Adrian Müller
 */

#include "Representations/BehaviorControl/TeamSkills.h"
#include "Tools/BehaviorControl/Framework/Card/TeamCard.h"


// ttrb
#include "Tools/Math/Geometry.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/TimeToReachBall.h"
#include "Representations/Communication/TeamData.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Tools/BehaviorControl/Framework/Card/TeamCard.h"
#include "Representations/Communication/GameInfo.h"  // to sync time inbetween bots


// roles and 
#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/Communication/RobotInfo.h"

TEAM_CARD(R2K_TeamCard,
{,
  CALLS(Role),
  CALLS(TeamActivity),
  CALLS(TeammateRoles),
  CALLS(TimeToReachBall),
  REQUIRES(FieldBall),  // ttrb
  REQUIRES(FrameInfo),  // ttrb
  REQUIRES(TeamData),   // ttrb
  REQUIRES(GameInfo),   // ttrb, check for state change
  REQUIRES(RobotInfo),  // roles

  DEFINES_PARAMETERS(
              {
                ,
                // (unsigned)(std::numeric_limits<unsigned>::max())offsetToFrameTimeThisBot,
                (unsigned)(0) offsetToFrameTimeThisBot,
    }),

});

class R2K_TeamCard : public R2K_TeamCardBase
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
    theTeamActivitySkill(TeamBehaviorStatus::R2K_NORMAL_GAME);  //default
  

  if (theGameInfo.state == STATE_READY)
      {
        // we will see READY for 45sec - when gone, we reach state PLAY == finish updating offset
        offsetToFrameTimeThisBot = theFrameInfo.time;
      }

    // theTimeToReachBallSkill(TimeToReachBall());
    TimeToReachBall timeToReachBall;
    int dist = Geometry::distance(theFieldBall.endPositionRelative, Vector2f(0, 0));
   // timeToReachBall.timeWhenReachBall = dist;
    // timeToReachBall.timeWhenReachBall = dist + theFrameInfo.time  // unfortunately, frame info is not synced globally 
    timeToReachBall.timeWhenReachBall = dist + theFrameInfo.time + offsetToFrameTimeThisBot;

    // later, taken from DribbleEngine.cpp
    // const float timeToReachBall = std::max(0.f, std::max(motionRequest.ballEstimate.position.x() / theWalkingEngineOutput.maxSpeed.translation.x(), motionRequest.ballEstimate.position.x() / theWalkingEngineOutput.maxSpeed.translation.x()) - minBallDistanceForVelocity);



    // get min time
    
    auto minTime = timeToReachBall.timeWhenReachBallStriker;
    if (timeToReachBall.timeWhenReachBall < minTime) minTime = timeToReachBall.timeWhenReachBall;   // i am closest
    for (const auto& buddy : theTeamData.teammates)
    {
      // if (buddy.theTeamBehaviorStatus.timeToReachBall.timeWhenReachBall < (minTime))
      if (buddy.theTeamBehaviorStatus.timeToReachBall.timeWhenReachBall < (minTime))
      {
        // buddy is closer
        minTime = buddy.theTeamBehaviorStatus.timeToReachBall.timeWhenReachBall;
        // timeToReachBall.timeWhenReachBallStriker = minTime;
      }
      
      timeToReachBall.timeWhenReachBallStriker = minTime;
      
    } // rof: scan team
    
    theTimeToReachBallSkill(timeToReachBall);

    theTeammateRolesSkill(TeammateRoles());
     
  
    
    if (theGameInfo.state != STATE_PLAYING)  // set defaults
    {
      PlayerRole pRole;
      pRole.numOfActiveSupporters = 5;
      if (1 == theRobotInfo.number) pRole.role = PlayerRole::RoleType::goalkeeper;
      theRoleSkill(pRole);
    }
    else 
      theRoleSkill(PlayerRole());
  }
};

MAKE_TEAM_CARD(R2K_TeamCard);
