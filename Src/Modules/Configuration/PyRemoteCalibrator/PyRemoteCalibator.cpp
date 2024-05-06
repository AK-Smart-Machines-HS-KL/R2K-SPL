#include "PyRemoteCalibrator.h"

#define PY_MAX_PACKAGE_SEND_SIZE 6000000
#define PY_MAX_PACKAGE_RECEIVE_SIZE 3000000

thread_local PyRemoteCalibrator* PyRemoteCalibrator::theInstance = nullptr;

PyRemoteCalibrator::PyRemoteCalibrator() : 
    tcpConnection("192.168.50.99", 4242, PY_MAX_PACKAGE_SEND_SIZE, PY_MAX_PACKAGE_RECEIVE_SIZE)
{
    printf("PyRemoteCalibrator created\n");
    
    if(tcpConnection.connected())
    {
        printf("Connected to host\n");
    }
    else
    {
        printf("Failed to connect to host\n");
    }
    theInstance = this;
    read(theCameraSettings, fileName);
}

PyRemoteCalibrator::~PyRemoteCalibrator()
{
    printf("PyRemoteCalibrator destroyed\n");
    theInstance = nullptr;
}

void PyRemoteCalibrator::read(std::unique_ptr<CameraSettings>& theRepresentation, const char* fileName)
{
    ASSERT(!theRepresentation);
    theRepresentation = std::make_unique<CameraSettings>();
    loadModuleParameters(*theRepresentation, TypeRegistry::demangle(typeid(CameraSettings).name()).c_str(), fileName);
}

// Receives and prints a heartbeat response from the host PC
void PyRemoteCalibrator::receiveResponse() {
    unsigned char buffer[4];  // Adjust to the expected integer size (e.g., 4 bytes for int)
    if (tcpConnection.receive(buffer, sizeof(buffer), true)) {
        int heartbeatValue = 0;
        std::memcpy(&heartbeatValue, buffer, sizeof(heartbeatValue));  // Copy bytes to int
        printf("Received heartbeat value: %d\n", heartbeatValue);
    } else {
        printf("No response received.\n");
    }
}


void PyRemoteCalibrator::update(CameraSettings& cameraSettings)
{   
    if(tcpConnection.connected())
    {
        if(tcpConnection.sendHeartbeat())
        {
            printf("Heartbeat sent\n");
        }
        else
        {
            printf("Failed to send heartbeat\n");
        }

    }
    receiveResponse();
    if(theCameraSettings)
    {
        //printf("CameraSettings updated\n");
        //CameraSettings::Collection& camera = theCameraSettings->cameras[0];
        //camera.settings[CameraSettings::Collection::CameraSetting::contrast] = 100;
        
        cameraSettings = *theCameraSettings;

        theCameraSettings = nullptr;
    }
}

MAKE_MODULE(PyRemoteCalibrator, infrastructure);