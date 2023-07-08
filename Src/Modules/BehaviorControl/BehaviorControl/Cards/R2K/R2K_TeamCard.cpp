/**
 * @file R2K_TeamCard.cpp
 * @author Adrian Müller
 * @version: 1.3
 * @date: 9/2022
 * 
 * 
 * v1.0
 * functions, values set:
 * - TeamBehaviorStatus::R2K_NORMAL_GAME;  default settings
 * - TeammateRfoutppoles: just the default values, ie., for 1..5: goalie, defense, defense, offense, offense
 * - playsTheBall() (ie who is the striker)
 * - isGoalkeeper() (wether he playsTheBall() or not)
 *   - theCaptain stores the robot number, who is currently acting as goalkeeper (this will be changed in v13)
 * 
 * Notes:
 * - timeToReachBall is computed per bot - the resulting time frame ist NOT synced - BUT:
 * - we compute a unique answer for playsTheBall() (ie "striker") by
 *    - looping over all buddies, computing distance bot - ball
 *    - set role = PlayerRole::ballPlayer (resp. PlayerRole::goalkeeperAndBallPlayer) for bot with min distance
 *   
 * - accuracy of playsTheBall() is dependend on EBC update frequency AND correctness of self localization of buddies
 * - see ReferenceCard.cpp for usage
 * 
 * 
 * v1.1
 * - TeamBehaviorStatus::R2K_SPARSE_MODE iff opp. team has >= 3 penalties OR (opp has >= 2 penalties AND own penalties >=1)
 * - if not SPARSE: TeamBehaviorStatus::R2K_NORMAL_GAME / OFFENSIVE / DEFENSIVE is set due to score difference (tie, back, lead)
 *  - ToDo:
 *    - add time limits to switch mode strategically when game is about to end (switch to DEFENSIVE)
 *    - take into account secsTillUnpenalized for SPARSE mode (ie, adapt strategie ahead)
 * 
 *  - computes role.supportIndex(), role.numOfActiveSupporters
 *   - Note: supportIndex() == 0 means left-most player; supportIndex() == numOfActiveSupporter you are right most player
 * 
 * - if goalie is !PLAYIiNG: dynamically assigns left-most bot as substitute, sync this temp. assignment with thePlayerRole.isGoalkeeper()
 *   - NOTE: "dynamically" means: this role assiginment may change at any time, when relative positions change 
 * 
 * v1.2
 * - more sophisticated computing of TeammateRoles (take into account, which bot is penaliezed)
 *  - static assignement STATE_INITIAL, 
 *  - else: dynamic (re-)assignment for 8 TeammateRoles  
 * - provide wrapper functions (see TeammateRoles)
 *
 * v1.3 
 * - new, different update policies for  PlayerRole, TimeToReachBall and TeammateRoles
 * - TeammateRoles: triggered by changes in GameInfo.state, TeamBehaviorStatus, numOfActiveSupporters (eg penalized, fallen),
 * - PlayerRole, TimeToReachBall: if the striker changes, only this information gets updated. Update frequency is limited to 2secs
 *      
 * v.1.4
 * - PlayerRole  goalkeeper,ballPlayer, goalkeeperAndBallPlayer vanished, TeammateRoles captain is used (stores strikers number)
 *
 * v1.5
 * - code clean up 
 * - fixed error in defaultPoseProvider.cfg (+y is left, -y is right. Adjusted and optimized r2k_tactics[][]
 * - dynamic role assignment is coupled to change in #penalized bots; if this is unchanged, we use last lineUp
 * 
 * v.1.6: several HOT FIX on GORE 23: disable computing of roles, tactics, ...
 * v 1.7. (Adria ) reverting hot fixes, code clean-up, dealing with incomplete TeamData, TeamcommStatus offline triggers static role assignment
 * 
 * 
 * 
 * ToDo
 * - EBC: broadcast, when striker changes etc.

 * - read real field dimensions from config
 *  - static assignement for state FINISHED
 *
*/

