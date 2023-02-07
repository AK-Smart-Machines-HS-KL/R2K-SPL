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
#include <string>
#include <condition_variable>

MODULE(BeepBroadcaster,
{,
  PROVIDES(BeepCommData),
  REQUIRES(EnhancedKeyStates),

});

struct BeepRequest {
  float duration;
  float volume;
  std::vector<float> frequencies;
};

class BeepBroadcaster : public BeepCommsBase
{
private:
  bool buttonToggle = true;

  // Async
  std::mutex mtx;
  std::thread workerThread;
  std::list<BeepRequest> requestQueue;
  std::condition_variable workerSignal;
  bool shutdownWorkers = false;

  void update(BeepCommData& audioData) override;
  void requestMultipleFrequencies(float duration, float volume, std::vector<float> frequencies);
  void stopWorkers();
  void startWorkers();
  void init_pcm();
  void handleBeepRequests();


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
