/**
 * @file EventBasedCommunicationHandler.h
 * @author Connor Lismore (2022.02.13)
 * @brief Module Hemaxader for the Event Based Communication. Responsible for ensuring messages are send by checking for events and other changes. Reduces the amount of messages put out.
 * @version 0.3
 * @date 
 * 
 * r2kTest: Regression Testing Code um scenen zu laden und werte zu speicher für das vergleichen von Änderungen (Codebase Change, EBC Tweaking, etc)
 * https://zwoogle3.informatik.hs-kl.de/rzwei_kickers/r2k_robocup/-/wikis/R2K-Regression-Testing
 * 
 * Changelog: Added Adjusting sendinterval for message delay bandwidth. Now adjusts based on time remaining vs message remaining
 * added whistleHeard and dribbleGame priority message sending
 * 
 * * 4/23, Adrian
 * a) added getOwnTeamInfoMessageBudget() to patch a defect in SimRobot (which returns messageBudget = 0 all the time) 
*     We now have
 *    - a rough estimation of the budget in SimRobot
 *    - true messageBudget from OwnTeamInfo during game
 *    The function replaces several, redundant code fragments in EventBasedCommunicationHandler.cpp
 *    Reduced minMessageBudget (was: ebcMinAvailableMsg )  from 20 to 6, 
 *      since we now are safe in ebcSendThisFrame():  if(getOwnTeamInfoMessageBudget() <= minMessageBudget) return false;
 * 
 * b) improved ebcMessageIntervalAdjust(); dynamic burn-down rate now is:  (time left / budget left) * defaultFlexibleInterval  (which is ~4000msec)
 *    the result of the computation is stored in variable flexibleInterval
 * 
 * c) refactored variable names a lot; re-organized .cfg
 * 
 * d) added theGameInfo.state == STATE_FINISHED when to block sending messages 
 */


#pragma once

#include "Representations/Communication/EventBasedCommunicationData.h"

#include "Representations/BehaviorControl/BehaviorStatus.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Infrastructure/RobotHealth.h"
#include "Representations/Modeling/FieldCoverage.h"
#include "Representations/Modeling/ObstacleModel.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Modeling/Whistle.h"
#include "Representations/Perception/FieldFeatures/FieldFeatureOverview.h"
#include "Representations/Perception/ImagePreprocessing/CameraMatrix.h"
#include "Representations/Sensing/FallDownState.h"
#include "Tools/Module/Module.h"
#include "Representations/Communication/BHumanMessage.h"
#include "Representations/Communication/TeamData.h"
#include "Tools/Communication/BNTP.h"
#include "Tools/Communication/RoboCupGameControlData.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Communication/TeamCommStatus.h"

MODULE(EventBasedCommunicationHandler,
{,
  // v- using for calculations
  REQUIRES(FrameInfo),
  REQUIRES(OwnTeamInfo),
  REQUIRES(TeamCommStatus),  // wifi on off?
  // Uses following for messages
  USES(GameInfo),
  USES(BallModel),
  USES(BehaviorStatus),
  USES(RobotInfo),
  USES(Whistle),
  USES(TeamBehaviorStatus),
  USES(TeammateRoles),

  //provides to representation eventBasedData
  PROVIDES(EventBasedCommunicationData),


  LOADS_PARAMETERS(
  {,
    (int) sendInterval,               /* 2000msec When ebc is enabled, replaces the sendInterval from teamMessageHandler */
    (int) defaultFlexibleInterval,    // default: 4000, ie., 4000 msec between two messages per bot
    (int) messageBudget,              /* 1200 chose, according to rules*/
    (int) minMessageBudget,           /* 6 If minimum is reached, currently stops messaging, later to reduce messages being send */
    (int) ebcModeSwitch,              /* 2 = Mode: 20% Message Reduction (Burst Mode), 1 = Round Robin mode, 2 = ebc classic is on */
    (bool)ebcDebugMessages,           /* true Wether to enable messaging such as messageReceived, ballNotSeen, etc. */
    (bool)ebcDebugMessagesFull,       /* false Wether to enable messaging per bot */
    (int) activeRobots,              /* 5 needed to check for the number of robots, need to check wether more robots on the field get more messages per game */
    
    (int) ebcBoostTime,               /* 585 Time for boost, when timer reaches number, boost will stop */
    (bool)ebcIsStriker,               // did striker change?
  }),
});

//#define SELF_TEST_EventBasedCommunicationHandler

/**
 * @class 
 * 
 */
class EventBasedCommunicationHandler : public EventBasedCommunicationHandlerBase
{
public:
  EventBasedCommunicationHandler() : EventBasedCommunicationHandlerBase() {}

private:
  // v- output stuff
  mutable unsigned timeLastSent = 0;                                          //used with theFrameInfo.timeLastSent
  
  // event based communications: the monitors 
  unsigned int myUrgencyLevel = 0;                                              //Current ebc level, counts up during game through time and events, when 100, sends a message
  unsigned int sendCount = 0;                                             //Amount of messages a nao has send
  unsigned int receiveCount = 0;                                           //Amount of messages a nao has received
  BehaviorStatus::Activity lastBehavior;                                 //Used to check if nao current behavior has changed
 
  const unsigned int thisRobotTurnToSend = sendInterval / activeRobots;  //Utilized for round robin, checks which robots turn it is to send

  const unsigned int EBCMaxLevel = 100;                                       //Max ebc level to be reached before a message can be send
  const int EBCCountUp = EBCMaxLevel / 40;                                    //incremental increase for ebcount until a message will be sent
  const int EBCCountBoost = EBCCountUp / 2;                                   //the result is: 1 Boost for start and VERY important states
  const int EBCReset = 0;                                                     //constant used to reset ebc back to 0 if a message was send
  unsigned int frameTimeStart = 0;                                            //Used for the beginning of a game so that each nao sends a message
  bool ebc_dribbling_active = false;                                          //Check that dribbling is active, if so, send message now
  bool whistleDetected = false;

  int flexibleInterval = sendInterval;                                 // decrease bandwith by increasing send intervall by ebc_flexibleInterval 
  bool gameIsFinished = false;
  
  void update(EventBasedCommunicationData& ebc);                              //update method
  int  ebcImportantMessageSend();                                             //Function to raise ebcLevel to 100%, therefore sending message instantly
  // return value: #messages written
  void ebcLevelMonitor();                                                     //Mode 2 only: function for keeping track of the ebc_level and changing it when needed
  void ebcLevelRestart();                                                     //function for setting ebc_level back to 0
  void ebcMessageMonitor(const EventBasedCommunicationData& ebc);             //function for keeping track of messages send and how many are left
  void ebcMessageIntervalAdjust(const EventBasedCommunicationData& ebc);      //
  
  bool ebcSendThisFrame(const EventBasedCommunicationData& ebc);              //bool utilized to check wether frame should be send over at the teammessagehandler                           // this function patches the fact theOwnTeamInfo.messageBudget == 0 in SimRobot
  int getOwnTeamInfoMessageBudget(); 
  unsigned int getTotalSecsRemaining();                                       // counting down 1200 .. 0 
};

unsigned int EventBasedCommunicationHandler::getTotalSecsRemaining() {
  if (theGameInfo.firstHalf == 1) return 600 + theGameInfo.secsRemaining;
  else return theGameInfo.secsRemaining;
  
};