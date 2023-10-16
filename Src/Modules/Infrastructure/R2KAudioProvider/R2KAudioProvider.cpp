/**
 * @file R2KAudioProvider.cpp
 * This file implements a module that provides audio samples using PortAudio. Code from NaoDevils.
 * @author Aaron Larisch, Thomas Jäger
 */

#include "R2KAudioProvider.h"
#include "Tools/Settings.h"
#include "Platform/SystemCall.h"
#include "Platform/Thread.h"
#include <type_traits>

MAKE_MODULE(R2KAudioProvider, infrastructure);

//#ifdef TARGET_ROBOT

std::mutex R2KAudioProvider::mutex;


R2KAudioProvider::R2KAudioProvider()
{
  // PortAudio API is not thread-safe.
  std::lock_guard<std::mutex> l(mutex);

  PaError paerr = Pa_Initialize();
  if (paerr != paNoError)
  {
    OUTPUT_ERROR("PortAudioRecorder: Failed to initialize: " << Pa_GetErrorText(paerr) << "(" << paerr << ")");
    return;
  }
}

R2KAudioProvider::~R2KAudioProvider()
{
 
}

void R2KAudioProvider::update(R2KAudioData& r2kAudioData)
{
  
  DEBUG_RESPONSE_ONCE("module:R2KAudioProvider:leggo")
  {
  int numDevices = Pa_GetDeviceCount();
  if(numDevices < 0) {
        OUTPUT_TEXT("Failed to retrieve device count: " << Pa_GetErrorText(numDevices));
        Pa_Terminate();
    }
  OUTPUT_TEXT("Number of devices:" << numDevices);
  
  for(int i = 0; i < numDevices; i++) {

      const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);

      printf("Device #%d:\n", i);
      printf("Name: %s\n", deviceInfo->name);
      printf("Max Input Channels: %d\n", deviceInfo->maxInputChannels);
      printf("Max Output Channels: %d\n", deviceInfo->maxOutputChannels);
      } 


  //Pa_Terminate();
  } // DR
  
  
  r2kAudioData.dummyData = 1;
}

/*
#else // !defined TARGET_ROBOT

R2KAudioProvider::R2KAudioProvider() {}
R2KAudioProvider::~R2KAudioProvider() {}
void R2KAudioProvider::update(R2KAudioData&) {}

#endif
*/