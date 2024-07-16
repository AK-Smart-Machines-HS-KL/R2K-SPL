#include "SACController.h"
#include "Platform/SystemCall.h"

thread_local SACController* SACController::theInstance = nullptr;

SACController::SACController() : 
    tcpConnection("192.168.50.99", 4242, 0, 0)
{
    printf("SACController created\n");
    
    if(tcpConnection.connected())
    {
        SystemCall::say("SAC");
        printf("Connected to host\n");
    }
    else
    {
        printf("Failed to connect to host\n");
    }
    theInstance = this;
}

SACController::~SACController()
{
    printf("SACController destroyed\n");
    theInstance = nullptr;
}

void SACController::update(SACCommands& saccommands)
{
    saccommands.mode = 1;
}

MAKE_MODULE(SACController, behaviorControl);