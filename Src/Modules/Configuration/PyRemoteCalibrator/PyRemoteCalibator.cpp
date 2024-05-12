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

// Receives and updates the contrast value from the host PC
void PyRemoteCalibrator::receiveContrastValue() {
    unsigned char buffer[4];  // Buffer to receive 4 bytes (int)
    int bytesReceived = tcpConnection.receive(buffer, sizeof(buffer), false); // Non-blocking receive

    if (bytesReceived > 0) {
        int newContrastValue = 0;
        std::memcpy(&newContrastValue, buffer, sizeof(newContrastValue));  // Copy bytes to int
        printf("Received contrast value: %d\n", newContrastValue);

        // Update the contrast setting in theCameraSettings
        if (theCameraSettings) {
            CameraSettings::Collection& camera = theCameraSettings->cameras[0];
            camera.settings[CameraSettings::Collection::CameraSetting::contrast] = newContrastValue;
            printf("Updated camera contrast to: %d\n", newContrastValue);
        }
    }
}


void PyRemoteCalibrator::update(CameraSettings& cameraSettings)
{   
    if(tcpConnection.connected())
    {
        receiveContrastValue();
    }
    if(theCameraSettings)
    {
        cameraSettings = *theCameraSettings;
        //theCameraSettings = nullptr;
    }

}

MAKE_MODULE(PyRemoteCalibrator, infrastructure);