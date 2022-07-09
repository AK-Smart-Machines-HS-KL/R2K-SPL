/**
 * @file R2K_TeamCard.cpp
 * @author Adrian Müller
 * @version: 1.0
 * 
 * functions, values set:
 * timeToReachBall is computed per bot - the resulting time frame ist NOT synced - BUT:
 * - we compute a unique answer for playsTheBall() (ie "striker") by
 *    - looping over all buddies, computing distance bot - ball
 *    - set role = PlayerRole::ballPlayer (resp. PlayerRole::goalkeeperAndBallPlayer) for bot with min distance
 *   - playsTheBall() (ie who is the striker)
 *   - isGoalkeeper() ( w/o playsTheBall() set)
 * - TeamBehaviorStatus::R2K_NORMAL_GAME;  default setting
 * - TeammateRoles: just the default values, ie., for 1..5: goalie, defense, defense, offense, offense
 *
 * 
 * Note: accuracy of playsTheBall() is dependend on EBC update frequency AND correctness of self localization of buddies 
 * 
 * To Do: 
 * - EBC: broadcast, when striker changes
 * - compute numOfActiveSupporters by x position
 * - set TeamBehaviorStatus dynamically
 * - change goalkeeper, if #1 is penalized
 * - read real field dimensions from config
 *
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


// roles 
#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/Communication/RobotInfo.h"

#include <algorithm>  // min()
#include "Representations/BehaviorControl/TeammateRoles.h"  // GOALKEEPER, DEFENSE,...

TEAM_CARD(R2K_TeamCard,
  { ,
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
                  (unsigned)(0) offsetToFrameTimeThisBot,  // unused yet
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
    return false;
  }

  void execute() override
  {

    theTeamActivitySkill(TeamBehaviorStatus::R2K_NORMAL_GAME);  //default


    // default settings for role[] as defined in TeammateRoles.h : 
    // TeammateRoles::GOALKEEPER,
    // TeammateRoles::DEFENSE,
    //  TeammateRoles::DEFENSE,
    //  TeammateRoles::OFFENSE,
    //  TeammateRoles::OFFENSE,
    //  TeammateRoles::UNDEFINED
    // 
    // to do: change roles dynamically
    TeammateRoles teamMateRoles;
    theTeammateRolesSkill(teamMateRoles);  

    // not used yet: sync frame time between bots
    // we need STATE_INITIAL because Nao will jump to STATE_PLAYING, when chest button is pressed (unless we have the game controller available)
    // 
    // Then we can go like this: we will see READY for 45sec - when gone, we reach state PLAY == finish updating offset
    if (theGameInfo.state == STATE_INITIAL)
    {
      offsetToFrameTimeThisBot = theFrameInfo.time;
    }



    // later, taken from DribbleEngine.cpp
    // const float timeToReachBall = std::max(0.f, std::max(motionRequest.ballEstimate.position.x() / theWalkingEngineOutput.maxSpeed.translation.x(), motionRequest.ballEstimate.position.x() / theWalkingEngineOutput.maxSpeed.translation.x()) - minBallDistanceForVelocity);

    PlayerRole pRole;

    switch (theRobotInfo.number) {
    case 1:
      pRole.role = PlayerRole::goalkeeper;
      break;
      // 2-5: unused yet
    case 2:
      pRole.role = PlayerRole::supporter1;
      break;
    case 3:
      pRole.role = PlayerRole::supporter2;
      break;
    case 4:
      pRole.role = PlayerRole::supporter3;
      break;
    case 5:
      pRole.role = PlayerRole::supporter4;
      break;
    default:
      pRole.role = PlayerRole::none;
    }

    // ToDo: compute by x position
    pRole.numOfActiveSupporters = (-1 + theRobotInfo.number); // nasty hack  

    // get min distance, set playsTheBall(); updates per frame 
    auto dist = 9000;  // max - real field dimensions should be read from config
    auto minDist = 0;
    auto buddyDist = 9000;

    // to be on the safe
    if (theFieldBall.ballWasSeen())
      dist = Geometry::distance(theFieldBall.endPositionRelative, Vector2f(0, 0));
    
    // B -Human code was:
    // theTimeToReachBallSkill(TimeToReachBall());

    TimeToReachBall timeToReachBall;
    // this code fragment does NOT sync teamwiese
    timeToReachBall.timeWhenReachBall = dist + theFrameInfo.time + offsetToFrameTimeThisBot;

    minDist = dist;
        
    for (const auto& buddy : theTeamData.teammates)
    {  // compute and compare my buddies distance with minimal distance
      minDist = std::min(minDist, buddyDist = Geometry::distance(theFieldBall.endPositionOnField, buddy.theRobotPose.translation));
    } // rof: scan team
      

    // ToDo EBC: code to check for striker change, then broadcast
    if (minDist == dist) {
      if (pRole.isGoalkeeper()) pRole.role = PlayerRole::goalkeeperAndBallPlayer;
      else pRole.role = PlayerRole::ballPlayer;

      timeToReachBall.timeWhenReachBallStriker = timeToReachBall.timeWhenReachBall;  // to: get sync working
 
    }

    theRoleSkill(pRole);
    theTimeToReachBallSkill(timeToReachBall);
  }  // execute
};

MAKE_TEAM_CARD(R2K_TeamCard);
