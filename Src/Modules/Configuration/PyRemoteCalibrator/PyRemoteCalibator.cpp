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

// Receives and updates camera settings from the host PC
void PyRemoteCalibrator::receiveCameraSettings() {
    char buffer[256];  // Buffer to receive setting:value
    int bytesReceived = tcpConnection.receive(reinterpret_cast<unsigned char*>(buffer), sizeof(buffer) - 1, false); // Non-blocking receive

    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';  // Null-terminate the received string
        std::string receivedData(buffer);

        // Parse setting and value
        std::istringstream iss(receivedData);
        std::string setting;
        int value;
        if (std::getline(iss, setting, ':') && iss >> value) {
            printf("Received %s value: %d\n", setting.c_str(), value);

            CameraSettings::Collection& camera = theCameraSettings->cameras[0];
            if (setting == "autoExposure") camera.settings[CameraSettings::Collection::autoExposure] = value;
            else if (setting == "autoExposureBrightness") camera.settings[CameraSettings::Collection::autoExposureBrightness] = value;
            else if (setting == "exposure") camera.settings[CameraSettings::Collection::exposure] = value;
            else if (setting == "gain") camera.settings[CameraSettings::Collection::gain] = value;
            else if (setting == "autoWhiteBalance") camera.settings[CameraSettings::Collection::autoWhiteBalance] = value;
            else if (setting == "autoFocus") camera.settings[CameraSettings::Collection::autoFocus] = value;
            else if (setting == "focus") camera.settings[CameraSettings::Collection::focus] = value;
            else if (setting == "autoHue") camera.settings[CameraSettings::Collection::autoHue] = value;
            else if (setting == "hue") camera.settings[CameraSettings::Collection::hue] = value;
            else if (setting == "saturation") camera.settings[CameraSettings::Collection::saturation] = value;
            else if (setting == "contrast") camera.settings[CameraSettings::Collection::contrast] = value;
            else if (setting == "sharpness") camera.settings[CameraSettings::Collection::sharpness] = value;
            else if (setting == "redGain") camera.settings[CameraSettings::Collection::redGain] = value;
            else if (setting == "greenGain") camera.settings[CameraSettings::Collection::greenGain] = value;
            else if (setting == "blueGain") camera.settings[CameraSettings::Collection::blueGain] = value;
            printf("Updated %s to: %d\n", setting.c_str(), value);
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