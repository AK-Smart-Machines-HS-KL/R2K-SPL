#include "PyRemoteCalibrator.h"

#define PY_MAX_PACKAGE_SEND_SIZE 6000000
#define PY_MAX_PACKAGE_RECEIVE_SIZE 3000000

thread_local PyRemoteCalibrator* PyRemoteCalibrator::theInstance = nullptr;

PyRemoteCalibrator::PyRemoteCalibrator() : 
    tcpConnection("192.168.50.99", 4242, 0, 0)
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

void PyRemoteCalibrator::receiveCameraSettings2() {
    unsigned char buffer[40];  // Buffer to hold the message, 40 bytes to accommodate all settings
    bool received = tcpConnection.receive(buffer, sizeof(buffer), false);  // Non-blocking receive

    if (received) {
        printf("Received data array:\n");
        for (size_t i = 0; i < sizeof(buffer); ++i) {
            printf("%d ", buffer[i]);
        }
        printf("\n");

        // Unpack and update camera settings
        if (theCameraSettings) {
            CameraSettings::Collection& camera = theCameraSettings->cameras[0];

            size_t offset = 0;
            camera.settings[CameraSettings::Collection::autoExposure] = buffer[offset];
            offset += 1;

            int32_t autoExposureBrightness;
            std::memcpy(&autoExposureBrightness, buffer + offset, sizeof(autoExposureBrightness));
            camera.settings[CameraSettings::Collection::autoExposureBrightness] = autoExposureBrightness;
            offset += sizeof(autoExposureBrightness);

            int32_t exposure;
            std::memcpy(&exposure, buffer + offset, sizeof(exposure));
            camera.settings[CameraSettings::Collection::exposure] = exposure;
            offset += sizeof(exposure);

            int32_t gain;
            std::memcpy(&gain, buffer + offset, sizeof(gain));
            camera.settings[CameraSettings::Collection::gain] = gain;
            offset += sizeof(gain);

            camera.settings[CameraSettings::Collection::autoWhiteBalance] = buffer[offset];
            offset += 1;

            camera.settings[CameraSettings::Collection::autoFocus] = buffer[offset];
            offset += 1;

            int32_t focus;
            std::memcpy(&focus, buffer + offset, sizeof(focus));
            camera.settings[CameraSettings::Collection::focus] = focus;
            offset += sizeof(focus);

            camera.settings[CameraSettings::Collection::autoHue] = buffer[offset];
            offset += 1;

            int32_t hue;
            std::memcpy(&hue, buffer + offset, sizeof(hue));
            camera.settings[CameraSettings::Collection::hue] = hue;
            offset += sizeof(hue);

            camera.settings[CameraSettings::Collection::saturation] = buffer[offset];
            offset += 1;

            camera.settings[CameraSettings::Collection::contrast] = buffer[offset];
            offset += 1;

            camera.settings[CameraSettings::Collection::sharpness] = buffer[offset];
            offset += 1;

            int32_t redGain;
            std::memcpy(&redGain, buffer + offset, sizeof(redGain));
            camera.settings[CameraSettings::Collection::redGain] = redGain;
            offset += sizeof(redGain);

            int32_t greenGain;
            std::memcpy(&greenGain, buffer + offset, sizeof(greenGain));
            camera.settings[CameraSettings::Collection::greenGain] = greenGain;
            offset += sizeof(greenGain);

            int32_t blueGain;
            std::memcpy(&blueGain, buffer + offset, sizeof(blueGain));
            camera.settings[CameraSettings::Collection::blueGain] = blueGain;
            offset += sizeof(blueGain);

            printf("Updated camera settings successfully.\n");
        }
    }
}

void PyRemoteCalibrator::update(CameraSettings& cameraSettings)
{   
    if(tcpConnection.connected())
    {
        receiveCameraSettings2();
    }
    if(theCameraSettings)
    {
        cameraSettings = *theCameraSettings;
        //theCameraSettings = nullptr;
    }

}

MAKE_MODULE(PyRemoteCalibrator, infrastructure);