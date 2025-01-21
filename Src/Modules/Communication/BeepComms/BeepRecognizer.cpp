/**
 * @file BeepRecognizer.cpp
 *
 * This file implements a module that identifies the sound of a beep
 *
 * @author Wilhelm Simus
 */

#include "BeepRecognizer.h"
#include "Platform/SystemCall.h"
#include "Platform/Thread.h"
#include "Tools/Debugging/Annotation.h"
#include "Tools/Debugging/DebugDrawings.h"
#include "Tools/Math/BHMath.h"
#include <algorithm>
#include <limits>
#include <type_traits>

MAKE_MODULE(BeepRecognizer, communication);

static DECLARE_SYNC;

BeepRecognizer::BeepRecognizer()
{
  averagingBuffers = std::vector<RingBuffer<long>> (numBands, RingBuffer<long>(averagingBufferSize));
  averageActivations = std::vector<std::vector<int>> (numBands, std::vector<int>(encodedBits, 0));

  signalWidth = bandWidth / encodedBits;

  canvas.setResolution(bufferSize + 1, bufferSize * 2 / 3);

  samplesSize = bufferSize * 2;
  spectrumSize = bufferSize + 1;

  // Allocate memory for FFTW plans
  samples = fftw_alloc_real(samplesSize);
  std::memset(samples, 0, sizeof(double) * bufferSize * 2);
  spectrum = fftw_alloc_complex(spectrumSize);

  // Create FFTW plan
  SYNC;
  fft = fftw_plan_dft_r2c_1d(bufferSize * 2, samples, spectrum, FFTW_MEASURE);
}

BeepRecognizer::~BeepRecognizer()
{
  SYNC;
  fftw_destroy_plan(fft);
  fftw_free(samples);
  fftw_free(spectrum);
}

void BeepRecognizer::update(Beep &theBeep){
  if (theBeep.messages.size() != numBands) {
    theBeep.messages.resize(numBands);
  }

  // Adapt number of channels to audio data.
  buffers.resize(theAudioData.channels);
  for(auto& buffer : buffers){
    buffer.reserve(bufferSize);
  }

  // Append current samples to buffers and sample down if necessary
  // Sample down is necessary only if sample rate is different between audio provider and this module
  ASSERT(theAudioData.sampleRate % sampleRate == 0);
  const size_t stepSize = theAudioData.sampleRate / sampleRate * theAudioData.channels;
  for(; sampleIndex < theAudioData.samples.size(); sampleIndex += stepSize)
  {
    --samplesRequired;
    // De-Weave Samples into sepearate buffers 
    for(size_t channel = 0; channel < theAudioData.channels; ++channel)
      buffers[channel].push_front(theAudioData.samples[sampleIndex + channel]);
  }
  sampleIndex -= theAudioData.samples.size();

  // Compute first channel index to access damage configuration.
  const int firstBuffer = theDamageConfigurationHead.audioChannelsDefect[0] ? 1 : 0;

  if(buffers[firstBuffer].full() && samplesRequired <= 0)
  {

    // Adjust averages for oldest data that is about to be removed
    if (averagingBuffers[0].full())
    {
      for (size_t band = 0; band < numBands; band++)
      {
        for (size_t bit = 0; bit < encodedBits; bit++)
        {
          if ((averagingBuffers[band].back() & (1 << bit)) > 0)
          {
            averageActivations[band][bit]--;
          }
        }
      }
    }    

    // decode audio data
    for(size_t channel = firstBuffer; channel < buffers.size(); ++channel){

      // if the channel is defect, skip it
      if(theDamageConfigurationHead.audioChannelsDefect[channel] || !buffers[channel].full()) {
        continue;
      }

      // Decode non-defect channel
      std::vector<long> data = decode(buffers[channel]);
      for (size_t band = 0; band < data.size(); band++)
      {
        averagingBuffers[band].push_front(data[band]);
      }
      break; // only decode first channel that is available
    }

    // Adjust averages for new data
    for (size_t band = 0; band < numBands; band++)
    {
      theBeep.messages[band] = 0;
      for (size_t bit = 0; bit < encodedBits; bit++)
      {
        if ((averagingBuffers[band].front() & (1 << bit)) > 0)
        {
          averageActivations[band][bit]++;
        }
        if (averageActivations[band][bit] >= averagingThreshold)
        {
          theBeep.messages[band] |= (1 << bit);
        }
      }
    }

    // Reset Samples Required 
    samplesRequired = static_cast<unsigned>(bufferSize * newSampleRatio);
  }

  // Note: I cannot get this to work I have tried so many things - Andy
  // The idea is to draw the spectrum like the Whistle Recognizer does (which is prettyy unreliable itself)
  // COMPLEX_IMAGE("module:BeepRecognizer")
  // {
  //   Image<PixelTypes::YUYVPixel> image;

  //   image.setResolution(spectrumSize, spectrumSize / 2);
  //   memset(image[0], 0, image.height * image.width * sizeof(PixelTypes::RGBPixel));
  //   for (size_t y = 0; y < image.height; y++)
  //   {
  //     for (size_t x = 0; x < image.width; x++)
  //     {
  //       image[y][x].color = 1; 
  //     }
  //   }
  //   SEND_DEBUG_IMAGE("module:BeepRecognizer", image);
  // }

     //Kurze say ausgabe zum testen
    if (theBeep.messages[2] > 1) {
            if (responseToggle) 
            {
                responseToggle = false;
                SystemCall::say("Something Robot 3");
            }
            
        } else if (theBeep.messages[4] > 0){
            if (responseToggle) 
            {
                responseToggle = false;
                SystemCall::say("Something Robot 5");
            }
        } else if (theBeep.messages[0] == 15) {
            if (responseToggle) 
            {
                responseToggle = false;
                SystemCall::say("Message 15 Robot 1");
            }
            
        } else if (theBeep.messages[0] == 10) {
            if (responseToggle) 
            {
                responseToggle = false;
                SystemCall::say("Message 10 Robot 1");
            }
            
        } else {
            responseToggle = true;
        }



}

