/**
 * @file EventBasedCommunicationHandler.cpp
 * @author Connor Lismore 
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
 */

#include "EventBasedCommunicationHandler.h"
#include "Tools/MessageQueue/OutMessage.h"
#include "Tools/Global.h"
#include "Platform/Time.h"

//#define SITTING_TEST 
//#define SELF_TEST

MAKE_MODULE(EventBasedCommunicationHandler, communication);

void EventBasedCommunicationHandler::update(EventBasedCommunicationData& ebc){    
  
  if(!ebc.ebcMessageMonitor){
    ebc.ebcMessageMonitor = [&] () {ebcMessageMonitor(ebc);};
  }

  if(!ebc.ebcMessageReceiveCheck){
    ebc.ebcMessageReceiveCheck = [&] () {ebcMessageReceiveCheck();};
  }

  if(!ebc.sendThisFrame){
    ebc.sendThisFrame = [&] () {return ebcSendThisFrame(ebc);};
  }

  if(!ebc.ebcSendMessageImportant){
    ebc.ebcSendMessageImportant = [&] () {return ebcImportantMessageSend();};
  }
}

int EventBasedCommunicationHandler::ebcImportantMessageSend(){
  ebc_my_level += EBCMaxLevel;
  return ebc_my_writes;
}

void EventBasedCommunicationHandler::ebcLevelRestart(){
  ebc_my_level = EBCReset; // restart
  if(ebcDebugMessagesFull)
    OUTPUT_TEXT("EBC Level restarted back to 0.");
}

//Here start functions, which are respinsible for any ebc related tasks
void EventBasedCommunicationHandler::ebcLevelMonitor(){
  if (theFrameInfo.time == frameTimeStart) {  // called once at startup, i.e., GAMESTATE = INIT
     ebcImportantMessageSend(); // get things started
   ebc_last_activity = BehaviorStatus::initial;  // deprecated
   }
 
   // OUTPUT_TEXT(" theGameInfo.state == STATE_PLAYING)" << theGameInfo.secsRemaining);
 
  // counts up by "1" per frame
   if(theGameInfo.state == STATE_PLAYING){
     if(theGameInfo.secsRemaining > ebcBoostTime){
       ebc_my_level+=EBCCountBoost;
    }
    
    // Have message count up happen, while slowing it down based on the number of robots in the game, only send messages in specific states
    if (theFrameInfo.time % num_of_robots == theRobotInfo.number - 1 && theRobotInfo.penalty == PENALTY_NONE) {
      ebc_my_level+=EBCCountUp;
    }
  
    // Has Behavior Changed during playing? Send Message (If card change happens, send message)
    
    if(ebc_last_activity != theBehaviorStatus.activity){ 
      ebcImportantMessageSend();  // this is an important event - send asap
      ebc_last_activity = theBehaviorStatus.activity; ///
      if(ebcDebugMessages){
        OUTPUT_TEXT("robot nr:" << theRobotInfo.number << ": msg: my behavior has changed.");
      }
    }
    

    // Has the Whistle been heard? Instant Message
    if(theWhistle.lastTimeWhistleDetected == 0 && !ebc_whistle_detected){
      ebcImportantMessageSend();
      if(ebcDebugMessages){
        OUTPUT_TEXT("Robot Nr. " << theRobotInfo.number << "msg: whistle deteced");
      }
      ebc_whistle_detected = true;
    }
    else if(theWhistle.lastTimeWhistleDetected >= 3 && ebc_whistle_detected){
      ebc_whistle_detected = false;
      if(ebcDebugMessagesFull){
        OUTPUT_TEXT("Robot Nr. " << theRobotInfo.number << "WHISTLE DETECTED RESET");
      }
    }

    //Is DribblingOrSidePass active? Send Message (Also Increase Message Output?)
    /*
    if(ebc_last_activity == BehaviorStatus::offenseDribblingOrSidePass && !ebc_dribbling_active){
      ebcImportantMessageSend();
      ebc_my_level+=EBCCountBoost;
      ebc_dribbling_active = true;
      if(ebcDebugMessages){
        OUTPUT_TEXT("================");
        OUTPUT_TEXT("robot nr:" << theRobotInfo.number << ": *********Offense Dribbling Active******");
      } 
    }
    else if(ebc_last_activity != BehaviorStatus::offenseDribblingOrSidePass && ebc_dribbling_active){
      ebc_dribbling_active = false;
      if(ebcDebugMessages){
        OUTPUT_TEXT("Dribbling Complete!");
      }
    }
    */
    //Player: I Became Striker? Send Message
    
    /*
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
    
    */
  }

  // Ball is not seen: state change: send message
  // // Has robot fallen? send message
}

  
    
  

  

  

  


void EventBasedCommunicationHandler::ebcMessageMonitor(const EventBasedCommunicationData& ebc){
  if(theOwnTeamInfo.messageBudget > 0){
    ebc_my_writes++;
    /*
    if(ebcDebugMessages){
      OUTPUT_TEXT("================");
      OUTPUT_TEXT("theRobotInfo.number:" << theRobotInfo.number);
      OUTPUT_TEXT("frame last send: " << timeLastSent);
      OUTPUT_TEXT("secs remaining: " << theGameInfo.secsRemaining);  // 10m * 60sec = 1200 msg
      OUTPUT_TEXT("avail messages: " << theOwnTeamInfo.messageBudget);
      OUTPUT_TEXT("my_writes: "  << ebc_my_writes);
      OUTPUT_TEXT("Bandwidth: " << ebc_flexibleInterval);
    }
    */
  }
}