#include "Representations/BehaviorControl/TeamSkills.h"
#include "Tools/BehaviorControl/Framework/Card/TeamCard.h"

#include <algorithm>  // find
#include <vector>


// #include "Tools/Debugging/Annotation.h"


// ttrb, isPlayBall
#include "Tools/Math/Geometry.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/TimeToReachBall.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"  
#include "Representations/Communication/TeamData.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Tools/BehaviorControl/Framework/Card/TeamCard.h"
#include "Representations/Communication/GameInfo.h"  // to sync time inbetween bots

#include "Representations/Communication/EventBasedCommunicationData.h"



// roles 
#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/Communication/RobotInfo.h"

#include "Representations/Modeling/RobotPose.h" // supporterindex
#include <algorithm>  // min()
#include "Representations/BehaviorControl/TeammateRoles.h"  // GOALKEEPER, DEFENSE,...
#include "Representations/Communication/TeamInfo.h"         // access scores, OwnTeamInfo, OppTeamInfo
#include "Representations/Communication/TeamCommStatus.h"


#define GN TeammateRoles::GOALKEEPER_NORMAL
#define GA TeammateRoles::GOALKEEPER_ACTIVE

#define DL TeammateRoles::DEFENSE_LEFT
#define DM TeammateRoles::DEFENSE_MIDDLE
#define DR TeammateRoles::DEFENSE_RIGHT

#define OL TeammateRoles::OFFENSE_LEFT
#define OM TeammateRoles::OFFENSE_MIDDLE
#define OR TeammateRoles::OFFENSE_RIGHT

#define UN TeammateRoles::UNDEFINED


