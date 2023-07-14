/**
 * @file TeamMessageHandler.cpp
 *
 * @author <a href="mailto:jesse@tzi.de">Jesse Richter-Klug</a>
 */

#include "TeamMessageHandler.h"
#include "Tools/MessageQueue/OutMessage.h"
#include "Tools/Global.h"
#include "Tools/Settings.h"
#include "Platform/File.h"
#include "Platform/Time.h"

//#define SITTING_TEST
//#define SELF_TEST

MAKE_MODULE(TeamMessageHandler, communication);
struct TeamMessage
{};

TeamMessageHandler::TeamMessageHandler() :
  theBNTP(theFrameInfo, theRobotInfo)
{}

void TeamMessageHandler::update(BHumanMessageOutputGenerator& outputGenerator)
{
  if (ebcEnable) {
    bool ebcSendThisFrame = theEventBasedCommunicationData.sendThisFrame();
    if(ebcSendThisFrame) theEventBasedCommunicationData.ebcMessageMonitor();
    
    outputGenerator.sendThisFrame =
#ifndef SITTING_TEST
#ifdef TARGET_ROBOT
      theMotionRequest.motion != MotionRequest::playDead &&
      theMotionInfo.executedPhase != MotionPhase::playDead &&
#endif
#endif // !SITTING_TEST

      ebcSendThisFrame;
  }
  else {

    outputGenerator.sendThisFrame =
#ifndef SITTING_TEST
#ifdef TARGET_ROBOT
      theMotionRequest.motion != MotionRequest::playDead &&
      theMotionInfo.executedPhase != MotionPhase::playDead &&
#endif
#endif // !SITTING_TEST
      (theFrameInfo.getTimeSince(timeLastSent) >= sendInterval || theFrameInfo.time < timeLastSent);
  }

  theRobotStatus.hasGroundContact = theGroundContactState.contact && theMotionInfo.executedPhase != MotionPhase::getUp && theMotionRequest.motion != MotionRequest::getUp;
  theRobotStatus.isUpright = theFallDownState.state == FallDownState::upright || theFallDownState.state == FallDownState::staggering || theFallDownState.state == FallDownState::squatting;
  if(theRobotStatus.hasGroundContact)
    theRobotStatus.timeOfLastGroundContact = theFrameInfo.time;
  if(theRobotStatus.isUpright)
    theRobotStatus.timeWhenLastUpright = theFrameInfo.time;
}

void TeamMessageHandler::update(TeamData& teamData)
{
  theBNTP.update();  

  maintainBMateList(teamData);
}

void TeamMessageHandler::maintainBMateList(TeamData& teamData) const
{
  //@author <a href="mailto:tlaue@uni-bremen.de">Tim Laue</a>
  {
    // Iterate over deprecated list of teammate information and update some convenience information
    // (new information has already been coming via handleMessages)
    for(auto& teammate : teamData.teammates)
    {
      Teammate::Status newStatus = Teammate::PLAYING;
      if(theOwnTeamInfo.players[teammate.number - 1].penalty != PENALTY_NONE)
        newStatus = Teammate::PENALIZED;
      else if(!teammate.isUpright)
        newStatus = Teammate::FALLEN;

      if(newStatus != teammate.status)
      {
        teammate.status = newStatus;
        teammate.isPenalized = teammate.status == Teammate::PENALIZED;
        teammate.timeWhenStatusChanged = theFrameInfo.time;
      }

      teammate.isGoalkeeper = teammate.number == 1;
    }

    // Remove elements that are too old:
    auto teammate = teamData.teammates.begin();
    while(teammate != teamData.teammates.end())
    {
      if(theFrameInfo.getTimeSince(teammate->timeWhenLastPacketReceived) > networkTimeout)
        teammate = teamData.teammates.erase(teammate);
      else
        ++teammate;
    }

    // Other stuff
    teamData.numberOfActiveTeammates = 0;
    teammate = teamData.teammates.begin();
    while(teammate != teamData.teammates.end())
    {
      if(teammate->status != Teammate::PENALIZED)
        teamData.numberOfActiveTeammates++;
      ++teammate;
    }
  }
}
