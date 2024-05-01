#include "PythonRemoteRobot.h"
#include "ConsoleRoboCupCtrl.h"

PythonRemoteRobot::PythonRemoteRobot(const std::string& robotName, const std::string& ip) :
  RobotTextConsole(Settings("Nao", "Nao"), robotName, nullptr, nullptr), ip(ip)
{ 
  ctrl->printLn("PythonRemoteRobot::PythonRemoteRobot()");
  mode = SystemCall::remoteRobot;
  // try to connect for one second
  //Thread::start(this, &PythonRemoteRobot::connect);
  //Thread::stop();

  // try to connect for one second
  Thread::start(this, &PythonRemoteRobot::connectToPC);
  const unsigned char* dataToSend = reinterpret_cast<const unsigned char*>(robotName.data());
  unsigned char* dataReceived = nullptr;
  int receivedSize = 0;
  if (sendAndReceive(dataToSend, robotName.size(), dataReceived, receivedSize)) {
      ctrl->printLn("Robot name sent successfully.");
      if (receivedSize > 0) {
          std::string response(reinterpret_cast<char*>(dataReceived), receivedSize);
          ctrl->printLn( "Received from host: " );
          free(dataReceived); // Always remember to free received data
      }
  } else {
      ctrl->printLn("Failed to send robot name.");
  }
  Thread::stop();
}

void PythonRemoteRobot::connectToPC()
{
  ctrl->printLn("PythonRemoteRobot::connect()");
  Thread::nameCurrentThread(robotName + ".PythonRemoteRobot.connect");
  TcpConnection::connect(pyHostIp.c_str(), pyHostPort, TcpConnection::sender);
}

void PythonRemoteRobot::connect()
{
  ctrl->printLn("PythonRemoteRobot::connect()");
  Thread::nameCurrentThread(robotName + ".PythonRemoteRobot.connect");
  TcpConnection::connect(ip.c_str(), 9999, TcpConnection::sender);
}

bool PythonRemoteRobot::main()
{
  return false;
}

void PythonRemoteRobot::announceStop()
{
  {
    SYNC;
    debugSender->out.bin << DebugRequest("disableAll");
    debugSender->out.finishMessage(idDebugRequest);
  }
    Thread::sleep(1000);
    Thread::announceStop();
}

void PythonRemoteRobot::update()
{
  //ctrl->printLn("PythonRemoteRobot::update()");
}