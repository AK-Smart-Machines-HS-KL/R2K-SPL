#include "SACController.h"
#include "Platform/SystemCall.h"

thread_local SACController* SACController::theInstance = nullptr;

SACController::SACController() : 
    tcpConnection("192.168.50.99", 5000, 0, 0)
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
    if(tcpConnection.connected())
    {
        receiveMessage(saccommands);
    }
}

void SACController::receiveMessage(SACCommands& saccommands) {
    unsigned char buffer[1];  // Adjust buffer size if necessary
    int bytesReceived = tcpConnection.receive(buffer, sizeof(buffer), false);

    if (bytesReceived > 0) {
        // First byte is the message ID
        unsigned char messageId = buffer[0];
        SystemCall::say("Packet:");
        //SystemCall::say(messageId);
        // Second byte is the value (behavior ID, mode, or direction ID)
       // unsigned char messageValue = buffer[1];

        // Check the message ID to determine if it's a behavior, mode, or direction

    }
}

MAKE_MODULE(SACController, behaviorControl);