/**
 * @file Tools/Communication/BNTP.cpp
 *
 * Representations and functions for time synchronization inside
 * the team. Implementation of parts of the Network Time Protocol.
 *
 * @author <a href="mailto:tlaue@uni-bremen.de">Tim Laue</a>
 * @author <A href="mailto:jesse@tzi.de">Jesse Richter-Klug</A>
 */

#include "BNTP.h"
#include "Tools/Debugging/Debugging.h"
#include "Platform/Time.h"

BNTP::BNTP(const FrameInfo& theFrameInfo, const RobotInfo& theRobotInfo) : theFrameInfo(theFrameInfo), theRobotInfo(theRobotInfo) {}

void BNTP::rcvRequest(BNTPRequestComponent * comp, RobotMessageHeader & header) {
  const unsigned receiveTimestamp = Time::getCurrentSystemTime();

  BNTPRequest& ntpRequest = receivedNTPRequests[header.senderID];
  if(ntpRequest.origination < header.timestamp)
  {
    ntpRequest.origination = header.timestamp;
    ntpRequest.receipt = receiveTimestamp;
  }
  

  if(header.senderID < Settings::lowestValidPlayerNumber ||
    header.senderID > Settings::highestValidPlayerNumber)
    return;
    
  BNTPResponseComponent::priority = std::numeric_limits<int>::max();
}

void BNTP::rcvResponse(BNTPResponseComponent * comp, RobotMessageHeader & header) {

  const unsigned receiveTimestamp = Time::getCurrentSystemTime();

  // Invalidate the previous synchronization if it doesn't explain the received message.
  auto& remoteSMB = timeSyncBuffers[header.senderID - Settings::lowestValidPlayerNumber];
  remoteSMB.validate(header.timestamp, receiveTimestamp);

  // Integrate the teammate's response (if the message contains one)
  for(const BNTPMessage& ntpMessage : comp->messages)
  {
    // Only use responses for this player.
    if(ntpMessage.receiver != theRobotInfo.number)
      continue;
    // Ignore measurements that are normally impossible.
    // This catches cases in which a robot gets a response for a request but has been restarted in the meantime.
    if(receiveTimestamp < ntpMessage.requestOrigination || header.timestamp < ntpMessage.requestReceipt)
      continue;
    const unsigned totalRoundTrip = receiveTimestamp - ntpMessage.requestOrigination;
    const unsigned remoteProcessingTime = header.timestamp - ntpMessage.requestReceipt;
    if(remoteProcessingTime > totalRoundTrip)
      continue;
    const int offset = static_cast<int>(ntpMessage.requestReceipt - ntpMessage.requestOrigination + header.timestamp - receiveTimestamp) / 2;
    remoteSMB.update(offset, totalRoundTrip - remoteProcessingTime, receiveTimestamp);
  }

  // Upon receiving a message from a robot that we aren't synchronized with, force sending an NTP request at next occasion.
  if(!remoteSMB.isValid())
    BNTPRequestComponent::priority = 1;
}

void BNTP::sndRequest(BNTPRequestComponent * comp) {
  BNTPRequestComponent::priority = 0;
}

void BNTP::sndResponse(BNTPResponseComponent * comp) {
  // Respond to buffered requests.
  comp->messages.clear();
  for(const auto& pair : receivedNTPRequests)
  {
    const BNTPRequest& request = pair.second;
    BNTPMessage ntpMessage;
    ntpMessage.receiver = pair.first;
    ntpMessage.requestOrigination = request.origination;
    ntpMessage.requestReceipt = request.receipt;
    comp->messages.emplace_back(ntpMessage);
  }
  
  const_cast<std::unordered_map<std::uint8_t, BNTPRequest>&>(receivedNTPRequests).clear();
  BNTPResponseComponent::priority = 0;
}

void BNTP::update() {
    // Send NTP requests to teammates?
  if (theFrameInfo.getTimeSince(lastNTPRequestSent) >= ntpRequestInterval) {
    const_cast<unsigned&>(lastNTPRequestSent) = theFrameInfo.time;
    BNTPRequestComponent::priority = 1;
  }
    
}

void SynchronizationMeasurementsBuffer::update(int newOffset, unsigned newRoundTrip, unsigned receiveTimestamp)
{
  const unsigned driftBound = 2 * (receiveTimestamp - timestamp) / clockDriftDivider;
  if(!timestamp ||
     newRoundTrip < roundTrip + 2 * driftBound ||
     2 * static_cast<unsigned>(std::abs(newOffset - offset)) > newRoundTrip + roundTrip + 2 * driftBound)
  {
    offset = newOffset;
    roundTrip = newRoundTrip;
    timestamp = receiveTimestamp;
  }
}

void SynchronizationMeasurementsBuffer::validate(unsigned sendTimestamp, unsigned receiveTimestamp)
{
  // Is the buffer already invalid?
  if(!timestamp)
    return;
  // Calculate an interval within which the send timestamp must have been.
  const int exactRemoteTimestamp = static_cast<int>(receiveTimestamp) + offset;
  const int errorBound = (roundTrip + 1u) / 2u + 2 * (receiveTimestamp - timestamp) / clockDriftDivider;
  unsigned lowestPossibleRemoteTimestamp = exactRemoteTimestamp - errorBound - maxPacketDelay;
  unsigned highestPossibleRemoteTimestamp = exactRemoteTimestamp + errorBound;
  if(sendTimestamp < lowestPossibleRemoteTimestamp || sendTimestamp > highestPossibleRemoteTimestamp)
    timestamp = 0;
}
