/**
 * @file TIPlaybackCard.cpp
 * @author Jonas Lambing
 * @brief This card executes skills from a playback sequence
 * 
 * This card executes skills with parameters specified by a playback sequence.
 * The execution happens on a per-action basis.
 * That means, that the next action will trigger as soon as the current action
 * is either finished (isDone()) or exceeded the maximum time specified in the playback file.
 * 
 * MaxTime is calculated during the teach in process.
 * 
 * This card is a collection for all available teach in cards.
 *
 * @version 1.0
 * @date 2021-08-26
 *
 * Changes for 
 * @version 2.0
 * @date 2022-01-24
 * 
 * @author Adrian Müller
 * Major revision: Works now together with the enhanced TIPlaybackProvider and TIExecute:
 * a) pre-cond checks wether at least on card playback000x.csv is qualified (the sequence in it triggers)
 * b) indexOfBestTeachInScore() loops over all cards, ie. all available playback sequences (ie., playback0001 ... playback000MAX), MAX >=1.
 *    Selects sequence with best trigger point, registered in parameter card_index 
 * c) set_next_action() loops over actions 0 .. size-1, stores it in actionIndex
 *    So we have: currentAction = theTIPlayback.data[cardIndex].actions[actionIndex];
 * d) generic call for each action with theTIExecuteSkill(currentAction);
 *	  Next action: either time limit is exceeded OR skill.isDone()
 * e) post-condition: -1 == actionIndex : set_next_action() reached end of actions
 *
 * Notes: 
 *    Register all skills in playback000x.scv in Execute.cpp  MAP_EXPLICIT, MAP, MAP_ISDONE, and add CALLS() here
 *    b) is a control mechanism for game tactics
 *    d) time limit might come in to early
 *    d) skill.isDone() does not work properly on all skills, eg theWalkToTargetSkill() is buggy with this respect
 * 
 * 
 * @version 2.1
 * @date 2022-01-24
 * @author Adrian Müller
 * - change semantics of trigger points: any robot close to the point of recording will trigger
 * 
 * @version 2.2
 * @date 2024-04-22
 * @author Adrian Müller
 * - fixed multi triggering by checking cooldown time 
 * - ending sequence: setting acition_index-2 == wait for cooldwon period, default -1 (== first call ever) means: start immediately
 * ToDo:
 *	include generic map for head movements
 */

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/TI/TIPlaybackData.h"
#include "Representations/Configuration/GlobalOptions.h" 
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Modeling/RobotPose.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Tools/Math/Geometry.h"

CARD(TIPlaybackCard,
{,
  CALLS(Activity),
  CALLS(LookForward),
  CALLS(LookActive),
  CALLS(Stand),
  CALLS(TIExecute),
  REQUIRES(FrameInfo),
	REQUIRES(GlobalOptions),
  REQUIRES(GameInfo),
	REQUIRES(RobotInfo),
	REQUIRES(RobotPose),
  REQUIRES(FieldBall),
  REQUIRES(TIPlaybackSequences),

    DEFINES_PARAMETERS(
    {,
					 (bool)(false) onceP,
           (int)(0)startTime,              // Time when the action was started
					 (int)(-1)cardIndex,             // Index of the card inside the stack of worldmodels (and playbacks resp.)
					 (int)(-1)actionIndex,            // Index of the action inside the card, ie., which playback action is active
					 (PlaybackAction)currentAction,  // A copy of the action-data
					 (bool)(false)action_changed,    // used as flag: action is called the first time in execute(); reset after First usage, set again in setNextAction()
																					 // so we can set information like the robots starting position, only once, before action starts
           (unsigned int)(0) timeLastRun,
           (unsigned int)(15000) cooldown,  // waiting time, until next playback will be executed
           (unsigned int)(500) min_distance, // radial distance bot from trigger point
           (int)(0) dynamicMaxTime,          // computed max time for current action (overrides CSV value for movement skills)
           (int)(-1) triggerSetPlay,         // setPlay value captured when sequence started — used to detect game-state change
           (int)(-1) triggerGameState,       // gameState value captured when sequence started — used to detect game-state change
    }),
});


// TODO: mapping of head movements
class TIPlaybackCard : public TIPlaybackCardBase
{
	bool preconditions() const override
	{


    // OUTPUT_TEXT(theFrameInfo.getTimeSince(timeLastRun));

		// Dont execute if the card stack is empty
		return ((-1 == actionIndex || (-2 == actionIndex && theFrameInfo.getTimeSince(timeLastRun) > cooldown )) && !theTIPlaybackSequences.models.empty() && teachInScoreReached(theRobotInfo.number));
			// return (teachInScoreReached(theRobotInfo.number));
	}

	bool postconditions() const override
	{
		if(-2 != actionIndex) return false; // keep running while sequence is executing
		// Sequence done: exit to allow card stack to continue searching for applicable cards
		return true;
	}