std::vector<long> BeepRecognizer::decode(const RingBuffer<AudioData::Sample>& buffer)
{
  // Create new empty data vector
  std::vector<long> data(numBands, 0);

  // Compute volume of samples.
  float volume = 0;
  for(AudioData::Sample sample : buffer){
    volume = std::max(volume, std::abs(static_cast<float>(sample)));
  }

  // OUTPUT_TEXT("Volume: " << volume);
  // Abort if not loud enough.
  if(volume == 0 || volume < (std::is_same<AudioData::Sample, short>::value ? std::numeric_limits<short>::max() : 1) * minVolume){
    return data;
  }

  // Copy samples to FFTW input and normalize them.
  const double factor = 1.0 / volume;
  for(size_t i = 0; i < buffer.size(); ++i){
    samples[i] = buffer[i];
  }

  // samples -> spectrum
  fftw_execute(fft);

  // Calculate Amplitudes of FFT Spectrum
  std::vector<double> amplitudes(spectrumSize);
  for (size_t i = 0; i < amplitudes.size(); i++)
  {
    amplitudes[i] = getAmplitude(spectrum[i]);
  }

  // Decode Data from audio spectrum to data vector
  for (size_t band = 0; band < numBands; band++)
  {

    for (size_t bit = 0; bit < encodedBits; bit++)
    {
      // Calculate in which bucket a specific bit would be
      float frequencyOffset = (band * encodedBits + bit) * signalWidth;
      float targetFrequency = baseFrequency + frequencyOffset;
      int bucket = targetFrequency * samplesSize / sampleRate;

      // Check if bucket is above this peak 
      if (getAmplitude(spectrum[bucket]) > signalBaseline) {
        data[band] = data[band] | 1 << bit;
      }
    }
  }

  return data;
}

double getAmplitude(fftw_complex& value) {
  return std::sqrt(std::pow(value[0], 2)  + std::pow(value[1], 2));
}

double getPhase(fftw_complex& value) {
  return std::atan2(value[0], value[1]); // Phase = direction of complex number
}
