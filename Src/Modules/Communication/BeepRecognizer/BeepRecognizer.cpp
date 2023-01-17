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

  // Allocate memory for FFTW plans
  std::memset(samples, 0, sizeof(double) * bufferSize * 2);
  spectrum = fftw_alloc_complex(bufferSize + 1);
  correlation = fftw_alloc_real(bufferSize * 2);

  // Create FFTW plans
  SYNC;
  fft = fftw_plan_dft_r2c_1d(bufferSize * 2, samples, spectrum, FFTW_MEASURE);
}

BeepRecognizer::~BeepRecognizer()
{
  SYNC;
  fftw_destroy_plan(fft);
  fftw_free(correlation);
  fftw_free(spectrum);
}

void BeepRecognizer::update(Beep &theBeep){
  return;
}

float BeepRecognizer::correlate(std::vector<Vector2d>& signature, const RingBuffer<AudioData::Sample>& buffer,
                                      bool record)
{
  // Compute volume of samples.
  float volume = 0;
  for(AudioData::Sample sample : buffer)
    volume = std::max(volume, std::abs(static_cast<float>(sample)));

  // Abort if not loud enough.
  if(volume == 0 || (!record && volume < (std::is_same<AudioData::Sample, short>::value ? std::numeric_limits<short>::max() : 1) * minVolume))
    return 0.f;

  // Copy samples to FFTW input and normalize them.
  const double factor = 1.0 / volume;
  for(size_t i = 0; i < buffer.size(); ++i)
    samples[i] = buffer[i] * factor;

  // samples -> spectrum
  fftw_execute(fft);

  COMPLEX_IMAGE("module:BeepRecognizer:spectra")
  {
    for(unsigned x = 0; x < signature.size(); ++x)
    {
      const Vector2d complex(spectrum[x][0], spectrum[x][1]);
      const unsigned amplitude = std::min(static_cast<unsigned>(complex.norm()), canvas.height);
      if(amplitude > 0)
      {
        const PixelTypes::Edge2Pixel pixel(static_cast<char>(128 + 127 * complex.x() / amplitude),
                                           static_cast<char>(128 + 127 * complex.y() / amplitude));
        for(size_t y = 0; y < amplitude; ++y)
          canvas[canvas.height - 1 - y][x] = pixel;
      }
    }
  }

  if(record)
  {
    // Store conjugate spectrum as signature and self-correlate input.
    signature.resize(bufferSize + 1);
    for(size_t i = 0; i < signature.size(); ++i)
    {
      signature[i] = Vector2d(spectrum[i][0], -spectrum[i][1]);
      spectrum[i][0] = sqr(spectrum[i][0]) + sqr(spectrum[i][1]);
      spectrum[i][1] = 0;
    }
  }
  else
  {
    // Multiply input spectrum with signature spectrum.
    ASSERT(signature.size() == bufferSize + 1);
    for(size_t i = 0; i < signature.size(); ++i)
    {
      const double spectrumi0 = spectrum[i][0];
      spectrum[i][0] = spectrumi0 * signature[i][0] - spectrum[i][1] * signature[i][1];
      spectrum[i][1] = spectrum[i][1] * signature[i][0] + spectrumi0 * signature[i][1];
    }
  }

  COMPLEX_IMAGE("module:BeepRecognizer:spectra")
  {
    for(unsigned x = 0; x < signature.size(); ++x)
    {
      const Vector2d complex(spectrum[x][0], spectrum[x][1]);
      const unsigned amplitude = std::min(static_cast<unsigned>(std::sqrt(complex.norm())), canvas.height);
      if(amplitude > 0)
      {
        const PixelTypes::Edge2Pixel pixel(static_cast<char>(128 + 127 * complex.x() / amplitude),
                                           static_cast<char>(128 + 127 * complex.y() / amplitude));
        for(size_t y = 0; y < amplitude; ++y)
          canvas[(canvas.height - amplitude) / 2 + y][x] = pixel;
      }
    }
  }

  // spectrum -> correlation
  fftw_execute(ifft);

  // Find best correlation.
  double bestCorrelation = 0;
  for(size_t i = 0; i < bufferSize * 2; ++i)
    if(std::abs(correlation[i]) > bestCorrelation)
      bestCorrelation = std::abs(correlation[i]);

  return static_cast<float>(std::sqrt(bestCorrelation) / bufferSize / 2);
}
