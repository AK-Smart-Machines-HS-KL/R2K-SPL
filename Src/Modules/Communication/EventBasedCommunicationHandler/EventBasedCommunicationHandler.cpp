/**
 * @file EventBasedCommunicationHandler.cpp
 * @author Connor Lismore, Adrian Mueller
 * @brief Module for the Event Based Communication. Responsible for ensuring messages are send by checking for events and other changes. Reduces the amount of messages put out.
 * @version 0.3
 * @date 2022-03-17
 * 
 * r2kTest: Regression Testing Code um scenen zu laden und werte zu speicher für das vergleichen von Änderungen (Codebase Change, EBC Tweaking, etc)
 * https://zwoogle3.informatik.hs-kl.de/rzwei_kickers/r2k_robocup/-/wikis/R2K-Regression-Testing
 * 
 * Changelog: Added Adjusting sendinterval for message delay bandwidth. Now adjusts based on time remaining vs message remaining
 * added whistleHeard and dribbleGame priority message sending
 *  
 * 
 */

#include "EventBasedCommunicationHandler.h"
#include "Tools/MessageQueue/OutMessage.h"
#include "Tools/Global.h"
#include "Platform/Time.h"
#include "Representations/Communication/TeamData.h"

//#define SITTING_TEST 
//#define SELF_TEST

MAKE_MODULE(EventBasedCommunicationHandler, communication);

void EventBasedCommunicationHandler::update(EventBasedCommunicationData& ebc){    

 /*
  unsigned int activeBuddies = 1; // myself

  for (const auto& buddy : theTeamData.teammates)
  {
    if (!buddy.isPenalized && buddy.isUpright) activeBuddies++;
  };
  activeRobots = activeBuddies;

  */

  if(!ebc.ebcMessageMonitor){
    ebc.ebcMessageMonitor = [&] () {ebcMessageMonitor(ebc);};
  }

  if(!ebc.sendThisFrame){
    ebc.sendThisFrame = [&] () {return ebcSendThisFrame(ebc);};
  }

  if(!ebc.ebcSendMessageImportant){
    ebc.ebcSendMessageImportant = [&] () {return ebcImportantMessageSend();};
  }
}

int EventBasedCommunicationHandler::getOwnTeamInfoMessageBudget() {
   
   // to be replaced by theOwnTeamInfo.messageBudget
  #ifdef TARGET_ROBOT
  return theOwnTeamInfo.messageBudget;
  #else
  return messageBudget - sendCount * activeRobots; // this is a ROUGH estimation
  #endif
}

void EventBasedCommunicationHandler::ebcMessageMonitor(const EventBasedCommunicationData& ebc){
  if (getOwnTeamInfoMessageBudget() >= minMessageBudget) {
    sendCount++;
    /*
    if(ebcDebugMessages){
      OUTPUT_TEXT("================");
      OUTPUT_TEXT("theRobotInfo.number:" << theRobotInfo.number);
      OUTPUT_TEXT("frame last send: " << timeLastSent);
      OUTPUT_TEXT("secs remaining: " << theGameInfo.secsRemaining);  // 10m * 60sec = 1200 msg
      OUTPUT_TEXT("avail messages: " << getOwnTeamInfoMessageBudget());
      OUTPUT_TEXT("my_writes: "  << ebc_my_writes);
      OUTPUT_TEXT("Bandwidth: " << ebc_flexibleInterval);
    }
    */
  }
}

int EventBasedCommunicationHandler::ebcImportantMessageSend(){
  myUrgencyLevel += EBCMaxLevel;  // raise urgency above threshold
  return sendCount;
}

void EventBasedCommunicationHandler::ebcLevelRestart(){
  myUrgencyLevel = EBCReset; // restart, reset urgency is minimal (0)
  if(ebcDebugMessagesFull)
    OUTPUT_TEXT("EBC Level restarted back to 0.");
}

