/**
 * @file R2K_TeamCard.cpp
 * @author Adrian Müller
 * @version: 1.8
 * @date: 9/2022, updated 3/2026
 *
 * v1.0  Initial: TeamBehaviorStatus, TeammateRoles defaults, playsTheBall/striker, isGoalkeeper
 * v1.1  R2K_SPARSE_MODE logic, supporterIndex, dynamic substitute goalie
 * v1.2  Sophisticated TeammateRoles (penalized bots), static STATE_INITIAL assignment
 * v1.3  Separate update policies: PlayerRole/TimeToReachBall vs. TeammateRoles
 * v1.4  captain field now stores striker robot number (not goalkeeper)
 * v1.5  Code cleanup, fixed defaultPoseProvider.cfg sign error, line-up coupled to penalty changes
 * v1.6  HOT FIX GORE 23: disabled dynamic role/tactic computation
 * v1.7  Reverted hot fixes; TeamCommStatus offline triggers static role assignment
 * v1.8  Code cleanup: named constants, merged duplicate branches, fixed OOB in goalie shift,
 *        fixed recomputeLineUp never resetting, removed dead code and debug artefacts,
 *        corrected stale/misleading comments
 *
 * Responsibilities:
 * - Assign tactical roles to robots based on field position (closer to own goal → defensive)
 * - Compute overall team strategy (R2K_SPARSE_GAME, R2K_NORMAL_GAME, etc.)
 * - Robot #1 always maps to GOALIE_BOT_NUMBER; goalie skills are exclusive to that number
 *
 * Notes:
 * - timeToReachBall is computed per-bot and NOT synced team-wide. The striker is determined by
 *   looping all buddies and finding the one with minimum distance to the team ball position.
 * - Accuracy of playsTheBall/captain depends on EBC update frequency and self-localisation quality.
 * - captain (TeammateRoles) stores the robot number of the current striker (not goalkeeper).
 *
 * ToDo:
 * - Read real field dimensions from config (currently using MAX_FIELD_DISTANCE_MM sentinel)
 * - Add time-limit logic: switch to DEFENSIVE when leading late in the game
 * - Take secsTillUnpenalized into account for SPARSE mode (adapt strategy ahead of return)
 * - Static assignment for STATE_FINISHED
 * - EBC: broadcast explicitly when striker changes
 */

#include "Representations/BehaviorControl/TeamSkills.h"
#include "Tools/BehaviorControl/Framework/Card/TeamCard.h"

#include <algorithm>
#include <vector>

#include "Tools/Math/Geometry.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/TimeToReachBall.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"
#include "Representations/Communication/TeamData.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/EventBasedCommunicationData.h"
#include "Representations/BehaviorControl/PlayerRole.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/BehaviorControl/TeammateRoles.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Communication/TeamCommStatus.h"

// Tactic table shorthand aliases
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
    CALLS(Role),
    CALLS(TeammateRoles),
    CALLS(TimeToReachBall),
    REQUIRES(FieldBall),
    REQUIRES(FrameInfo),
    REQUIRES(TeamData),
    REQUIRES(GameInfo),
    REQUIRES(RobotInfo),
    REQUIRES(RobotPose),
    REQUIRES(TeamCommStatus),
    CALLS(TeamActivity),
    REQUIRES(OwnTeamInfo),
    REQUIRES(OpponentTeamInfo),
    REQUIRES(EventBasedCommunicationData),

    DEFINES_PARAMETERS(
                {
                  ,
                  (bool)    (true)                     refreshAllData,          // true: triggers computation on first frame
                  (unsigned) (STATE_INITIAL)           lastGameState,
                  (unsigned) (SET_PLAY_NONE)           lastGamePhase,
                  (int)(-1)                            lastTeamBehaviorStatus,  // -1: not set yet
                  (int)(1000)                          decayPlaysTheBall,       // ms; B-Human default ballWasSeen is 500
                  (int)(10000)                         decayUpdateSupporterIndex,
                  (unsigned)(0)                        playsTheBallHasChangedFrame,
                  (unsigned)(0)                        lastUpdateSupporterIndexFrame,
                  (TeammateRoles)(TeammateRoles())     lastTeammateRoles,
                  (TimeToReachBall)(TimeToReachBall()) lastTimeToReachBall,
                  (PlayerRole)(PlayerRole())           lastPlayerRole,
                  (int)(-1)                            lastNrOwnPenalties,      // -1: not set yet
    }),
});