TEAM_CARD(R2K_TeamCard,
  { ,
    // CALLS(AnnotationManager),
    CALLS(Role),
    CALLS(TeammateRoles),
    CALLS(TimeToReachBall),
    REQUIRES(FieldBall),  // ttrb
    REQUIRES(FrameInfo),  // ttrb
    REQUIRES(TeamData),   // ttrb
    REQUIRES(GameInfo),   // ttrb, check for state change
    REQUIRES(RobotInfo),  // roles
    REQUIRES(RobotPose),  // supporterindex
    REQUIRES(TeamCommStatus),
    // USES(TeamBehaviorStatus),   // to be tested
    CALLS(TeamActivity),
    REQUIRES(OwnTeamInfo),    // score, penalty
    REQUIRES(OpponentTeamInfo),  // score, penalty
    REQUIRES(EventBasedCommunicationData),  // R2K EBC handling

    DEFINES_PARAMETERS(
                {
                  ,
                  // (unsigned)(std::numeric_limits<unsigned>::max())offsetToFrameTimeThisBot,
                  (bool)    (true)                     refreshAllData, // true so first computation is triggered
                  (unsigned) (STATE_INITIAL)           lastGameState,
                  (unsigned) (SET_PLAY_NONE)           lastGamePhase,
                  (int)(-1)                            lastTeamBehaviorStatus, // -1 means: not set yet
                  (int)(2000)                          decayPlaysTheBall,
                  (unsigned)(0)                        playsTheBallHasChangedFrame,   // store the frame when this bot claims to be playing the ball
                  (TeammateRoles)(TeammateRoles())     lastTeammateRoles,
                  (TimeToReachBall)(TimeToReachBall()) lastTimeToReachBall,
                  (PlayerRole)(PlayerRole())           lastPlayerRole,
                  (int)(-1)                            lastNrOwnPenalties,  // -1 means: not set yet
                  // (std::vector<int>) ({1,2,3,4,5 })    lineUp,  //defaults; later vector is used to buffer the actual line up of the five bots
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

private:
  int myEbcWrites = 0;  // tnmp. hack for tracing ebc
  bool recomputeLineUp = false; // check for fresh penalties
  std::vector<int> lineUp = {1,2,3,4,5};

  void execute() override
  {

    // this tactic table is used in step d4 below. It becomes active for STATE_PLAYING
     //         nr of active players x team tactic x 5 TeamMate roles[]
    const int r2k_tactics[5][TeamBehaviorStatus::numOfTeamActivities][5] =
    {
      //      R2K_NORMAL_GAME, R2K_DEFENSIVE_GAME,R2K_OFFENSIVE_GAME, R2K_SPARSE_GAME

      // 1 player
        { {GN,UN,UN,UN,UN}, {GN,UN,UN,UN,UN}, {GN,UN,UN,UN,UN}, {OM,UN,UN,UN,UN} },
        // 2 player
            { {GN,OM,UN,UN,UN}, {GN,OM,UN,UN,UN}, {GN,OM,UN,UN,UN}, {DM,OM,UN,UN,UN} },
            // 3 player
                { {GN,DM,OM,UN,UN}, {GN,DR,DM,UN,UN}, {GA,DM,OM,UN,UN}, {GN,OR,OM,UN,UN} },
                // 4 player
                    { {GN,DM,OR,OM,UN}, {GN,DR,DL,OM,UN}, {GA,DM,DL,OM,UN}, {GN,DM,OL,OM,UN} },
                    // 5 player
                        { {GN,DR,DL,OR,OL}, {GN,DR,DL,DM,OM}, {GA,DM,DL,OR,OM}, {GN,DM,OL,OR,OM} }


    };

    // will be used as: r2k_tactics[activeBuddies][teamBehaviorStatus - 1][i_pos];

    // 2019 code
    // RoboCup::TeamInfo& team = gameCtrlData.teams[gameCtrlData.teams[0].teamNumber == Global::getSettings().teamNumber ? 0 : 1];

    // 2022 code
    int own_score = theOwnTeamInfo.score;
    int opp_score = theOpponentTeamInfo.score;
    // Note: for unknown reasons, the counter is too high by 1 when the two loops below are finished
    int own_penalties = -1;
    int opp_penalties = -1;
    // loop over players and sum up penalized states
    for (const auto& buddy : theOwnTeamInfo.players) {
      if (buddy.penalty != PENALTY_NONE) own_penalties++;
    }
    for (const auto& buddy : theOpponentTeamInfo.players) {
      if (buddy.penalty != PENALTY_NONE) opp_penalties++;
    }

    TeammateRoles teamMateRoles;

// OUTPUT_TEXT("onw" << own_penalties << "opp" << opp_penalties);

    // to do: who is active - loop supp. index, number active
    // what if substitute goalie?
    int teamBehaviorStatus = TeamBehaviorStatus::R2K_NORMAL_GAME; // patch due to update errors

    // n vs. 2  || 3 vs 3 or less 
    if (opp_penalties >= 18 || (own_penalties > 18 && opp_penalties > 18)) {  //undeployed robots count as penalized; the array is 20 bots long
    // HOT FIX
    // if(true) {
      
      theTeamActivitySkill(TeamBehaviorStatus::R2K_SPARSE_GAME);
      teamBehaviorStatus = TeamBehaviorStatus::R2K_SPARSE_GAME;
  
/*    
      theTeamActivitySkill(TeamBehaviorStatus::R2K_NORMAL_GAME);
      teamBehaviorStatus = TeamBehaviorStatus::R2K_NORMAL_GAME;
  */  
    }
    else {
      if (own_score < opp_score ) {
        theTeamActivitySkill(TeamBehaviorStatus::R2K_OFFENSIVE_GAME);
        teamBehaviorStatus = TeamBehaviorStatus::R2K_OFFENSIVE_GAME;
      }

      // to do: add time limit, so we will not spoil our leadership in the last n minutes
       else if (own_score -2 > opp_score) {
        theTeamActivitySkill(TeamBehaviorStatus::R2K_DEFENSIVE_GAME);
        teamBehaviorStatus = TeamBehaviorStatus::R2K_DEFENSIVE_GAME;
      } else {
        theTeamActivitySkill(TeamBehaviorStatus::R2K_NORMAL_GAME);
        teamBehaviorStatus = TeamBehaviorStatus::R2K_NORMAL_GAME;
      }
    }


    /* information flow for role assignments:
    a) count #active players
    b) is our goali active? (ie not penalized)
    c) make a sorted, lean copy of relevant data (helper class BotOnField)
    d1) PlayerRole:: computing the supporterindex for each bot from left to right
    d2) TeammateRoles:: static assignment for STATE_INITIAL else
    d3) TeammateRoles: dynamic assignment
    d4) Use tactic table for assignments of active bots

    e) find min distance to ball for all bots
    f) who plays the ball?
    g) bot#1 is  penalized?

    h) since version 1.3: check for triggers, whether team relevant data should be updated (and send) or not

    */
    // a) count #active players
    unsigned int activeBuddies = 0;
    std::vector<BotOnField> botsLineUp;
    bool goalieIsActive = false;


// OUTPUT_TEXT("own penalties "<< own_penalties );  // 16
    if (own_penalties != lastNrOwnPenalties) {
      recomputeLineUp = true;
      // OUTPUT_TEXT("recomputeLineUp  " << lastNrOwnPenalties << " " << own_penalties);
      lastNrOwnPenalties = own_penalties;
    }

    for (int i = 0; i < 5; i++)
        if (theOwnTeamInfo.players[i].penalty == PENALTY_NONE)  activeBuddies++;

    // 

    if(recomputeLineUp){

    // OUTPUT_TEXT("aB " << activeBuddies);
    // OUTPUT_TEXT("theTeamData.numberOfActiveTeammates " << theTeamData.numberOfActiveTeammates);#

      if (1 >= activeBuddies) {   // setting  arbritary lineup
        for (unsigned int j = 0; j < 4; j++) {
          botsLineUp.push_back(BotOnField(j, (float)lineUp[j] * 100));
        }
      }   
      else {  // do a real computation    
       
        for (const auto& buddy : theTeamData.teammates) {
           
          if(!buddy.isPenalized)  { 
            // OUTPUT_TEXT("pb " << buddy.number <<  buddy.theRobotPose.translation.x());
            botsLineUp.push_back(BotOnField(buddy.number, buddy.theRobotPose.translation.x()));
          }
        
          // b) is our goalie active ? (ie not penalized)
          if (1 == buddy.number && !buddy.isPenalized)
            goalieIsActive = true;  // This flag will be used below
        }
      } // fi: line up computed on / offline (valid team data yes/no)
    
    // HOT FIX
    
    // now add myself 
    if (theRobotInfo.penalty == PENALTY_NONE){
      if (recomputeLineUp) {
        botsLineUp.push_back(BotOnField(theRobotInfo.number, theRobotPose.translation.x()));
      }
      else {
        // this is not the real x-pos; it merely is used for sorting
        botsLineUp.push_back(BotOnField(theRobotInfo.number, (float)lineUp[theRobotInfo.number-1] * 100));
        // OUTPUT_TEXT(theRobotInfo.number);
      }
    }
  }
    /*
    int  ms = botsLineUp.size(); 

    OUTPUT_TEXT(theRobotInfo.number << " " << ms << "ab " << activeBuddies);
    */    
    // observation: activeBuddies flickers from 4..5 in Simrobt
   // ASSERT(botsLineUp.size() == activeBuddies);

   // special case: I am the active goalie
    if (theRobotInfo.number == 1 && theRobotInfo.penalty == PENALTY_NONE) 
      goalieIsActive = true;
     

    // c) make a sorted, lean copy of relevant data (helper class BotOnField, see bottom of file)
    std::sort(botsLineUp.begin(), botsLineUp.end());

    if (recomputeLineUp) {
      for(unsigned int i=1;i <= botsLineUp.size();i++) // size() should equal activeBuddies; but does not always
        lineUp.at(i-1)= botsLineUp[i-1].number;
    }
  
    PlayerRole pRole;
    // deprecated
    // if (1 == theRobotInfo.number) pRole.role = PlayerRole::goalkeeper;
       
    pRole.numOfActiveSupporters = activeBuddies-1;

  
   
    // d1) PlayerRole:: computing the supporterindex for each bot from left to right
    /// tbd pRole.supporterIndex = activeBuddies;  // initally assuming we are righmost bot
    int count = -1;             // so, we start with goalie =  supporterIndex[0]
        
      // ASSERT(role.supporterIndex() - firstSupporterRole <= activeBuddies);  // we are in range supporter0 

    for (auto& mate : botsLineUp)
    {
      count++;
      if (theRobotPose.translation.x() <= mate.xPos)  // we are more left than rightmost
      {
        // pRole.role = PlayerRole::supporter4;
        // pRole.role = static_cast<PlayerRole> (static_cast<int>(PlayerRole::firstSupporterRole) + count);

        // PATCH: for communication problems
        // switch (theRobotInfo.number - 1) {
        switch (count) {
        case 0: pRole.role = PlayerRole::supporter0;   break;
        case 1: pRole.role = PlayerRole::supporter1;   break;
        case 2: pRole.role = PlayerRole::supporter2;   break;
        case 3: pRole.role = PlayerRole::supporter3;   break;
        case 4: pRole.role = PlayerRole::supporter4;   break;
        default: pRole.role = PlayerRole::none; OUTPUT_TEXT("default count: " << count);
        }
        break;  // done searching
      }
      // ASSERT(role.supporterIndex() - firstSupporterRole <= activeBuddies);  // we are in range supporter0 

    }
       
   
    // d2: static assignment , only for specific gamestates


    if (theGameInfo.state == STATE_READY || theGameInfo.state ==  STATE_SET || !theTeamCommStatus.isWifiCommActive ) {
      // default settings
      switch (theRobotInfo.number - 1) {
        case 0: pRole.role = PlayerRole::supporter0;   break;
        case 1: pRole.role = PlayerRole::supporter1;   break;
        case 2: pRole.role = PlayerRole::supporter2;   break;
        case 3: pRole.role = PlayerRole::supporter3;   break;
        case 4: pRole.role = PlayerRole::supporter4;   break;
        default: pRole.role = PlayerRole::none; OUTPUT_TEXT("default count: " << count);
      }
    }

      
    if (theGameInfo.state == STATE_READY || theGameInfo.state == STATE_SET|| !theTeamCommStatus.isWifiCommActive){ 
    //    theGameInfo.state == STATE_PLAYING) {
      // HOT FIX GORE 2023 

    // no computation of botsLineUp etc. for these game states
      int nActive = 0;
      for (auto &gcPlayer : theOwnTeamInfo.players)
      {
        if (gcPlayer.penalty == PENALTY_NONE) {
          nActive++;
        }
      }
      nActive = std::min(nActive, 5);
      
      int roleIdx = 0;
      for (size_t i = 0; i < 5; i++)
      {
        if (theOwnTeamInfo.players[i].penalty == PENALTY_NONE) {
          teamMateRoles.roles[i] = r2k_tactics[nActive - 1][teamBehaviorStatus - 1][roleIdx];
          roleIdx++;
        } else {
          teamMateRoles.roles[i] = TeammateRoles::UNDEFINED;
        }
      }
      // TEMP ANTI HOT FIX
      // teamMateRoles.captain = 3;
      theTeammateRolesSkill(teamMateRoles);
    }
    else {

      //d3: dynamic assignment
      // OUTPUT_TEXT("d3b");

      for (int i = 0; i < 5; i++) teamMateRoles.roles[i] = UN;
      // we use roles[] temporarily to store the robot numbers. 
      // In step d4, we replace these numbers by R2K_TEAM_ROLES
      // 
      // write bot numbers from left to right into roles[], according to their position on field 
      count = 0;
      for (auto& mate : botsLineUp)  // botsLineUp were sorted above; it does not contain inactive bots
      {
        teamMateRoles.roles[count++] = mate.number;
        // OUTPUT_TEXT("blp" << mate.number);
      }
      // botsLineUp.size() == number of bots not PENALIZED

      // we don't do dynamic assignment for an active goalie (ie bot #1)
      // 
      // we have robots by their number, by x-position: 
      //      eg [2,3,1,5,4]
      // we gain [1,2,3,5,4]

      bool shiftRight = false;
      if (goalieIsActive && (teamMateRoles.roles[0] != 1)) // somehow, the goalie ran into the field                      
      {
        // OUTPUT_TEXT("goalie is out of zone");
        for (int i = 5; i > 0; i--) {  // search for goalie
          if (1 == teamMateRoles.roles[i]) shiftRight = true;  // "i" is the goalies' offset 
          if (shiftRight) teamMateRoles.roles[i] = teamMateRoles.roles[i-1]; // shift right
        }
        teamMateRoles.roles[0] = 1;
      } // nothing to do wrt. goalie
      // example now is:        [1,2,3,5,UN]
    // now we have the robot numbers sorted left-to-right in roles[] -> tactical role 
    // eg [0,3,4,5,UN] -to do -> [GN,OL,DR,DL,OR,UN] 
    // 
    // r2k_tactics[5][TeamBehaviorStatus::numOfTeamActivities][5] =
    
    

      // d4
      // make a copy of teamMateRoles.roles[], so we can store tactical role in teamMateRoles.roles[]
      int sorted_bots[5];
      for (int i = 0; i <= 4; i++) {
        sorted_bots[i] = teamMateRoles.roles[i];
      }
      // now this is identical to    teamMateRoles.roles[] = mate.number or UNDEFINED
      
      for (int bot = 1; bot <= 5; bot++) {  // #bot
          
        // auto itr = std::find(sorted_bots, sorted_bots+n,i);
        int i_pos;  // offset in sorted_bots == rank left2right on real soccer field

        bool found = false;  
        for (i_pos = 0; i_pos <= 4; i_pos++) {

          // looking for rank of bot
          if (bot == sorted_bots[i_pos]) {  // bots count from 1..5
            found = true; 
            teamMateRoles.roles[bot - 1] = r2k_tactics[activeBuddies-1][teamBehaviorStatus - 1][i_pos];
            break;
          }
        }
        if (!found) teamMateRoles.roles[bot-1] = UN;
      }  // rof: bot #1..5
  
    } // else: dynamic assignment, done

     // f) goalie plays the ball?´deprecated code segment deleted

    
 
    // get min distance, set playsTheBall(); updates per frame 
    auto dist = 9000;  // max - real field dimensions should be read from config
    auto minDist = 0;
    auto buddyDist = 9000;

   
    if (theFieldBall.ballWasSeen(2000))  // to be on the safe side
      dist = (int)Geometry::distance(theFieldBall.endPositionRelative, Vector2f(0, 0));
    

    TimeToReachBall timeToReachBall;
    // this code fragment does NOT sync teamwiese
    // timeToReachBall.timeWhenReachBall = dist + theFrameInfo.time; // +offsetToFrameTimeThisBot;
    timeToReachBall.timeWhenReachBall = myEbcWrites;

    minDist = dist;
  
    // e) find min distance to ball for all _active_ bots
    //     check: if buddy is penalized it doens't cout
    for (const auto& buddy : theTeamData.teammates)
    {  // compute and compare my buddies distance with minimal distance
      if(!buddy.isPenalized)
        minDist = std::min(minDist, buddyDist = (int) Geometry::distance(theFieldBall.endPositionOnField, buddy.theRobotPose.translation));
    } // rof: scan team
   

    // f) who plays the ball? 
    // decayed update of captain (striker), 
    // we use "captain" to store the bot who plays the ball
    
    // is one of my buddies the striker?
    // OUTPUT_TEXT("time " << theFrameInfo.getTimeSince(playsTheBallHasChanged));
    
    if (theFrameInfo.getTimeSince(playsTheBallHasChangedFrame) < decayPlaysTheBall) 
      teamMateRoles.captain = lastTeammateRoles.captain;  // nothing changed
    else {
      for (const auto& buddy : theTeamData.teammates)
      {  // compute and compare my buddies distance with minimal distance
        buddyDist = (int)Geometry::distance(theFieldBall.endPositionOnField, buddy.theRobotPose.translation);
        if (buddyDist == minDist) teamMateRoles.captain = buddy.number;
      } // rof: who plays the ball

      // or am I the striker?

     
      if (minDist == dist) {  // i am the striker
        //  // TEMP ANTI HOT FIX
        teamMateRoles.captain = theRobotInfo.number;
        // teamMateRoles.captain = 3;
        /* 
        if (pRole.isGoalkeeper()) pRole.role = PlayerRole::goalkeeperAndBallPlayer;
        else pRole.role = PlayerRole::ballPlayer
        */


        // tmp. disabled for EBC testing
        // timeToReachBall.timeWhenReachBallStriker = timeToReachBall.timeWhenReachBall;  // to: get sync working
      }
    }  // fi: decay time 

    if (teamMateRoles.captain != lastTeammateRoles.captain)
    {  // another bot is playing the ball
      playsTheBallHasChangedFrame = theFrameInfo.time;  // reset timer
      // OUTPUT_TEXT(" new cap." << teamMateRoles.captain << " by " << theRobotInfo.number);
    }

    // h)  do we need to update?
    if (  // full update
      lastGameState != theGameInfo.state ||
      lastGamePhase !=  theGameInfo.gamePhase ||
      lastPlayerRole.numOfActiveSupporters != pRole.numOfActiveSupporters||
      lastTeamBehaviorStatus != teamBehaviorStatus || 
      lastTeammateRoles.roles != teamMateRoles.roles 
      // lastActiveBuddies != activeBuddies
     )
      refreshAllData = true;

    if (refreshAllData) {
      // OUTPUT_TEXT("team data are refreshed.");
      lastGameState = theGameInfo.state;
      lastGamePhase = theGameInfo.gamePhase;
      lastTeamBehaviorStatus = teamBehaviorStatus;
      lastPlayerRole = pRole;
      lastTimeToReachBall = timeToReachBall;
      if(recomputeLineUp)lastTeammateRoles = teamMateRoles;
    }

    // partial updat
    if (1 <= pRole.numOfActiveSupporters && lastTeammateRoles.captain != teamMateRoles.captain) {
      lastTeammateRoles.captain = teamMateRoles.captain;
      // lastPlayerRole = pRole;
      refreshAllData = true;
    }

    if (refreshAllData) {
      myEbcWrites = theEventBasedCommunicationData.ebcSendMessageImportant();
      // OUTPUT_TEXT("Nr: " << theRobotInfo.number << " : R2K TeamCard ebc  update");
      refreshAllData = false;
      //reomputeLineUp = true;
    }
    theRoleSkill(lastPlayerRole);
    theTimeToReachBallSkill(lastTimeToReachBall);
    if (theGameInfo.state != STATE_READY && theGameInfo.state != STATE_SET 
        && theTeamCommStatus.isWifiCommActive){
      // HOT FIX
      // && theGameInfo.state != STATE_PLAYING) { // we sended the teammateRoles already at line 347
      theTeammateRolesSkill(lastTeammateRoles);
    }

  }  // execute

  class BotOnField
  {
  public:
    int number;
    float xPos;

    BotOnField(int n, const float x)
    {
      number = n;
      xPos = x;
    };
    bool operator<(const BotOnField& other) const
    {
      return xPos < other.xPos;
    }
  };
};

MAKE_TEAM_CARD(R2K_TeamCard);
