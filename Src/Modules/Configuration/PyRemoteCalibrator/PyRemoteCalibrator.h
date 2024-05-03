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
    
    void update(CameraSettings& cameraSettings) override;

    template<typename T> void read(std::unique_ptr<T>& theRepresentation, const char* fileName = nullptr)
    {
        ASSERT(!theRepresentation);
        theRepresentation = std::make_unique<T>();
        loadModuleParameters(*theRepresentation, TypeRegistry::demangle(typeid(T).name()).c_str(), fileName);
    }
    
public:
  PyRemoteCalibrator();
  ~PyRemoteCalibrator();
};


MAKE_MODULE(PyRemoteCalibrator, infrastructure);
