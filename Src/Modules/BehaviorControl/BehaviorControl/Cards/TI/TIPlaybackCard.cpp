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
  CALLS(Stand),
  CALLS(TIExecute),
  REQUIRES(FrameInfo),
	REQUIRES(GlobalOptions),
  REQUIRES(GameInfo),
	REQUIRES(RobotInfo),
	REQUIRES(RobotPose),
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
    }),
});


// TODO: mapping of isDone()
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
		// Exit the card if no more playback actions have to be done
		// return !preconditions();
    return -2 == actionIndex;
	}

	void execute() override
	{

		theActivitySkill(BehaviorStatus::testingBehavior);
		// OUTPUT_TEXT("ti started");

    // if (-2 == actionIndex && theFrameInfo.getTimeSince(timeLastRun) > cooldown) actionIndex = -1;
		// called only one
		if(!startTime) cardIndex = indexOfBestTeachInScore(theRobotInfo.number); // selects best sequence in playback0001.csv, plaback0002.csv,...
		ASSERT(-1 != cardIndex); // at least one model must qualify, since teachInScoreReached() is called in pre-cond

		// Figure out which action to play; sets startTime 
		currentAction =  setNextAction();

		// Playback reached the end (OR no model found, which should not happen) -> stand still
		if(actionIndex == -1)
		{
			theLookForwardSkill();
			theStandSkill();
			return;
		}

		// TODO: Better Conditions for action Execution
    if(currentAction.skill != PlaybackAction::Skills::KickAtGoal)
		  theLookForwardSkill();  // ToDo: this is just a generic action to prevent MEEKs
		theTIExecuteSkill(currentAction);
	}

  PlaybackAction setNextAction()
	{

		if(!startTime)
		// set for first action now
		{
			startTime      = state_time;
			action_changed = true;
			actionIndex    = 0;
		}
		// Replay is finished, nothing more to do.
		if(-1 == actionIndex) return {};


		
		// Switch to next action if maxTime was exceeded OR skillIsDone
		// OUTPUT_TEXT("start" << startTime << " state " << state_time);

		if(((state_time - startTime) > currentAction.maxTime) ) // || theTIExecuteSkill.isDone())
		{
			actionIndex++;
			action_changed = true; // flag for one-time setups below
			// OUTPUT_TEXT("actionIndex" << actionIndex);
		}

		// check: this next action is out of bounds -> we reached the end
		if(static_cast<size_t>(actionIndex) >= theTIPlaybackSequences.data[cardIndex].actions.size())
		{
			OUTPUT_TEXT("Reached end of playback sequence");
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
			action_changed = false;
			startTime      = state_time;
			// OUTPUT_TEXT("Action: " + std::to_string(actionIndex));
			// OUTPUT_TEXT("playback000" << cardIndex + 1 << " action Index and Name: " << actionIndex << " " << TypeRegistry::getEnumName(theTIPlaybackSequences.data[cardIndex].actions[actionIndex].skill));
			// OUTPUT_TEXT("remaining time" << diff);


			// obsolete code
			// prepare check for isDone(): set exit criterion
			// destPos = theRobotPose * theTIPlaybackSequences.data[cardIndex].actions[actionIndex].poseParam;
		}
    return currentAction;
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
    float minimal_distance = 500.0f;
    int world_model_index = -1;  // start counting at 0
    int current_bestWorldModelIndex = -1;

    // OUTPUT_TEXT("Computing indexOfBestTeachInScore");

    for (WorldData data : theTIPlaybackSequences.models)
    {
      WorldModel& model = data.trigger;
      world_model_index++;
      // OUTPUT_TEXT(model.fileName);
      if ( //model.robotNumber == Number && // when this is disabled, any robot close to the point of recording will trigger
        thisIsATriggerPoint(model)) // this is the first OR a better trigger point
      {
        current_bestWorldModelIndex = world_model_index;
        minimal_distance = Geometry::distance(theRobotPose.translation, model.robotPose.translation);  // new minimum
      }
    }  // rof: scan all world models

    ASSERT(current_bestWorldModelIndex >= 0);  // there must be at least one trigger point, because teachInScoreReached() was true in the pre-condition
    OUTPUT_TEXT("trigger became active for robot " << Number << " from file " << theTIPlaybackSequences.models[current_bestWorldModelIndex].fileName);

    return current_bestWorldModelIndex;
  };

};


MAKE_CARD(TIPlaybackCard);