#pragma once

#include "Tools/Debugging/TcpConnection.h"
#include "RobotTextConsole.h"


/**
 * @class PythonRemoteRobot
 *
 * A class representing a thread that communicates with a remote robot via TCP.
 * The class establishes a socket connection between the robot and a python application.
 */
class PythonRemoteRobot : public RobotTextConsole, public TcpConnection
{
private:
  const std::string ip; /**< The ip of the robot. */

public:
  /**
   * @param robotName The name of the robot.
   * @param ip The ip address of the robot.
   */
  PythonRemoteRobot(const std::string& robotName, const std::string& ip);

  /**
   * The function is called to announce the termination of the thread.
   */
  void announceStop() override;

  /**
   * The function must be called to exchange data with SimRobot.
   * It sends the motor commands to SimRobot and acquires new sensor data.
   */
  void update() override;

protected:
  /**
   * That function is called once before the first main(). It can be used
   * for things that can't be done in the constructor.
   */
  void init() override
  {
    RobotTextConsole::init();
    Thread::nameCurrentThread(robotName + ".PythonRemoteRobot");
  }

  /**
   * The function is called from the framework once in every frame.
   */
  bool main() override;

private:
  /**
   * The function connects to another thread.
   */
  void connect();

};