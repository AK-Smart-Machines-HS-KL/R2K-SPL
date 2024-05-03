#include "PyRemoteCalibrator.h"

thread_local PyRemoteCalibrator* PyRemoteCalibrator::theInstance = nullptr;

PyRemoteCalibrator::PyRemoteCalibrator()
{
    theInstance = this;
    read(theCameraSettings, fileName);
}

PyRemoteCalibrator::~PyRemoteCalibrator()
{
    theInstance = nullptr;
}

void PyRemoteCalibrator::read(std::unique_ptr<CameraSettings>& theRepresentation, const char* fileName)
{
    ASSERT(!theRepresentation);
    theRepresentation = std::make_unique<CameraSettings>();
    loadModuleParameters(*theRepresentation, TypeRegistry::demangle(typeid(CameraSettings).name()).c_str(), fileName);
}

void PyRemoteCalibrator::update(CameraSettings& cameraSettings)
{
    if(theCameraSettings)
    {
        //printf("CameraSettings updated\n");
        CameraSettings::Collection& camera = theCameraSettings->cameras[0];
        camera.settings[CameraSettings::Collection::CameraSetting::contrast] = 100;
        
        cameraSettings = *theCameraSettings;

        //theCameraSettings = nullptr;
    }
}