	void execute() override
	{

		theActivitySkill(BehaviorStatus::testingBehavior);
		// OUTPUT_TEXT("ti started");

		// After cooldown: reset from holding-alive state so the sequence can restart (only if trigger still valid)
		if(actionIndex == -2 && theFrameInfo.getTimeSince(timeLastRun) > cooldown
		   && teachInScoreReached(theRobotInfo.number))
			actionIndex = -1;

		// Select best sequence only on fresh start, not when holding alive after completion
		if(actionIndex == -1) cardIndex = indexOfBestTeachInScore(theRobotInfo.number);
		ASSERT(-1 != cardIndex); // at least one model must qualify, since teachInScoreReached() is called in pre-cond

		// Figure out which action to play; sets startTime 
		currentAction =  setNextAction();

		// Playback reached the end (OR no model found, which should not happen) -> stand still
		if(actionIndex < 0)
		{
			theLookForwardSkill();  // Ensure head motion is set when sequence completes
			theStandSkill();        // Ensure motion request is set
      actionIndex=-2;
			return;
		}

		// Execute the current action with active head tracking.
		// GoToBallAndDribble (Dribble) and GoToBallAndKick (KickAtGoal) manage head control
		// internally via GoToBallHeadControl → LookActive. Calling LookActive here too would
		// trigger a "headMotionRequest set more than once" Meeek error for those skills.
		const bool skillHandlesHead = (currentAction.skill == PlaybackAction::Skills::Dribble ||
		                               currentAction.skill == PlaybackAction::Skills::KickAtGoal);
		if(!skillHandlesHead)
			theLookActiveSkill(/* withBall: */ true);
		theTIExecuteSkill(currentAction);
	}

  PlaybackAction setNextAction()
	{
		// Guard against invalid cardIndex
		if (cardIndex < 0 || cardIndex >= static_cast<int>(theTIPlaybackSequences.data.size()))
		{
      OUTPUT_ERROR("TI: setNextAction called with invalid cardIndex: " << cardIndex << " (data size: " << static_cast<int>(theTIPlaybackSequences.data.size()) << ")");
			actionIndex = -1;
			return {};
		}

		// Sequence completed and holding alive: don't restart
		if(actionIndex == -2) return {};

		if(!startTime)
		// set for first action now
		{
			startTime      = state_time;
			action_changed = true;
			actionIndex    = 0;
		}
		// Replay is finished, nothing more to do.
		if(actionIndex < 0) return {};


		
		// Switch to next action if dynamicMaxTime was exceeded OR robot reached target.
		// dynamicMaxTime is computed per-action from distance; movementTargetReached() checks proximity.
		const bool timeExceeded = (state_time - startTime) > dynamicMaxTime;
		const bool skillDone    = movementTargetReached(currentAction);
		if(timeExceeded || skillDone)
		{
			actionIndex++;
			action_changed = true; // flag for one-time setups below
			// OUTPUT_TEXT("actionIndex" << actionIndex);
		}

		// check: this next action is out of bounds -> we reached the end
		if(static_cast<size_t>(actionIndex) >= theTIPlaybackSequences.data[cardIndex].actions.size())
		{
			OUTPUT_TEXT("TI: Playback done — robot " << theRobotInfo.number << ", sequence: " << theTIPlaybackSequences.data[cardIndex].fileName);
			currentAction = {};
			actionIndex   = -2;  // set post condition
            timeLastRun = theFrameInfo.time;
            startTime = 0;
			return currentAction;
		}

		// ok: we are inbetween o .. #actions-1
		currentAction = theTIPlaybackSequences.data[cardIndex].actions[actionIndex];
		if(action_changed)  // do setups for the new action
		{
			action_changed  = false;
			startTime       = state_time;
			dynamicMaxTime  = computeDynamicMaxTime(currentAction);
		}
    return currentAction;
	}

  /**
   * @brief Compute a per-action time budget from current distance to target.
   *
   * For movement-to-target skills (WalkToPoint, WalkToBall) the CSV maxTime is
   * discarded in favour of: estimated_travel_time + buffer.
   * All other skills keep their CSV maxTime unchanged.
   *
   * walkSpeed: conservative SPL walk speed (mm/s) used for the travel estimate.
   * buffer:    extra time (ms) added on top of the travel estimate.
   * minimum:   floor (ms) so the skill always gets at least one motion cycle.
   */
  int computeDynamicMaxTime(const PlaybackAction& pa) const
  {
    constexpr float walkSpeed = 160.f;  // mm/s – conservative; accounts for turning, acceleration, obstacle avoidance
    constexpr int   buffer    = 5000;   // ms
    constexpr int   minimum   = 1000;   // ms

    float dist = 0.f;
    switch(pa.skill)
    {
      case PlaybackAction::Skills::WalkToPoint:
        dist = (theRobotPose.translation - pa.poseParam.translation).norm();
        break;
      case PlaybackAction::Skills::WalkToBall:
        // Ball is a moving target: use current relative distance as an educated guess
        dist = theFieldBall.endPositionRelative.norm();
        break;
      default:
        return pa.maxTime;  // non-movement skill: keep CSV value
    }
    return std::max(minimum, static_cast<int>(dist / walkSpeed * 1000.f) + buffer);
  }