//Here start functions, which are responsible for any ebc related tasks
void EventBasedCommunicationHandler::ebcLevelMonitor(){
  if (theFrameInfo.time == frameTimeStart) {  // called once at startup, i.e., GAMESTATE = INIT
     ebcImportantMessageSend(); // get things started
   lastBehavior = BehaviorStatus::initial;  // deprecated
   }
  
  // counts up by "1" per frame
   if(theGameInfo.state == STATE_PLAYING){
     if(theGameInfo.secsRemaining > ebcBoostTime){
       myUrgencyLevel+=EBCCountBoost;
    }
    
    // Have message count up happen, while slowing it down based on the number of robots in the game, only send messages in specific states
    if (theFrameInfo.time % activeRobots == theRobotInfo.number - 1 && theRobotInfo.penalty == PENALTY_NONE) {
      myUrgencyLevel+=EBCCountUp;
    }
  
    // Has Behavior Changed during playing? Send Message (If card change happens, send message)
    
    if(lastBehavior != theBehaviorStatus.activity){ 
      ebcImportantMessageSend();  // this is an important event - send asap
      lastBehavior = theBehaviorStatus.activity; ///
      if(ebcDebugMessages){
        OUTPUT_TEXT("robot nr:" << theRobotInfo.number << ": msg: my behavior has changed.");
        if (theRobotInfo.number == 4) {
          OUTPUT_TEXT("flexibleInterval " << flexibleInterval << " ebc_message budget : " << getOwnTeamInfoMessageBudget() <<
            " active bots: " << activeRobots);
        };

      }
    }
    

    // Has the Whistle been heard? Instant Message
    if(theWhistle.lastTimeWhistleDetected == 0 && !whistleDetected){
      ebcImportantMessageSend();
      if(ebcDebugMessages){
        OUTPUT_TEXT("Robot Nr. " << theRobotInfo.number << "msg: whistle deteced");
      }
      whistleDetected = true;
    }
    else if(theWhistle.lastTimeWhistleDetected >= 3 && whistleDetected){
      whistleDetected = false;
      if(ebcDebugMessagesFull){
        OUTPUT_TEXT("Robot Nr. " << theRobotInfo.number << "WHISTLE DETECTED RESET");
      }
    }

    //Is DribblingOrSidePass active? Send Message (Also Increase Message Output?)
    
    if(lastBehavior == BehaviorStatus::offenseForwardPassCard && !ebc_dribbling_active){
      ebcImportantMessageSend();
      myUrgencyLevel+=EBCCountBoost;
      ebc_dribbling_active = true;
      if(ebcDebugMessages){
        OUTPUT_TEXT("================");
        OUTPUT_TEXT("robot nr:" << theRobotInfo.number << ": *********Offense Dribbling Active******");
      } 
    }
    else if(lastBehavior!= BehaviorStatus::offenseForwardPassCard && ebc_dribbling_active){
      ebc_dribbling_active = false;
      if(ebcDebugMessages){
        OUTPUT_TEXT("Dribbling Complete!");
      }
    }
    
    //Player: I Became Striker? Send Message
        
    if(theTeammateRoles.playsTheBall(theRobotInfo.number) && !ebcIsStriker){
      ebcImportantMessageSend();
      ebcIsStriker = true;
      if(ebcDebugMessages){
        OUTPUT_TEXT("Robot Nr. " << theRobotInfo.number << ": msg: I became striker.");
      }
    }
    else if( !theTeammateRoles.playsTheBall(theRobotInfo.number) && ebcIsStriker){
      ebcImportantMessageSend();
      ebcIsStriker = false;
      if (ebcDebugMessages) {
        OUTPUT_TEXT("Robot Nr. " << theRobotInfo.number << ": msg: I lost striker.");
      }
    };
  }

  // Ball is not seen: state change: send message
  // // Has robot fallen? send message
}

void EventBasedCommunicationHandler::ebcMessageIntervalAdjust(const EventBasedCommunicationData& ebc){
  // AM
  // ebc_flexibleInterval = (theGameInfo.secsRemaining * 1000) / ebcMaxAvailableMessage - ebc_my_receives;

  // sample: 200 sec left, 40 messages left in budget -> send 1 msg each 5 frame * defaultFlexibleInterval
  // the lower the constant - eg 2000 - the lower the #msg send  --> flexible intervall is ~ 2sec
  flexibleInterval = defaultFlexibleInterval * getTotalSecsRemaining() / getOwnTeamInfoMessageBudget();

  if(theGameInfo.state == STATE_READY || theGameInfo.state == STATE_SET){
    flexibleInterval = sendInterval * 2;
  }

  /*
  else if(ebc_flexibleInterval < ebcSendInterval || theGameInfo.secsRemaining < 1){
    ebc_flexibleInterval = ebcSendInterval;
  }
  else if(ebc_flexibleInterval > ebcSendIntervalMax){
    ebc_flexibleInterval = ebcSendIntervalMax;
  }
  */
}

