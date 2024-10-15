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

void PyRemoteCalibrator::receiveSettingValue() {
    unsigned char buffer[6];  // Assume maximum needed size is 6 bytes
    int bytesReceived = tcpConnection.receive(buffer, sizeof(buffer), false);

    if (bytesReceived > 0) {
        // First byte is the setting ID
        CameraSettingId settingId = static_cast<CameraSettingId>(buffer[0]);

        switch (settingId) {
            case autoExposure:
                updateBooleanSetting(settingId, buffer[1]);
                break;
            case autoWhiteBalance:
                updateBooleanSetting(settingId, buffer[1]);
                break;
            case autoFocus:
                updateBooleanSetting(settingId, buffer[1]);
                break;
            case autoHue:
                updateBooleanSetting(settingId, buffer[1]);
                break;
            case autoExposureBrightness:
                updateIntegerSetting(settingId, buffer + 1);
                break;
            case exposure:
                updateIntegerSetting(settingId, buffer + 1);
                break;
            case gain:
                updateIntegerSetting(settingId, buffer + 1);
                break;
            case hue:
                updateIntegerSetting(settingId, buffer + 1);
                break;
            case focus:
                updateIntegerSetting(settingId, buffer + 1);
                break;
            case redGain:
                updateIntegerSetting(settingId, buffer + 1);
                break;
            case greenGain:
                updateIntegerSetting(settingId, buffer + 1);
                break;
            case blueGain:
                updateIntegerSetting(settingId, buffer + 1);
                break;
            case saturation:
                updateByteSetting(settingId, buffer[1]);
                break;
            case contrast:
                updateByteSetting(settingId, buffer[1]);
                break;
            case sharpness:
                updateByteSetting(settingId, buffer[1]);
                break;
        }
    }
}

void PyRemoteCalibrator::updateByteSetting(CameraSettingId id, unsigned char value) {
    OUTPUT_TEXT("Received byte setting ID: " << id << "; Value:" << value);
    printf("Received %d to byte value %d\n", id, value);

    if (!theCameraSettings) return; // Check if theCameraSettings pointer is valid

    CameraSettings::Collection& camera = theCameraSettings->cameras[0]; // Assuming single camera handling for simplicity
    switch (id) {
        case saturation:
            camera.settings[CameraSettings::Collection::CameraSetting::saturation] = value;
            printf("Updated saturation to: %d\n", value);
            break;
        case contrast:
            camera.settings[CameraSettings::Collection::CameraSetting::contrast] = value;
            printf("Updated contrast to: %d\n", value);
            break;
        case sharpness:
            camera.settings[CameraSettings::Collection::CameraSetting::sharpness] = value;
            printf("Updated sharpness to: %d\n", value);
            break;
        // Add cases for other settings that also use a single byte
        default:
            printf("Unsupported setting id %d\n", id);
            break;
    }
}

void PyRemoteCalibrator::updateBooleanSetting(CameraSettingId id, unsigned char value) {
    OUTPUT_TEXT("Received boolean setting value");
    printf("Received %d to boolean value %d\n", id, value);

    if (!theCameraSettings) return; // Check if theCameraSettings pointer is valid

    CameraSettings::Collection& camera = theCameraSettings->cameras[0]; // Assuming single camera handling for simplicity
    switch (id) {
        case autoExposure:
            camera.settings[CameraSettings::Collection::CameraSetting::autoExposure] = value;
            printf("Updated autoExposure to: %d\n", value);
            break;
        case autoWhiteBalance:
            camera.settings[CameraSettings::Collection::CameraSetting::autoWhiteBalance] = value;
            printf("Updated autoWhiteBalance to: %d\n", value);
            break;
        case autoFocus:
            camera.settings[CameraSettings::Collection::CameraSetting::autoFocus] = value;
            printf("Updated autoFocus to: %d\n", value);
            break;
        case autoHue:
            camera.settings[CameraSettings::Collection::CameraSetting::autoHue] = value;
            printf("Updated autoHue to: %d\n", value);
            break;
        // Add cases for other settings that also use a single byte
        default:
            printf("Unsupported setting id %d\n", id);
            break;
    }
}

void PyRemoteCalibrator::updateIntegerSetting(CameraSettingId id, const unsigned char* value) {
    OUTPUT_TEXT("Received integer setting value");
    int intValue = 0;
    std::memcpy(&intValue, value, sizeof(intValue));  // Copy bytes to int
    printf("Received %d to integer value %d\n", id, intValue);

    if (!theCameraSettings) return; // Check if theCameraSettings pointer is valid

    CameraSettings::Collection& camera = theCameraSettings->cameras[0]; // Assuming single camera handling for simplicity
    switch (id) {
        case autoExposureBrightness:
            camera.settings[CameraSettings::Collection::CameraSetting::autoExposureBrightness] = intValue;
            printf("Updated autoExposureBrightness to: %d\n", intValue);
            break;
        case exposure:
            camera.settings[CameraSettings::Collection::CameraSetting::exposure] = intValue;
            printf("Updated exposure to: %d\n", intValue);
            break;
        case gain:
            camera.settings[CameraSettings::Collection::CameraSetting::gain] = intValue;
            printf("Updated gain to: %d\n", intValue);
            break;
        case hue:
            camera.settings[CameraSettings::Collection::CameraSetting::hue] = intValue;
            printf("Updated hue to: %d\n", intValue);
            break;
        case focus:
            camera.settings[CameraSettings::Collection::CameraSetting::focus] = intValue;
            printf("Updated focus to: %d\n", intValue);
            break;
        case redGain:
            camera.settings[CameraSettings::Collection::CameraSetting::redGain] = intValue;
            printf("Updated redGain to: %d\n", intValue);
            break;
        case greenGain:
            camera.settings[CameraSettings::Collection::CameraSetting::greenGain] = intValue;
            printf("Updated greenGain to: %d\n", intValue);
            break;
        case blueGain:
            camera.settings[CameraSettings::Collection::CameraSetting::blueGain] = intValue;
            printf("Updated blueGain to: %d\n", intValue);
            break;
        // Add cases for other settings that also use a 4-byte integer
        default:
            printf("Unsupported setting id %d\n", id);
            break;
    }
}

void PyRemoteCalibrator::update(CameraSettings& cameraSettings)
{   
    if(tcpConnection.connected())
    {
        receiveSettingValue();
    }
    if(theCameraSettings)
    {
        cameraSettings = *theCameraSettings;
        //theCameraSettings = nullptr;
    }

}

MAKE_MODULE(PyRemoteCalibrator, infrastructure);