  /**
   * @brief True when the robot has reached the action's target within 200 mm.
   *
   * Only meaningful for movement-to-target skills.
   * Returns false for all other skills so their maxTime drives the transition.
   */
  bool movementTargetReached(const PlaybackAction& pa) const
  {
    constexpr float threshold = 200.f;  // mm
    switch(pa.skill)
    {
      case PlaybackAction::Skills::WalkToPoint:
        return (theRobotPose.translation - pa.poseParam.translation).norm() <= threshold;
      case PlaybackAction::Skills::WalkToBall:
        return theFieldBall.endPositionRelative.norm() <= threshold;
      default:
        return false;
    }
  }

  bool thisIsATriggerPoint(const WorldModel& model) const

  {
    ASSERT(!theTIPlaybackSequences.models.empty()); // has been checked in the pre-condition
    
    return  (model.setPlay == theGameInfo.setPlay &&
       std::abs(Geometry::distance(theRobotPose.translation, model.robotPose.translation)) <= min_distance);


    // OUTPUT_TEXT("Trigger Point " << world_model_index << " for robot" << theRobotInfo.number);


  }

  // param number unused yet
  bool teachInScoreReached(int number) const
  {
    ASSERT(!theTIPlaybackSequences.models.empty());  // has been checked in the pre-condition
    // OUTPUT_TEXT("checking trigger for robot " << Number  );
    for (WorldData data : theTIPlaybackSequences.models)
    {
      WorldModel& model = data.trigger;
     // if (//model.robotNumber == number && // when this is disabled, any robot close to the point of recording will trigger
      if (thisIsATriggerPoint(model))  // this is a trigger point for given world model
        return true;
    }
    return false;
  }

  int indexOfBestTeachInScore(int Number)
  {
    // Guard against empty models
    if (theTIPlaybackSequences.models.empty())
    {
      OUTPUT_ERROR("TI: indexOfBestTeachInScore called but no models available");
      return -1;
    }

    float minimal_distance = 500.0f;
    int world_model_index = -1;  // start counting at 0
    int current_bestWorldModelIndex = -1;

    // OUTPUT_TEXT("Computing indexOfBestTeachInScore");

    for (WorldData data : theTIPlaybackSequences.models)
    {
      WorldModel& model = data.trigger;
      world_model_index++;
      // Verify world_model_index is within bounds
      if (world_model_index >= static_cast<int>(theTIPlaybackSequences.models.size()))
      {
        OUTPUT_ERROR("TI: world_model_index out of bounds in indexOfBestTeachInScore");
        break;
      }
      // OUTPUT_TEXT(model.fileName);
      if ( //model.robotNumber == Number && // when this is disabled, any robot close to the point of recording will trigger
        thisIsATriggerPoint(model)) // this is the first OR a better trigger point
      {
        current_bestWorldModelIndex = world_model_index;
        minimal_distance = Geometry::distance(theRobotPose.translation, model.robotPose.translation);  // new minimum
      }
    }  // rof: scan all world models

    // Verify bounds of returned index before accessing
    if (current_bestWorldModelIndex >= 0 && current_bestWorldModelIndex < static_cast<int>(theTIPlaybackSequences.models.size()))
    {
      OUTPUT_TEXT("trigger became active for robot " << Number << " from file " << theTIPlaybackSequences.models[current_bestWorldModelIndex].fileName);
    }
    else
    {
      ASSERT(current_bestWorldModelIndex >= 0);  // there must be at least one trigger point, because teachInScoreReached() was true in the pre-condition
      return -1;
    }

    // Translate models[] index → data[] index by name-matching.
    // models[] and data[] are loaded independently and may differ in order
    // (e.g. due to orphan removal in enforceConsistency). Using a positional
    // index from models[] directly into data[] causes the wrong playback to run.
    std::string playbackName = theTIPlaybackSequences.models[current_bestWorldModelIndex].fileName;
    size_t rpos = playbackName.rfind("worldmodel");
    if (rpos != std::string::npos)
      playbackName.replace(rpos, 10, "playback");
    for (int i = 0; i < static_cast<int>(theTIPlaybackSequences.data.size()); i++)
    {
      if (theTIPlaybackSequences.data[i].fileName == playbackName)
        return i;
    }
    OUTPUT_ERROR("TI: No matching playback for triggered worldmodel: " << theTIPlaybackSequences.models[current_bestWorldModelIndex].fileName);
    return -1;
  };

};


MAKE_CARD(TIPlaybackCard);