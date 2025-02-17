/**
 * @file BeepBroadcaster.cpp
 * This file implements a module that provides the sending part of the audio communication system.
 * @author Nicolas Fortune, Andy Hobelsberger (Winter 2022/23), Sandro Kloos (Winter 2024/25)
 */

#include "BeepBroadcaster.h"
#include "Platform/SystemCall.h"
#include "Platform/Thread.h"
#include <mutex>
#include <condition_variable>
#include <type_traits>
#include <algorithm>
#include <sstream>
#include <iostream>


#define SAMPLE_RATE 44100
#define BUFFER_SIZE SAMPLE_RATE
#define VOLUME_MULTIPLIER 16000

MAKE_MODULE(BeepBroadcaster, communication);


#ifdef TARGET_ROBOT

#ifdef TARGET_ROBOT
  snd_pcm_t* pcm_handle;
#endif

typedef short sample_t;

BeepBroadcaster::BeepBroadcaster()
{
  init_pcm();
  startWorker();
}

BeepBroadcaster::~BeepBroadcaster()
{
    stopWorker();
    snd_pcm_close(pcm_handle);
}

void BeepBroadcaster::update(BeepCommData& beepCommData)
{
  //demo
  if (theFrameInfo.getTimeSince(lastTimeWithBeep) >= maxTimeOutBetweenBeeps){
  lastTimeWithBeep = theFrameInfo.time;
  if (responseToggle) {
    responseToggle = false;
    for (int i=0;i<=numBands;i++){
      //skips the robots own message 
      if (theRobotInfo.number-1!=i) {
      if(theBeep.messages[i] >= 10){      
        //Robot specific
        //11-14 offset for 10 broadcast messages at 4 messages per robot
        for (int j=1;j<=numBands;j++){
          if (theRobotInfo.number==j){
          if (theBeep.messages[i]==(j-1)*4+11){
            if(theRobotInfo.number==numBands){ 
              beepCommData.broadcastQueue.push_back((1-1)*4+11);
            }else{
              beepCommData.broadcastQueue.push_back((theRobotInfo.number)*4+11);  
            }
          }
          if (theBeep.messages[i]==(j-1)*4+12){

          }
          if (theBeep.messages[i]==(j-1)*4+13){

          }
          if (theBeep.messages[i]==(j-1)*4+14){
          }

          }
        }
      }  
    }

    }
  }else {
  responseToggle = true;
}
  }

    //Handle Ground Sensor
    if(!theGroundContactState.contact)
    {
        if (sensorToggle){
            sensorToggle = false;
            groundBeep(beepCommData, 1);
            OUTPUT_TEXT("High Beep!");
        }
    } else {
        sensorToggle = true;
    }

    // Handle Head Button
    if (theEnhancedKeyStates.isPressedFor(KeyStates::headFront, 100u))
    {
        if (buttonToggle)   
        {
            buttonToggle = false; 
            beepCommData.broadcastQueue.push_back(headButtonMessage);
            OUTPUT_TEXT("Beep: " << headButtonMessage);
        } 
    } else {
        buttonToggle = true;
    }
    
    if(sendToggle){
      beepCommData.broadcastQueue.push_back(sendmessage);  
      bool sendToggle=false;
    } 

    DEBUG_RESPONSE_ONCE("module:BeepComms:broadcast:1") 
        requestMessageBroadcast(1000, 0.5, 1);

    // The Recognized Beeps console output for debugging
  
    for(int e:theBeep.messages){   
    if(e>0){    
    std::stringstream ss;
    copy( theBeep.messages.begin(), theBeep.messages.end(), std::ostream_iterator<int>(ss, " "));
    std::string s = ss.str();
    s = s.substr(0, s.length()-1);
              OUTPUT_TEXT("The Beeps: " << s); 

    }
    }
    // Handle Response to 1
/*
    if (theRobotInfo.number != 1)
    {
        if (theBeep.messages[0] > 0) {
            if (responseToggle) 
            {
                responseToggle = false;
                beepCommData.broadcastQueue.push_back(1);
            }
            
        } else {
            responseToggle = true;
        }
        
    }
*/
    while (!beepCommData.broadcastQueue.empty())
    {
        int message = beepCommData.broadcastQueue.back();
        beepCommData.broadcastQueue.pop_back();
        requestMessageBroadcast(1000, 0.5, message);
    }
}

//void BeepBroadcaster::requestBeep(int message){
//    beepCommData.broadcastQueue.push_back(message);

/*
//5bit
{
//robotnumber = 0 ->broadcast
//10 message broadcast
//4 message per robot

if (robot_number==0){
    int msg = message;
    requestMessageBroadcast(1, 1.0f, msg);
}else
{
    int msg =(((robot_number-1)*4)+10+message);
    requestMessageBroadcast(1, 1.0f, msg);
}
sendToggle=true;*/
//}