void EventBasedCommunicationHandler::ebcMessageReceiveCheck(){
    ebc_my_receives++; 
    if(ebcDebugMessages){
	    OUTPUT_TEXT("robo: " << theRobotInfo.number << " my receives: " << ebc_my_receives);
    }
}

void EventBasedCommunicationHandler::ebcMessageIntervalAdjust(const EventBasedCommunicationData& ebc){
  ebc_flexibleInterval = (theGameInfo.secsRemaining * 1000) / theOwnTeamInfo.messageBudget;

  if(theGameInfo.state == STATE_READY || theGameInfo.state == STATE_SET){
    ebc_flexibleInterval = ebcSendInterval * 2;
  }
  else if(ebc_flexibleInterval < ebcSendInterval || theGameInfo.secsRemaining < 1){
    ebc_flexibleInterval = ebcSendInterval;
  }
  else if(ebc_flexibleInterval > ebcSendIntervalMax){
    ebc_flexibleInterval = ebcSendIntervalMax;
  }
}

//Function responsible for sending messages when requirements are met. Has multiple modes
bool EventBasedCommunicationHandler::ebcSendThisFrame(const EventBasedCommunicationData& ebc){

  if (theGameInfo.state == STATE_INITIAL || theGameInfo.state == STATE_READY) return false;

  if (theGameInfo.state == STATE_FINISHED && !ebc_gameFinish) {
    ebc_gameFinish = true;
    if (ebcDebugMessages) {
      OUTPUT_TEXT("GAME FINISHED");
      OUTPUT_TEXT("EBC Messages Remaining: " << theOwnTeamInfo.messageBudget);
      OUTPUT_TEXT("Robot Nr: " << theRobotInfo.number << ": Messages Sent: " << ebc_my_writes);
      
    }
  }
  else if (theGameInfo.state != STATE_FINISHED && ebc_gameFinish) {
    ebc_gameFinish = false;
  }
  //Has the minimum available messages been met? Stop sending messages then
  if(theOwnTeamInfo.messageBudget <= ebcMinAvailableMsgs){
    return false;
  } 
  //Mode 0: Burst Mode: All robots send a message every ebcSendInterval msecs all at once: sendInterval: see .cfg
  else if(0 == ebcModeSwitch){
    if((theFrameInfo.getTimeSince(timeLastSent) >= ebcSendInterval || theFrameInfo.time < timeLastSent)) {
      if(theFrameInfo.getTimeSince(timeLastSent) >= 2 * ebcSendInterval){
        timeLastSent = theFrameInfo.time;
      }
      else{
        timeLastSent += ebcSendInterval;
      }
      if(ebcDebugMessagesFull){
        OUTPUT_TEXT("Nr: " << theRobotInfo.number 
          << ": Messages Sent: " << ebc_my_writes 
          << ": Messages Left: " << theOwnTeamInfo.messageBudget
          << ": Bandwidth: " << ebc_flexibleInterval);
      }
      return true;
    }
  } 
  //Mode 1: Round Robin: Each robot sends a message per second in a specific order, looping after all 5 send a message: ebcSendInterval
  else if(1 == ebcModeSwitch){
    if((theFrameInfo.time % ebcSendInterval) / thisRobotTurnToSend == theRobotInfo.number - 1){
      if((theFrameInfo.getTimeSince(timeLastSent) >= ebcSendInterval || theFrameInfo.time < timeLastSent)) {
        if(theFrameInfo.getTimeSince(timeLastSent) >= 2 * ebcSendInterval){
          timeLastSent = theFrameInfo.time;
        }
        else{
          timeLastSent += ebcSendInterval;
        }
        if (ebcDebugMessagesFull) {
          OUTPUT_TEXT("Nr: " << theRobotInfo.number
            << ": Messages Sent: " << ebc_my_writes
            << ": Messages Left: " << theOwnTeamInfo.messageBudget
            << ": Bandwidth: " << ebc_flexibleInterval);
        }
        return true;
      }
    }
  } 
  //Mode 2: Classic EBC: behavior changes and increase in ebc cause messages to be send: sendInterval = 525
  else if(2 == ebcModeSwitch){
    
    if((theFrameInfo.getTimeSince(timeLastSent) >= ebc_flexibleInterval || theFrameInfo.time < timeLastSent)) {
      ebcLevelMonitor();
      if(ebc_my_level >= EBCMaxLevel){
        if(theFrameInfo.getTimeSince(timeLastSent) >= 2 * ebc_flexibleInterval){
          timeLastSent = theFrameInfo.time;
        }
        else {
          timeLastSent += ebc_flexibleInterval;
        }
        // OUTPUT_TEXT("Nr: " << theRobotInfo.number << " " << ebc_my_level);
        ebcLevelRestart();
        ebcMessageIntervalAdjust(ebc);
        // OUTPUT_TEXT("Nr: " << theRobotInfo.number << ": Time Sent: " << theGameInfo.secsRemaining);
        // OUTPUT_TEXT("Nr: " << theRobotInfo.number << ": Messages Sent: " << ebc_my_writes);
        // OUTPUT_TEXT("Nr: " << theRobotInfo.number << ": Messages Left: " << theOwnTeamInfo.messageBudget);
        // OUTPUT_TEXT("Nr: " << theRobotInfo.number << ": Bandwidth: " << ebc_flexibleInterval);

        // OUTPUT_TEXT("GC Messages Remaining: " << theOwnTeamInfo.messageBudget);
        return true;
      }
    }
  }
  return false;
}