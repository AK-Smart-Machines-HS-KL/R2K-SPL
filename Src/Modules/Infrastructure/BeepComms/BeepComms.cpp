/**
 * @file BeepComms.cpp
 * This file implements a module that provides the sending part of the audio communication system.
 */

#include "BeepComms.h"
#include "Platform/SystemCall.h"
#include "Platform/Thread.h"
#include <type_traits>

MAKE_MODULE(BeepComms, infrastructure);


#ifdef TARGET_ROBOT


BeepComms::BeepComms()
{
  //open the audio device
  snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);

  //set the audio format
  snd_pcm_set_params(handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 44100, 1, 500000);
  
}

BeepComms::~BeepComms()
{
  snd_pcm_close(handle);
}

void BeepComms::update(BeepCommData& audioData)
{
  if (theEnhancedKeyStates.isPressedFor(KeyStates::headFront, 100u))
  {
    BallLost(1, 2000);
    test = false;
    OUTPUT_TEXT("Text");
  }
  try
  {
    if (running==0)
    {
      beepThread.join();
    }
  }
  catch (const std::exception&)
  {
  }
  
}

//play 5 sine waves simultaneously
void BeepComms::playMultipleFrequencies(int robot,int frequency1, int frequency2, int frequency3, int frequency4, int frequency5, float duration, int volume){
    running = 1;
    int add = robot * 1000;
    int freq1 = frequency1;
    int freq2 = frequency2;
    int freq3 = frequency3;
    int freq4 = frequency4;
    int freq5 = frequency5;
    float dur = duration;
    int vol = volume;

    //assigning robot 
    if (freq1 != 0)
    {
        freq1 = freq1 + add;
    }
    if (freq2 != 0)
    {
        freq2 = freq2 + add;
    }
    if (freq3 != 0)
    {
        freq3 = freq3 + add;
    }
    if (freq4 != 0)
    {
        freq4 = freq4 + add;
    }
    if (freq5 != 0)
    {
        freq5 = freq5 + add;
    }

    //generate the sine wave
    int buf[44100];
    for (int i = 0; i < 44100; i++){
        buf[i] = vol * sin(2 * M_PI * freq1 * i / 44100);
        buf[i] = buf[i] + vol * sin(2 * M_PI * freq2 * i / 44100);
        buf[i] = buf[i] + vol * sin(2 * M_PI * freq3 * i / 44100);
        buf[i] = buf[i] + vol * sin(2 * M_PI * freq4 * i / 44100);
        buf[i] = buf[i] + vol * sin(2 * M_PI * freq5 * i / 44100);
    }

    //play the sine wave
    for (int i = 0; i < dur * 44100 / 512; i++){
        snd_pcm_writei(handle, buf, 512);
    }
    running = 0;
};


//Player is open
void BeepComms::PlayerOpen(int robot, int volume){
    std::thread beepThread(&BeepComms::playMultipleFrequencies, this, robot, 100, 0, 0, 0, 0, 1, volume);

    beepThread.detach();
};

//Check if facing the wrong direction
void BeepComms::CheckFacing(int robot, int volume){
    std::thread beepThread(&BeepComms::playMultipleFrequencies, this, robot, 100, 200, 0, 0, 0, 1, volume);

    beepThread.detach();
};

//Team has lost the ball
void BeepComms::BallLost(int robot, int volume){
    std::thread beepThread(&BeepComms::playMultipleFrequencies, this, robot, 100, 200, 400, 0, 0, 1, volume);

    beepThread.detach();
};



#else // !defined TARGET_ROBOT

BeepComms::BeepComms() {}
BeepComms::~BeepComms() {}
void BeepComms::update(BeepCommData&) {}

#endif