//Function responsible for sending messages when requirements are met. Has multiple modes
// Note: if we do not communicate, #robots = 1 is assumed -> all bots become the goalie in STATE_PLAYING ;-)

bool EventBasedCommunicationHandler::ebcSendThisFrame(const EventBasedCommunicationData& ebc){

    if (theGameInfo.state == STATE_FINISHED && !gameIsFinished) {
    gameIsFinished = true;
    if (ebcDebugMessages) {
      OUTPUT_TEXT("GAME FINISHED");
      OUTPUT_TEXT("EBC Messages Remaining: " << getOwnTeamInfoMessageBudget());
      OUTPUT_TEXT("Robot Nr: " << theRobotInfo.number << ": Messages Sent: " << sendCount);
      
    }
  }
  else if (theGameInfo.state != STATE_FINISHED && gameIsFinished) {
    gameIsFinished = false;
  }


  if (theGameInfo.state == STATE_INITIAL || theGameInfo.state == STATE_READY || theGameInfo.state == STATE_FINISHED) 
    return false;
    // Note: R2K_TeamCard deals with STATE_READY explicitely, to save bandwith here

  if(getOwnTeamInfoMessageBudget() <= minMessageBudget){
    return false;
  } 
  //Mode 0: Burst Mode: All robots send a message every ebcSendInterval msecs all at once: sendInterval: see .cfg
  else if(0 == ebcModeSwitch){
    if((theFrameInfo.getTimeSince(timeLastSent) >= sendInterval || theFrameInfo.time < timeLastSent)) {
      if(theFrameInfo.getTimeSince(timeLastSent) >= 2 * sendInterval){
        timeLastSent = theFrameInfo.time;
      }
      else{
        timeLastSent += sendInterval;
      }
      if(ebcDebugMessagesFull){
        OUTPUT_TEXT("Nr: " << theRobotInfo.number 
          << ": Messages Sent: " << sendCount 
          << ": Messages Left: " << getOwnTeamInfoMessageBudget()
          << ": Bandwidth: " << flexibleInterval);
      }
      return true;
    }
  } 
  //Mode 1: Round Robin: Each robot sends a message per second in a specific order, looping after all 5 send a message: ebcSendInterval
  else if(1 == ebcModeSwitch){
    if((theFrameInfo.time % sendInterval) / thisRobotTurnToSend == theRobotInfo.number - 1){
      if((theFrameInfo.getTimeSince(timeLastSent) >= sendInterval || theFrameInfo.time < timeLastSent)) {
        if(theFrameInfo.getTimeSince(timeLastSent) >= 2 * sendInterval){
          timeLastSent = theFrameInfo.time;
        }
        else{
          timeLastSent += sendInterval;
        }
        if (ebcDebugMessagesFull) {
          OUTPUT_TEXT("Nr: " << theRobotInfo.number
            << ": Messages Sent: " << sendCount
            << ": Messages Left: " << getOwnTeamInfoMessageBudget()
            << ": Bandwidth: " << flexibleInterval);
        }
        return true;
      }
    }
  } 
  //Mode 2: Classic EBC: behavior changes and increase in ebc cause messages to be send: sendInterval = 525
  else if(2 == ebcModeSwitch){
 
    if((theFrameInfo.getTimeSince(timeLastSent) >= flexibleInterval || theFrameInfo.time < timeLastSent)) {
      ebcLevelMonitor();
      if(myUrgencyLevel >= EBCMaxLevel){
        if(theFrameInfo.getTimeSince(timeLastSent) >= 2 * flexibleInterval){
          timeLastSent = theFrameInfo.time;
        }
        else {
          timeLastSent += flexibleInterval;
        }
        // OUTPUT_TEXT("Nr: " << theRobotInfo.number << " " << ebc_my_level);
        ebcLevelRestart();
        ebcMessageIntervalAdjust(ebc);
        // OUTPUT_TEXT("Nr: " << theRobotInfo.number << ": Time Sent: " << theGameInfo.secsRemaining);
        // OUTPUT_TEXT("Nr: " << theRobotInfo.number << ": Messages Sent: " << ebc_my_writes);
        // OUTPUT_TEXT("Nr: " << theRobotInfo.number << ": Messages Left: " << getOwnTeamInfoMessageBudget());
        // OUTPUT_TEXT("Nr: " << theRobotInfo.number << ": Bandwidth: " << ebc_flexibleInterval);

        // OUTPUT_TEXT("GC Messages Remaining: " << getOwnTeamInfoMessageBudget());
        return true;
      }
    }
  }
  return false;
}