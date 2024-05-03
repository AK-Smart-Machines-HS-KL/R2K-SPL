#include "PyRemoteCalibrator.h"

thread_local PyRemoteCalibrator* PyRemoteCalibrator::theInstance = nullptr;

PyRemoteCalibrator::PyRemoteCalibrator()
{
    theInstance = this;
    read(theCameraSettings);
}

PyRemoteCalibrator::~PyRemoteCalibrator()
{
    theInstance = nullptr;
}

void PyRemoteCalibrator::update(CameraSettings& cameraSettings)
{
    if(theCameraSettings)
    {
        cameraSettings = *theCameraSettings;
        theCameraSettings = nullptr;
    }
}