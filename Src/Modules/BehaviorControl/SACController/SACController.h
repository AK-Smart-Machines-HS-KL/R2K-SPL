#pragma once


# include "Representations/BehaviorControl/SACCommands.h"
#include "Tools/Communication/TcpComm.h"
#include "Tools/Module/Module.h"


MODULE(SACController,
{,
  PROVIDES(SACCommands),
});

class SACController : public SACControllerBase
{

private:
    static thread_local SACController* theInstance;

    TcpComm tcpConnection;

    void update(SACCommands& SACCommands) override;

    void receiveMessage();

public:
  SACController();
  ~SACController();
};