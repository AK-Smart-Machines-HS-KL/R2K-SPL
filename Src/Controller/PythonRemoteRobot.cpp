#include "PythonRemoteRobot.h"
#include "ConsoleRoboCupCtrl.h"

PythonRemoteRobot::PythonRemoteRobot(const std::string& robotName, const std::string& ip) :
  RobotTextConsole(Settings("Nao", "Nao"), robotName, nullptr, nullptr), ip(ip)
{
  mode = SystemCall::remoteRobot;
  // try to connect for one second
  Thread::start(this, &PythonRemoteRobot::connect);
  Thread::stop();
}

void PythonRemoteRobot::connect()
{
  Thread::nameCurrentThread(robotName + ".PythonRemoteRobot.connect");
  TcpConnection::connect(ip.c_str(), 9999, TcpConnection::sender);
}

bool PythonRemoteRobot::main()
{
  unsigned char* sendData = nullptr;
  unsigned char* receivedData;
  int sendSize = 0;
  int receivedSize = 0;
  MessageQueue temp;

  // If there is something to send, prepare a packet
  if(!debugSender->isEmpty())
  {
    SYNC;
    sendSize = static_cast<int>(debugSender->getStreamedSize());
    OutBinaryMemory stream(sendSize);
    stream << *debugSender;
    sendData = reinterpret_cast<unsigned char*>(stream.obtainData());
    // make backup
    debugSender->moveAllMessages(temp);
  }

  // exchange data with the router
  if(!sendAndReceive(sendData, sendSize, receivedData, receivedSize) && sendSize)
  {
    // sending failed, restore theDebugSender
    SYNC;
    // move all messages since cleared (if any)
    debugSender->moveAllMessages(temp);
    // restore
    temp.moveAllMessages(*debugSender);
  }

  // If a packet was prepared, remove it
  if(sendSize)
    delete[] sendData;

  // If a packet was received from the router program, add it to receiver queue
  if(receivedSize > 0)
  {
    SYNC;
    InBinaryMemory stream(receivedData, receivedSize);
    stream >> *debugReceiver;
    delete[] receivedData;
  }
  
  // Convert receivedSize to a string and print it
  std::string receivedSizeStr = std::to_string(receivedSize);
  ctrl->printLn(receivedSizeStr);

  Thread::sleep(receivedSize > 0 ? 1 : 20);
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
    char buf[33];
    QString statusText = QString::fromStdString(robotName) +
                        (isConnected() ? QString(": Python connected to ") + QString::fromStdString(ip)
                          : QString(": connection lost from ") + QString::fromStdString(ip));
    ctrl->printStatusText(statusText);
}