/**
 * @file RobotMessageHandler.h
 * @author <mailto:anho1001@stud.hs-kl.de> Andy Hobelsberger 
 * @brief 
 * @version 1.0
 * @date 2023-05-19
 * 
 * @copyright Copyright (c) 2023
 * 
 * based on SPLMessageHandler.cpp authored by
 * @author <a href="mailto:jesse@tzi.de">Jesse Richter-Klug</a>
 */

#pragma once

#include <queue>

#include "Tools/Communication/UdpComm.h"
#include "Tools/Communication/RoboCupGameControlData.h"
#include "RobotMessage.h"

/**
 * @class RobotMessageHandler
 * A class for team communication by broadcasting.
 */
class RobotMessageHandler
{
public:

  /**
   * The method starts the actual communication for local communication.
   * @param port The UDP port this handler is listening to.
   * @param localId An identifier for a local robot
   */
  void startLocal(int port, unsigned localId);

  void connectLocal(int port, unsigned localId);

  /**
   * The method starts the actual communication on the given port.
   * @param port The UDP port this handler is listening to.
   * @param subnet The subnet the handler is broadcasting to.
   */
  void start(int port, const char* subnet);

  void connect(int port, const char* subnet);

  /**
   * The method sends the outgoing message if possible.
   */
  void send();

  /**
   * The method receives packets if available.
   */
  void receive();

private:
  std::array<uint8_t, SPL_MAX_MESSAGE_BYTES> readBuffer;
  std::array<uint8_t, SPL_MAX_MESSAGE_BYTES> writeBuffer;
  std::queue<RobotMessage> incoming; /**< Incoming messages are stored here. */
  int port = 0; /**< The UDP port this handler is listening to. */
  UdpComm socket; /**< The socket used to communicate. */
  unsigned localId = 0; /**< The id of a local team communication participant or 0 for normal udp communication. */
};
