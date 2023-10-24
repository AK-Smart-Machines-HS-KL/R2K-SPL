/**
 * @file PortAudioRecorder.h
 * This file declares a module that provides audio samples using PortAudio.
 * @author Aaron Larisch, Thomas Röfer
 * @author R2K Thomas
 */

#pragma once

//#include <portaudio.h>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/optional_debug_tools.h>
#include "Representations/Infrastructure/R2KAudioData.h"

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
    (std::string) filename,
   }),
});

class R2KAudioProvider : public R2KAudioProviderBase
{
public:
  R2KAudioProvider();
private:

  //std::unique_ptr<tflite::FlatBufferModel> model;

  void update(R2KAudioData& r2kAudioData);
};

