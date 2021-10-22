/**
 * @file Tools/Communication/SPLMessageHandler.h
 * The file declares a class for the team communication between robots.
 *
 * @author <a href="mailto:jesse@tzi.de">Jesse Richter-Klug</a>
 *
 * based on TeamHandler authored by
 * @author Thomas Röfer
 */

#pragma once

#include "Tools/Communication/UdpComm.h"
#include "Tools/Communication/RoboCupGameControlData.h"
#include "Tools/Communication/SPLStandardMessageBuffer.h"

/**
 * @class SPLMessageHandler
 * A class for team communication by broadcasting.
 */
class SPLMessageHandler
{
public:
  using Buffer = SPLStandardMessageBuffer<9>;

  /**
   * Constructor.
   * @param in Incoming spl standard messages.
   * @param out Outgoing spl standard message.
   */
  SPLMessageHandler(Buffer& in, RoboCup::SPLStandardMessage& out) :
    in(in), out(out) {}

  /**
   * The method starts the actual communication for local communication.
   * @param port The UDP port this handler is listening to.
   * @param localId An identifier for a local robot
   */
  void startLocal(int port, unsigned localId);

  /**
   * The method starts the actual communication on the given port.
   * @param port The UDP port this handler is listening to.
   * @param subnet The subnet the handler is broadcasting to.
   */
  void start(int port, const char* subnet);

  /**
   * The method sends the outgoing message if possible.
   */
  void send();

  /**
   * The method receives packets if available.
   */
  void receive();

private:
  Buffer& in; /**< Incoming spl messages are stored here. */
  RoboCup::SPLStandardMessage& out; /**< Outgoing spl message is stored here. */
  int port = 0; /**< The UDP port this handler is listening to. */
  UdpComm socket; /**< The socket used to communicate. */
  unsigned localId = 0; /**< The id of a local team communication participant or 0 for normal udp communication. */
};
