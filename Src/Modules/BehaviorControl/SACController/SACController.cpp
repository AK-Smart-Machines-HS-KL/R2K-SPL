#include "SACController.h"
#include "Platform/SystemCall.h"

thread_local SACController* SACController::theInstance = nullptr;

SACController::SACController() : 
    tcpConnection("192.168.50.99", 5050, 0, 0)
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

enum DirectionId {
    Stop = 0,
    Forward,
    Backward,
    Left,
    Right,
    Leftwards,
    Rightwards
};

enum BehaviorId {
    defenseChaseBallCard = 0,
    offenseChaseBallCard,
    clearOwnHalfCard,
    clearOwnHalfCardGoalie,
    defenseCard,
    defenseLongShotCard,
    dive,
    goalShotCard,
    goalieLongShotCard,
    offenseFastGoalKick,
    offenseForwardPassCard,
    offenseReceivePassCard
};

void SACController::update(SACCommands& saccommands)
{
    if(tcpConnection.connected())
    {
        receiveMessage(saccommands);
    }
}

void SACController::receiveMessage(SACCommands& saccommands) {
    unsigned char buffer[2];  // Adjust buffer size if necessary
    int bytesReceived = tcpConnection.receive(buffer, sizeof(buffer), false);

    if (bytesReceived > 0) {
        // First byte is the message ID
        unsigned char messageId = buffer[0];
        // Second byte is the value (behavior ID, mode, or direction ID)
        unsigned char messageValue = buffer[1];

        // Check the message ID to determine if it's a behavior, mode, or direction
        if (messageId == 0x02) {
            // Behavior ID
            saccommands.cardIdx = messageValue;  // Update cardIdx

            switch (messageValue) {
                case defenseChaseBallCard:
                    OUTPUT_TEXT("Received behavior: defenseChaseBallCard\n");
                    break;
                case offenseChaseBallCard:
                    OUTPUT_TEXT("Received behavior: offenseChaseBallCard\n");
                    break;
                case clearOwnHalfCard:
                    OUTPUT_TEXT("Received behavior: clearOwnHalfCard\n");
                    break;
                case clearOwnHalfCardGoalie:
                    OUTPUT_TEXT("Received behavior: clearOwnHalfCardGoalie\n");
                    break;
                case defenseCard:
                    OUTPUT_TEXT("Received behavior: defenseCard\n");
                    break;
                case defenseLongShotCard:
                    OUTPUT_TEXT("Received behavior: defenseLongShotCard\n");
                    break;
                case dive:
                    OUTPUT_TEXT("Received behavior: dive\n");
                    break;
                case goalShotCard:
                    OUTPUT_TEXT("Received behavior: goalShotCard\n");
                    break;
                case goalieLongShotCard:
                    OUTPUT_TEXT("Received behavior: goalieLongShotCard\n");
                    break;
                case offenseFastGoalKick:
                    OUTPUT_TEXT("Received behavior: offenseFastGoalKick\n");
                    break;
                case offenseForwardPassCard:
                    OUTPUT_TEXT("Received behavior: offenseForwardPassCard\n");
                    break;
                case offenseReceivePassCard:
                    OUTPUT_TEXT("Received behavior: offenseReceivePassCard\n");
                    break;
                default:
                    OUTPUT_TEXT("Unknown behavior ID\n");
                    break;
            }
        } else if (messageId == 0x03) {
            // Mode
            saccommands.mode = messageValue;  // Update mode

            if (messageValue == 0) {
                OUTPUT_TEXT("Received mode: 0 (Auto mode)\n");
            } else if (messageValue == 1) {
                OUTPUT_TEXT("Received mode: 1 (Human Operator Mode)\n");
            } else {
                OUTPUT_TEXT("Unknown mode\n");
            }
        } else if (messageId == 0x04) {
            // Direction ID
            saccommands.direction = messageValue;  // Update direction
            
            switch (messageValue) {
                case Stop:
                    OUTPUT_TEXT("Received direction: Stop\n");
                    break;
                case Forward:
                    OUTPUT_TEXT("Received direction: Forward\n");
                    break;
                case Backward:
                    OUTPUT_TEXT("Received direction: Backward\n");
                    break;
                case Left:
                    OUTPUT_TEXT("Received direction: Left\n");
                    break;
                case Right:
                    OUTPUT_TEXT("Received direction: Right\n");
                    break;
                case Leftwards:
                    OUTPUT_TEXT("Received direction: Leftwards\n");
                    break;
                case Rightwards:
                    OUTPUT_TEXT("Received direction: Rightwards\n");
                    break;
                default:
                    OUTPUT_TEXT("Unknown direction ID\n");
                    break;
            }
        } else {
            OUTPUT_TEXT("Unknown message ID\n");
        }
    }
}

MAKE_MODULE(SACController, behaviorControl);