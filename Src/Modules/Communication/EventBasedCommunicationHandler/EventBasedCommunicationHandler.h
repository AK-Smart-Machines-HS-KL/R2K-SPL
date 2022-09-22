/**
 * @file EventBasedCommunicationHandler.h
 * @author Connor Lismore (2022.02.13)
 * @brief Module Header for the Event Based Communication. Responsible for ensuring messages are send by checking for events and other changes. Reduces the amount of messages put out.
 * @version 0.3
 * @date 
 * 
 * r2kTest: Regression Testing Code um scenen zu laden und werte zu speicher für das vergleichen von Änderungen (Codebase Change, EBC Tweaking, etc)
 * https://zwoogle3.informatik.hs-kl.de/rzwei_kickers/r2k_robocup/-/wikis/R2K-Regression-Testing
 * 
 * Changelog: Added Adjusting sendinterval for message delay bandwidth. Now adjusts based on time remaining vs message remaining
 * added whistleHeard and dribbleGame priority message sending
 * 
 * 
 * ToDo:
 * - ebcModeSwitch
 * - num_of_robots
 * - check and tune ebc_flexibleInterval
 * - ebcMaxAvailableMessage per messages? distribute even?
 */


#pragma once

#include "Representations/Communication/EventBasedCommunicationData.h"

#include "Representations/BehaviorControl/BehaviorStatus.h"
#include "Representations/BehaviorControl/TeamBehaviorStatus.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Infrastructure/RobotHealth.h"
#include "Representations/Infrastructure/TeamTalk.h"
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


MODULE(EventBasedCommunicationHandler,
{,
  // v- using for calculations
  REQUIRES(FrameInfo),
  REQUIRES(OwnTeamInfo),
  // Uses following for messages
  USES(BallModel),
  USES(BehaviorStatus),
	USES(GameInfo),
  USES(RobotInfo),
  USES(Whistle),
  USES(TeamBehaviorStatus),
  USES(TeammateRoles),

  //provides to representation eventBasedData
  PROVIDES(EventBasedCommunicationData),


  LOADS_PARAMETERS(
  {,
    (int) ebcSendInterval,               /* 1000 When ebc is enabled, replaces the sendInterval from teamMessageHandler */
    (int) ebcSendIntervalMax,            /* 2400 When ebc is enabled, upper limit for bandwith (see ebc_flexibleInterval) */
    (int) ebcModeSwitch,                 /* 2 = Mode: 20% Message Reduction (Burst Mode), 1 = Round Robin mode, 2 = ebc classic is on */
    (bool) ebcDebugMessages,             /* true Wether to enable messaging such as messageReceived, ballNotSeen, etc. */
    (bool)ebcDebugMessagesFull,          /* false Wether to enable messaging per bot */
    (int) num_of_robots,                 /* 5 needed to check for the number of robots, need to check wether more robots on the field get more messages per game */
    (int) ebcMaxAvailableMessage,         /* 1200 chose, according to rules*/
    (int) ebcMinAvailableMsgs,           /* 20 If minimum is reached, currently stops messaging, later to reduce messages being send */
    (int) ebcBoostTime,                  /* 585 Time for boost, when timer reaches number, boost will stop */
    (bool) ebcIsStriker,                 // did striker change?
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
  unsigned int ebc_my_level = 0;                                              //Current ebc level, counts up during game through time and events, when 100, sends a message
  unsigned int ebc_my_writes = 0;                                             //Amount of messages a nao has send
  unsigned int ebc_my_receives = 0;                                           //Amount of messages a nao has received
  BehaviorStatus::Activity ebc_last_activity;                                 //Used to check if nao current behavior has changed
 
  const unsigned int thisRobotTurnToSend = ebcSendInterval / num_of_robots;  //Utilized for round robin, checks which robots turn it is to send

  const unsigned int EBCMaxLevel = 100;                                       //Max ebc level to be reached before a message can be send
  const int EBCCountUp = EBCMaxLevel / 40;                                    //incremental increase for ebcount until a message will be sent
  const int EBCCountBoost = EBCCountUp / 2;                                   //Boost for start and VERY important states
  const int EBCReset = 0;                                                     //constant used to reset ebc back to 0 if a message was send
  unsigned int frameTimeStart = 0;                                            //Used for the beginning of a game so that each nao sends a message
  // bool ebc_dribbling_active = false;                                          //Check that dribbling is active, if so, send message now
  bool ebc_whistle_detected = false;

  int ebc_flexibleInterval = ebcSendInterval;                                 // decrease bandwith by increasing send intervall by ebc_flexibleInterval 
  bool ebc_gameFinish = false;
  
  void update(EventBasedCommunicationData& ebc);                              //update method
  int  ebcImportantMessageSend();                                             //Function to raise ebcLevel to 100%, therefore sending message instantly
  // return value: #messages written
  void ebcLevelMonitor();                                                     //Mode 2 only: function for keeping track of the ebc_level and changing it when needed
  void ebcLevelRestart();                                                     //function for setting ebc_level back to 0
  void ebcMessageMonitor(const EventBasedCommunicationData& ebc);             //function for keeping track of messages send and how many are left
  void ebcMessageReceiveCheck();                                              //function for counting up how many messages each robot has received so far
  void ebcMessageIntervalAdjust(const EventBasedCommunicationData& ebc);      //
  
  bool ebcSendThisFrame(const EventBasedCommunicationData& ebc);              //bool utilized to check wether frame should be send over at the teammessagehandler
};