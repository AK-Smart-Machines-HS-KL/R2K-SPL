/**
 * @file BeepBroadcaster.h
 * This file declares a module that provides the sending part of the audio communication system.
 * 
 * @author Nicolas Fortune, Andy Hobelsberger
 */

#pragma once

#ifdef TARGET_ROBOT
#include <alsa/asoundlib.h>
#endif
#include "Tools/Module/Module.h"
#include "Platform/Thread.h"
#include "Representations/Infrastructure/BeepCommData.h"
#include "Representations/Infrastructure/SensorData/KeyStates.h"
#include "Representations/Sensing/GroundContactState.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Communication/Beep.h"
#include <string>
#include <condition_variable>
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Infrastructure/AudioData.h"

MODULE(BeepBroadcaster,
{,
  PROVIDES(BeepCommData),
  REQUIRES(EnhancedKeyStates),
  REQUIRES(RobotInfo),
  REQUIRES(GroundContactState),
  REQUIRES(Beep),
  REQUIRES(FrameInfo),
  LOADS_PARAMETERS(
  {,
    (int) numBands,
    (int) encodedBits,
    (float) baseFrequency,
    (float) bandWidth,
    (int) headButtonMessage,
  }),
});

struct BeepRequest {
  float duration;
  float volume;
  std::vector<float> frequencies;
};

class BeepBroadcaster : public BeepBroadcasterBase
{
private:
  bool buttonToggle = true;
  bool sensorToggle = true;
  bool responseToggle = true;
  bool sendToggle=false;
  int sendmessage = 0;
  int maxTimeOutBetweenBeeps = 500;
  int lastTimeWithBeep = 0;


  // Async
  std::mutex mtx;
  std::thread workerThread;
  std::list<BeepRequest> requestQueue;
  std::condition_variable workerSignal;
  bool shutdown = false;

  void update(BeepCommData& audioData) override;
  void requestMultipleFrequencies(float duration, float volume, std::vector<float> frequencies);
  void requestMessageBroadcast(float duration, float volume, int message);
  void stopWorker();
  void startWorker();
  void init_pcm();
  void handleBeepRequests();
  void groundBeep(BeepCommData& beepCommData, int message);


public:
  /**
   * Default constructor
   */
  BeepBroadcaster();
  /**
   * Destructor.
   */
  ~BeepBroadcaster();
};
