/**
 * @file BeepComms.h
 * This file declares a module that provides audio samples.
 * @author Thomas Röfer
 * @author Lukas Plecher
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

MODULE(BeepComms,
{,
  PROVIDES(BeepCommData),
  REQUIRES(EnhancedKeyStates),

});

class BeepComms : public BeepCommsBase
{
private:
  bool test = true;
  int running = 0;
  std::thread beepThread;
#ifdef TARGET_ROBOT
  snd_pcm_t* handle;
#endif
  void update(BeepCommData& audioData) override;
  void playMultipleFrequencies(int robot, int frequency1, int frequency2, int frequency3, int frequency4, int frequency5, float duration, int volume);
  void PlayerOpen(int robot, int volume);
  void CheckFacing(int robot, int volume);
  void BallLost(int robot, int volume);

public:
  /**
   * Default constructor
   */
  BeepComms();

  /**
   * Destructor.
   */
  ~BeepComms();
};
