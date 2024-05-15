#pragma once


#include "Representations/Configuration/CameraSettings.h"
#include "Tools/Communication/TcpComm.h"
#include "Tools/Module/Module.h"


// Enum to identify settings
enum CameraSettingId {
    autoExposure = 0x01,
    autoExposureBrightness = 0x02,
    exposure = 0x03,
    gain = 0x04,
    autoWhiteBalance = 0x05,
    autoFocus = 0x06,
    focus = 0x07,
    autoHue = 0x08,
    hue = 0x09,
    saturation = 0x0A,
    contrast = 0x0B,
    sharpness = 0x0C,
    redGain = 0x0D,
    greenGain = 0x0E,
    blueGain = 0x0F
};

MODULE(PyRemoteCalibrator,
{,
  PROVIDES(CameraSettings),
});

class PyRemoteCalibrator : public PyRemoteCalibratorBase
{

private:
    static thread_local PyRemoteCalibrator* theInstance;

    std::unique_ptr<CameraSettings> theCameraSettings;
    const char* fileName = nullptr;
    
    void update(CameraSettings& cameraSettings) override;

    void read(std::unique_ptr<CameraSettings>& theRepresentation, const char* fileName);

    void receiveContrastValue();

    void receiveSettingValue();

    void updateBooleanSetting(CameraSettingId settingId, unsigned char value);
    void updateByteSetting(CameraSettingId settingId, unsigned char value);
    void updateIntegerSetting(CameraSettingId settingId, const unsigned char* value);

    TcpComm tcpConnection;

public:
  PyRemoteCalibrator();
  ~PyRemoteCalibrator();
};