#pragma once


#include "Representations/Configuration/CameraSettings.h"
#include "Tools/Module/Module.h"

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

public:
  PyRemoteCalibrator();
  ~PyRemoteCalibrator();
};


MAKE_MODULE(PyRemoteCalibrator, infrastructure);
