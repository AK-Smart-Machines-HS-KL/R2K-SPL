/**
 * @file BeepRecognizer.cpp
 *
 * This file implements a module that identifies the sound of a beep by
 * correlating with a number of templates.
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

  // Adapt number of channels to audio data.
  buffers.resize(theAudioData.channels);
  for(auto& buffer : buffers)
    buffer.reserve(bufferSize);

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

  // Correlate all channels with all signatures or only one if selectedName matches a whistle.
  if(buffers[firstBuffer].full() && samplesRequired <= 0)
  {
    int defects = 0; // Number of defect channels
    for(size_t i = 1; i < buffers.size(); ++i){
      if(theDamageConfigurationHead.audioChannelsDefect[i] || !buffers[i].full()) {
        ++defects;
        continue;
      }
      auto data = decode(buffers[i]);
    }

    
    // Reset Samples Required 
    samplesRequired = static_cast<unsigned>(bufferSize * newSampleRatio);
  }
}

std::vector<long> BeepRecognizer::decode(const RingBuffer<AudioData::Sample>& buffer)
{
  // Create new empty data vector
  std::vector<long> data(numBands, 0);

  // Compute volume of samples.
  float volume = 0;
  for(AudioData::Sample sample : buffer)
    volume = std::max(volume, std::abs(static_cast<float>(sample)));

  // Abort if not loud enough.
  if(volume == 0 || volume < (std::is_same<AudioData::Sample, short>::value ? std::numeric_limits<short>::max() : 1) * minVolume)
    return data;

  // Copy samples to FFTW input and normalize them.
  const double factor = 1.0 / volume;
  for(size_t i = 0; i < buffer.size(); ++i)
    samples[i] = buffer[i];

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
      if (amplitudes[bucket] > signalBaseline) {
        data[band] = data[band] | 1 << bit;
      }
    }
  }

  // Note: I cannot get this to work I have tried so many things - Andy
  COMPLEX_IMAGE("module:BeepRecognizer")
  {
    Image<PixelTypes::RGBPixel> image;

    image.setResolution(spectrumSize, spectrumSize / 2);
    memset(image[0], 0, image.height * image.width * sizeof(PixelTypes::RGBPixel));
    
    SEND_DEBUG_IMAGE("module:BeepRecognizer", image);
  }

  return data;
}

double getAmplitude(fftw_complex& value) {
  const Vector2d complex(value[0], value[1]);
  return complex.norm();
}
