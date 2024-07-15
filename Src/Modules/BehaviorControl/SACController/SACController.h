#pragma once


# include "Representations/Infrastructure/DummyRepresentation.h"
#include "Tools/Communication/TcpComm.h"
#include "Tools/Module/Module.h"


MODULE(SACController,
{,
  PROVIDES(DummyRepresentation),
});

class SACController : public SACControllerBase
{

private:
    static thread_local SACController* theInstance;

    TcpComm tcpConnection;

    void update(DummyRepresentation& dummyRepresentation) override;

public:
  SACController();
  ~SACController();
};