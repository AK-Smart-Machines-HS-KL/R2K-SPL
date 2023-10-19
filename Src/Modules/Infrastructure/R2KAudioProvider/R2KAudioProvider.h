/**
 * @file PortAudioRecorder.h
 * This file declares a module that provides audio samples using PortAudio.
 * @author Aaron Larisch, Thomas Röfer
 * @author R2K Thomas
 */

#pragma once

//#ifdef TARGET_ROBOT
//#include <portaudio.h>
//endif
#include "Tools/Module/Module.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Configuration/DamageConfiguration.h"
#include "Representations/Infrastructure/R2KAudioData.h"
#include <string>
#include <mutex>

MODULE(R2KAudioProvider,
{,  
  REQUIRES(FrameInfo),
  PROVIDES_WITHOUT_MODIFY(R2KAudioData),
  LOADS_PARAMETERS(
  {,  
    (std::string) deviceName, /**< Name of audio device. */
    (unsigned) channels, /**< Number of channels to capture. */
    (unsigned) sampleRate, /**< Sample rate to capture. */
    (float) latency, 
   }),
});

class R2KAudioProvider : public R2KAudioProviderBase
{
private:
  void update(R2KAudioData& r2kAudioData);
  /*
  PaStream* stream = nullptr;
  PaStreamParameters inputParameters;
  unsigned noDataCount = 0;
  static std::mutex mutex;
  */

public:
  R2KAudioProvider();
  ~R2KAudioProvider();
};