/*
//4bit
{
//robotnumber = 0 ->broadcast
//5 message broadcast
//2 message per robot

if (robot_number==0){
    sendmessage=message;
}else
{
    sendmessage=(((robot_number-1)*2)+5+message);
}
sendToggle=true;
}

*/

//play sine waves simultaneously
void BeepBroadcaster::requestMultipleFrequencies(float duration, float volume, std::vector<float> frequencies){
   BeepRequest request;
   request.duration = duration;
   request.volume = volume;
   request.frequencies = frequencies;
   std::lock_guard lock(mtx); // aquire lock to write to request queue
   requestQueue.push_back(request); // qrite request to queue
   workerSignal.notify_one(); // notify worker of new request
}

void BeepBroadcaster::requestMessageBroadcast(float duration, float volume, int message){
    float ownBaseFrequency = baseFrequency + (theRobotInfo.number - 1) * bandWidth;
    std::vector<float> frequencies;
    for (size_t bit = 0; bit < encodedBits; bit++)
    {
        if ((message & (1 << bit)) != 0) {
            frequencies.push_back(ownBaseFrequency + bit * (bandWidth / encodedBits));
            OUTPUT_TEXT("Robot nr. " << theRobotInfo.number << " broadcasting message " << message << "(" << frequencies.back() << "Hz)");
        }
    }
    requestMultipleFrequencies(1000, 0.5, frequencies);
};

void BeepBroadcaster::startWorker()
{
    workerThread = std::thread(&BeepBroadcaster::handleBeepRequests, this);
}

void BeepBroadcaster::init_pcm()
{
    //open the audio device
    snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);

    snd_pcm_hw_params_t *hw_params;
    snd_pcm_hw_params_alloca(&hw_params);

    snd_pcm_hw_params_any(pcm_handle, hw_params);
    snd_pcm_hw_params_set_buffer_size(pcm_handle, hw_params, 1152);
    snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S32_LE);
    snd_pcm_hw_params_set_channels(pcm_handle, hw_params, 1);
    snd_pcm_hw_params_set_rate(pcm_handle, hw_params, SAMPLE_RATE, 0);
    snd_pcm_hw_params_set_periods(pcm_handle, hw_params, 10, 0);
    snd_pcm_hw_params_set_period_time(pcm_handle, hw_params, 100000, 0); // 0.1 seconds

    //snd_pcm_hw_params(pcm_handle, hw_params);
    snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, SAMPLE_RATE, 1, 500000);

}

void BeepBroadcaster::stopWorker()
{
    shutdown = true; // tell worker thread to exit
    workerSignal.notify_all(); // Notify worker that something happened (if they are waiting)
    workerThread.join(); // wait for function to exit (should happen quickly)
}

void BeepBroadcaster::handleBeepRequests()
{
    std::unique_lock lock(mtx); // aquire mutex lock
    while (true)
    {
        workerSignal.wait(lock); // release lock and wait for signal from main thread
        if (shutdown) break; // exit loop. lock auto releases when function exits
        BeepRequest request = requestQueue.front();
        requestQueue.pop_front();
        
        //generate superimposed sine waves as signals
        sample_t buf[BUFFER_SIZE] = {0}; // signal buffer
        int signalSize = (int) (SAMPLE_RATE * request.duration / 1000); // total size of signal in samples
        
        do {
            int toGenerate = std::min(signalSize, BUFFER_SIZE);
            for (int i = 0; i < toGenerate; i++) {
                buf[i] = 0;
                for (float& frequency : request.frequencies) {
                    buf[i] += (sample_t) (VOLUME_MULTIPLIER * (request.volume * sin(frequency * M_PI * 2 * i / SAMPLE_RATE)));
                }
            }
            signalSize -= toGenerate;
            
            snd_pcm_sframes_t n = snd_pcm_writei(pcm_handle, &buf, toGenerate); // write data to audio buffer
            if (n < 0) {
                printf("Audio Stream Lost (%s) Recovering...\n", snd_strerror(int(n)));
                snd_pcm_recover(pcm_handle, int(n), 1);
                n = snd_pcm_writei(pcm_handle, &buf, toGenerate); // write data to audio buffer
            }
        } while (signalSize > 0 && !shutdown); // exit when signal is done OR shutdown is requested
    }
}

void BeepBroadcaster::groundBeep(BeepCommData& beepCommData, int message){
    beepCommData.broadcastQueue.push_back(message);
}

#else // !defined TARGET_ROBOT

BeepBroadcaster::BeepBroadcaster() {}
BeepBroadcaster::~BeepBroadcaster() {}
void BeepBroadcaster::update(BeepCommData&) {}


#endif