class R2K_TeamCard : public R2K_TeamCardBase
{
  bool preconditions() const override { return true; }
  bool postconditions() const override { return false; }

  // -------------------------------------------------------------------------
  // Named constants — replace all raw literals
  // -------------------------------------------------------------------------

  // Standard field team size for SPL 5v5.
  // Change here when moving to 7v7.
  static constexpr int FIELD_TEAM_SIZE = 5;

  // Robot number reserved for the goalie. Goalie-specific skills only execute on this number.
  static constexpr int GOALIE_BOT_NUMBER = 1;

  // Robots numbered >= TEACH_IN_MARKER_THRESHOLD are teach-in markers, not field players.
  // They appear in theTeamData but must be excluded from role/lineup computation.
  static constexpr int TEACH_IN_MARKER_THRESHOLD = 6;

  // SPARSE mode is triggered when the opponent has very few active players.
  // theOwnTeamInfo/theOpponentTeamInfo.players[] is 20 slots long (SPL protocol MAX_NUM_PLAYERS).
  // Undeployed slots may register as penalized depending on the game controller version.
  // The penalty loop starts at -1 to compensate for one such phantom count.
  // The threshold 18 means: the raw loop count reached 19 (out of 20), i.e. at most 1 real
  // opponent is active => numerical advantage large enough for sparse-game tactics.
  static constexpr int SPARSE_OPP_PENALTY_THRESHOLD = 18;

  // Sentinel distance [mm] used when ball has not been seen recently.
  // Should eventually be replaced by a value read from the field config.
  static constexpr int MAX_FIELD_DISTANCE_MM = 9000;

  // lineUp stores the last-known sorted robot numbers (left-to-right on field).
  // Size is TEACH_IN_MARKER_THRESHOLD so that index (robotNumber - 1) is always valid
  // for any numbered robot up to and including the teach-in markers.
  bool recomputeLineUp = false;
  std::vector<int> lineUp = {1, 2, 3, 4, 5, 6};

private:

  void execute() override
  {
    // Tactic table: [activePlayers-1][teamBehaviorStatus-1][positionRank left-to-right]
    // positionRank 0 = leftmost bot on field, FIELD_TEAM_SIZE-1 = rightmost
    const int r2k_tactics[FIELD_TEAM_SIZE][TeamBehaviorStatus::numOfTeamActivities][FIELD_TEAM_SIZE] =
    {
      //         R2K_NORMAL_GAME        R2K_DEFENSIVE_GAME     R2K_OFFENSIVE_GAME     R2K_SPARSE_GAME
      // 1 player
      { {GN,UN,UN,UN,UN},  {GN,UN,UN,UN,UN},  {GN,UN,UN,UN,UN},  {OM,UN,UN,UN,UN} },
      // 2 players
      { {GN,OM,UN,UN,UN},  {GN,OM,UN,UN,UN},  {GN,OM,UN,UN,UN},  {DM,OM,UN,UN,UN} },
      // 3 players
      { {GN,DM,OM,UN,UN},  {GN,DR,DM,UN,UN},  {GA,DM,OM,UN,UN},  {GN,OR,OM,UN,UN} },
      // 4 players
      { {GN,DM,OR,OM,UN},  {GN,DR,DL,OM,UN},  {GA,DM,DL,OM,UN},  {GN,DM,OL,OM,UN} },
      // 5 players
      { {GN,DR,DL,OR,OL},  {GN,DR,DL,DM,OM},  {GA,DM,DL,OR,OM},  {GN,DM,OL,OR,OM} }
    };
    // Index: r2k_tactics[activeBuddies - 1][teamBehaviorStatus - 1][positionRank]

    // -----------------------------------------------------------------------
    // Score and penalty snapshot
    // -----------------------------------------------------------------------
    int own_score = theOwnTeamInfo.score;
    int opp_score = theOpponentTeamInfo.score;

    // The players[] array has 20 slots (SPL MAX_NUM_PLAYERS); undeployed/bench slots may
    // register as penalized. We initialise at -1 to cancel exactly one such phantom count.
    // This gives the number of truly penalised field players on each side.
    int own_penalties = -1;
    int opp_penalties = -1;
    for (const auto& buddy : theOwnTeamInfo.players)
      if (buddy.penalty != PENALTY_NONE) own_penalties++;
    for (const auto& buddy : theOpponentTeamInfo.players)
      if (buddy.penalty != PENALTY_NONE) opp_penalties++;

    // -----------------------------------------------------------------------
    // Team strategy / TeamBehaviorStatus
    // -----------------------------------------------------------------------
    TeammateRoles teamMateRoles;
    int teamBehaviorStatus = TeamBehaviorStatus::R2K_NORMAL_GAME;

    // SPARSE: opponent has almost no active players (numerical advantage)
    if (opp_penalties >= SPARSE_OPP_PENALTY_THRESHOLD ||
        (own_penalties > SPARSE_OPP_PENALTY_THRESHOLD && opp_penalties > SPARSE_OPP_PENALTY_THRESHOLD))
    {
      theTeamActivitySkill(TeamBehaviorStatus::R2K_SPARSE_GAME);
      teamBehaviorStatus = TeamBehaviorStatus::R2K_SPARSE_GAME;
    }
    else
    {
      if (own_score == opp_score)
      {
        theTeamActivitySkill(TeamBehaviorStatus::R2K_NORMAL_GAME);
        teamBehaviorStatus = TeamBehaviorStatus::R2K_NORMAL_GAME;
      }
      // ToDo: add time limit near end of game to avoid throwing away a lead
      else if (own_score < opp_score)
      {
        theTeamActivitySkill(TeamBehaviorStatus::R2K_OFFENSIVE_GAME);
        teamBehaviorStatus = TeamBehaviorStatus::R2K_OFFENSIVE_GAME;
      }
      else
      {
        theTeamActivitySkill(TeamBehaviorStatus::R2K_DEFENSIVE_GAME);
        teamBehaviorStatus = TeamBehaviorStatus::R2K_DEFENSIVE_GAME;
      }
    }

    /* Information flow for role assignments:
       a) count active players
       b) is our goalie active?
       c) build sorted botsLineUp (fresh from team data, or from cache)
       d1) compute supporterIndex (PlayerRole) left-to-right
       d2) static assignment for STATE_READY / STATE_SET / wifi-down
       d3) dynamic assignment (STATE_PLAYING, wifi up): sort bots by field position
       d4) map sorted positions to tactical roles via r2k_tactics table
       e) find minimum distance to ball across all active bots
       f) determine striker (captain = robot number with minimum ball distance)
       g) bot #1 returning from penalty: goalie treatment (implicit via GOALIE_BOT_NUMBER constant;
          the goalie-specific skills enforce this via theRobotInfo.number == GOALIE_BOT_NUMBER)
       h) decide whether to broadcast updated data via EBC
    */

    // -----------------------------------------------------------------------
    // a) Count active players
    // -----------------------------------------------------------------------
    unsigned int activeBuddies = 0;

    // Only count slots up to TEACH_IN_MARKER_THRESHOLD+1; bots #TEACH_IN_MARKER_THRESHOLD and
    // beyond are teach-in markers, not real field players.
    for (int i = 0; i < TEACH_IN_MARKER_THRESHOLD + 1; i++)
      if (theOwnTeamInfo.players[i].penalty == PENALTY_NONE) activeBuddies++;

    // Clamp to standard game team size
    if (activeBuddies > static_cast<unsigned int>(FIELD_TEAM_SIZE))
      activeBuddies = FIELD_TEAM_SIZE;

    // -----------------------------------------------------------------------
    // b) Is our goalie active? (uses fresh GameController data, always reliable)
    // -----------------------------------------------------------------------
    bool goalieIsActive = (theOwnTeamInfo.players[GOALIE_BOT_NUMBER - 1].penalty == PENALTY_NONE);

    // -----------------------------------------------------------------------
    // Decide if lineup needs fresh computation
    // -----------------------------------------------------------------------
    if (own_penalties != lastNrOwnPenalties)
    {
      recomputeLineUp = true;
      lastNrOwnPenalties = own_penalties;
    }
    if (theFrameInfo.getTimeSince(lastUpdateSupporterIndexFrame) > decayUpdateSupporterIndex)
    {
      lastUpdateSupporterIndexFrame = theFrameInfo.time;
      recomputeLineUp = true;
    }

    // -----------------------------------------------------------------------
    // c) Build botsLineUp: sorted list of active robots by x-position
    // -----------------------------------------------------------------------
    std::vector<BotOnField> botsLineUp;

    if (recomputeLineUp)
    {
      if (activeBuddies <= 1)
      {
        // Fallback when we have no useful teammate data: build a placeholder
        // lineup from the last known order so supporter-index computation
        // still produces a defined result.
        // FIELD_TEAM_SIZE - 1 slots for teammates; self is added below.
        for (int j = 0; j < FIELD_TEAM_SIZE - 1; j++)
          botsLineUp.push_back(BotOnField(lineUp[j], (float)(j + 1) * 100));
      }
      else
      {
        for (const auto& buddy : theTeamData.teammates)
        {
          // Exclude teach-in marker bots
          if (buddy.number < TEACH_IN_MARKER_THRESHOLD && !buddy.isPenalized)
            botsLineUp.push_back(BotOnField(buddy.number, buddy.theRobotPose.translation.x()));
        }
      }

      // Always add self when not penalised (using real current position)
      if (theRobotInfo.penalty == PENALTY_NONE)
        botsLineUp.push_back(BotOnField(theRobotInfo.number, theRobotPose.translation.x()));
    }
    else
    {
      // Lineup order is unchanged; reconstruct from cached lineUp array.
      // activeBuddies is always freshly computed from the game controller, so we know
      // exactly how many cached entries are valid.
      unsigned int cachedSize = std::min(static_cast<unsigned int>(lineUp.size()), activeBuddies);
      for (unsigned int j = 0; j < cachedSize; j++)
        botsLineUp.push_back(BotOnField(lineUp[j], (float)(j + 1) * 100));
      // Note: cached xPos is ordinal-rank * 100 mm (not real field position).
      // This is intentional: relative order is preserved; exact positions are stale.
    }

    // c) Sort by x-position (ascending = left-to-right on field)
    std::sort(botsLineUp.begin(), botsLineUp.end());

    // Cache the sorted order and release the recompute flag
    if (recomputeLineUp)
    {
      for (unsigned int i = 0; i < botsLineUp.size(); i++)
        lineUp.at(i) = botsLineUp[i].number;
      recomputeLineUp = false;
    }

    // -----------------------------------------------------------------------
    // d1) PlayerRole: supporterIndex = rank in sorted lineup (0 = leftmost)
    // -----------------------------------------------------------------------
    PlayerRole pRole;
    pRole.numOfActiveSupporters = activeBuddies - 1;

    int count = -1;  // starts at -1 so first bot gets count==0
    for (const auto& mate : botsLineUp)
    {
      count++;
      if (theRobotPose.translation.x() <= mate.xPos)
      {
        switch (count)
        {
          case 0: pRole.role = PlayerRole::supporter0; break;
          case 1: pRole.role = PlayerRole::supporter1; break;
          case 2: pRole.role = PlayerRole::supporter2; break;
          case 3: pRole.role = PlayerRole::supporter3; break;
          case 4: pRole.role = PlayerRole::supporter4; break;
          default: pRole.role = PlayerRole::none;
            OUTPUT_TEXT("R2K_TeamCard: supporterIndex out of range: " << count);
        }
        break;
      }
    }

    // -----------------------------------------------------------------------
    // d2 / d3 / d4) Role assignment — static or dynamic
    //
    // Static: used when the game is not live or wifi is unavailable.
    //   - Assigns roles by robot number (1→pos0, 2→pos1, … ) ignoring field positions.
    //   - Safe fallback: no team communication required.
    // Dynamic: used during STATE_PLAYING with wifi up.
    //   - Builds a left-to-right sorted bot list and maps positions to tactical roles.
    // -----------------------------------------------------------------------

    // Condition is factored out to avoid repeating the same three-way test twice.
    const bool useStaticRoleAssignment =
        theGameInfo.state == STATE_READY ||
        theGameInfo.state == STATE_SET   ||
        !theTeamCommStatus.isWifiCommActive;  // wifi down: best-effort static fallback

    if (useStaticRoleAssignment)
    {
      // d2) supporter index: assign by robot number (no position data needed)
      switch (theRobotInfo.number - 1)
      {
        case 0: pRole.role = PlayerRole::supporter0; break;
        case 1: pRole.role = PlayerRole::supporter1; break;
        case 2: pRole.role = PlayerRole::supporter2; break;
        case 3: pRole.role = PlayerRole::supporter3; break;
        case 4: pRole.role = PlayerRole::supporter4; break;
        default: pRole.role = PlayerRole::none;
      }

      // Static tactical roles: iterate the first FIELD_TEAM_SIZE GC player slots and
      // assign roles in order of unpenalised appearance (left fills first tactic slot, etc.)
      int nActive = 0;
      for (int i = 0; i < FIELD_TEAM_SIZE; i++)
        if (theOwnTeamInfo.players[i].penalty == PENALTY_NONE) nActive++;
      nActive = std::min(nActive, FIELD_TEAM_SIZE);

      int roleIdx = 0;
      for (int i = 0; i < FIELD_TEAM_SIZE; i++)
      {
        if (theOwnTeamInfo.players[i].penalty == PENALTY_NONE)
          teamMateRoles.roles[i] = r2k_tactics[nActive - 1][teamBehaviorStatus - 1][roleIdx++];
        else
          teamMateRoles.roles[i] = UN;
      }
      theTeammateRolesSkill(teamMateRoles);
    }
    else
    {
      // d3) Dynamic assignment: temporarily store sorted bot numbers in roles[]
      for (int i = 0; i < FIELD_TEAM_SIZE; i++) teamMateRoles.roles[i] = UN;

      count = 0;
      for (const auto& mate : botsLineUp)
        teamMateRoles.roles[count++] = mate.number;

      // Enforce goalie at position 0: if the goalie ran into the field,
      // shift the intervening bots one slot to the right and place goalie at [0].
      if (goalieIsActive && teamMateRoles.roles[0] != GOALIE_BOT_NUMBER)
      {
        bool shiftRight = false;
        // Fixed: loop starts at FIELD_TEAM_SIZE - 1 (was 5, which was out-of-bounds for a
        // size-5 array).
        for (int i = FIELD_TEAM_SIZE - 1; i > 0; i--)
        {
          if (GOALIE_BOT_NUMBER == teamMateRoles.roles[i]) shiftRight = true;
          if (shiftRight) teamMateRoles.roles[i] = teamMateRoles.roles[i - 1];
        }
        teamMateRoles.roles[0] = GOALIE_BOT_NUMBER;
      }

      // d4) Replace the temporarily stored bot numbers with tactical roles.
      // sorted_bots[i] = robot number at sorted position i (left-to-right)
      int sorted_bots[FIELD_TEAM_SIZE];
      for (int i = 0; i < FIELD_TEAM_SIZE; i++)
        sorted_bots[i] = teamMateRoles.roles[i];

      // For each robot slot, find its position rank and look up the tactical role.
      for (int bot = 1; bot <= FIELD_TEAM_SIZE; bot++)
      {
        bool found = false;
        for (int i_pos = 0; i_pos < FIELD_TEAM_SIZE; i_pos++)
        {
          if (bot == sorted_bots[i_pos])
          {
            found = true;
            teamMateRoles.roles[bot - 1] =
                r2k_tactics[activeBuddies - 1][teamBehaviorStatus - 1][i_pos];
            break;
          }
        }
        if (!found) teamMateRoles.roles[bot - 1] = UN;
      }
    }

    // -----------------------------------------------------------------------
    // e) Compute this bot's distance to the team ball position
    // -----------------------------------------------------------------------
    int dist = MAX_FIELD_DISTANCE_MM;
    if (theFieldBall.ballWasSeen(decayPlaysTheBall))
      dist = static_cast<int>(
          Geometry::distance(theFieldBall.teamPositionOnField, theRobotPose.translation));

    TimeToReachBall timeToReachBall;
    timeToReachBall.timeWhenReachBall = dist;

    // e) Find the minimum distance to ball across all active teammates
    int minDist = dist;
    int buddyDist;
    for (const auto& buddy : theTeamData.teammates)
    {
      if (!buddy.isPenalized)
      {
        buddyDist = static_cast<int>(
            Geometry::distance(theFieldBall.teamPositionOnField, buddy.theRobotPose.translation));
        if (buddyDist < minDist) minDist = buddyDist;
      }
    }

    // -----------------------------------------------------------------------
    // f) Determine striker (captain = robot number with minimum ball distance)
    //    Gated by decayPlaysTheBall to avoid rapid captain changes.
    // -----------------------------------------------------------------------
    if (theFrameInfo.getTimeSince(playsTheBallHasChangedFrame) < decayPlaysTheBall)
    {
      // Within decay window: keep last known striker
      teamMateRoles.captain = lastTeammateRoles.captain;
    }
    else
    {
      // Check each teammate
      for (const auto& buddy : theTeamData.teammates)
      {
        buddyDist = static_cast<int>(
            Geometry::distance(theFieldBall.teamPositionOnField, buddy.theRobotPose.translation));
        if (buddyDist == minDist)
        {
          teamMateRoles.captain = buddy.number;
          timeToReachBall.timeWhenReachBallStriker = buddyDist;
        }
      }
      // Check if I am the striker (my dist equals the minimum)
      if (minDist == dist)
      {
        teamMateRoles.captain = theRobotInfo.number;
        timeToReachBall.timeWhenReachBallStriker = dist;
      }
    }

    if (teamMateRoles.captain != lastTeammateRoles.captain)
    {
      playsTheBallHasChangedFrame = theFrameInfo.time;
      teamMateRoles.timestamp = theFrameInfo.time;
    }

    // -----------------------------------------------------------------------
    // h) Decide whether a full EBC broadcast is needed
    // -----------------------------------------------------------------------
    if (lastGameState != theGameInfo.state                              ||
        lastGamePhase != theGameInfo.gamePhase                          ||
        lastPlayerRole.numOfActiveSupporters != pRole.numOfActiveSupporters ||
        lastTeamBehaviorStatus != teamBehaviorStatus                    ||
        lastTeammateRoles.roles != teamMateRoles.roles)
      refreshAllData = true;

    if (refreshAllData)
    {
      lastGameState = theGameInfo.state;
      lastGamePhase = theGameInfo.gamePhase;
      lastTeamBehaviorStatus = teamBehaviorStatus;
      lastPlayerRole = pRole;
      lastTimeToReachBall = timeToReachBall;
      lastTeammateRoles = teamMateRoles;
    }

    // Partial update: striker changed even if full refresh was not triggered
    if (pRole.numOfActiveSupporters >= 1 && lastTeammateRoles.captain != teamMateRoles.captain)
    {
      lastTeammateRoles.captain = teamMateRoles.captain;
      refreshAllData = true;
    }

    if (refreshAllData)
    {
      theEventBasedCommunicationData.ebcSendMessageImportant();
      refreshAllData = false;
    }

    // -----------------------------------------------------------------------
    // Emit skills
    // -----------------------------------------------------------------------
    theRoleSkill(lastPlayerRole);
    theTimeToReachBallSkill(lastTimeToReachBall);

    // TeammateRoles for PLAYING+wifi: the static path already called theTeammateRolesSkill
    // above (using the fresh local teamMateRoles). For the dynamic path we emit lastTeammateRoles.
    if (!useStaticRoleAssignment)
      theTeammateRolesSkill(lastTeammateRoles);

  }  // execute()

  // -------------------------------------------------------------------------
  // Helper: lightweight record for sorting active robots by field x-position
  // -------------------------------------------------------------------------
  class BotOnField
  {
  public:
    int number;
    float xPos;

    BotOnField(int n, float x) : number(n), xPos(x) {}

    bool operator<(const BotOnField& other) const { return xPos < other.xPos; }
  };
};

MAKE_TEAM_CARD(R2K_TeamCard);
