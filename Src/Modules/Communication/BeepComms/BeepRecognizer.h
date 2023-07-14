/**
 * @file BeepRecognizer.h
 *
 * This file declares a module that identifies the sound of a beep by
 * correlating with a number of frequencies.
 *
 * @author Wilhelm Simus
 */

#pragma once

#include "Representations/Communication/Beep.h"
#include "Representations/Communication/BHumanMessage.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Configuration/DamageConfiguration.h"
#include "Representations/Infrastructure/AudioData.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Modeling/Whistle.h"
#include "Tools/Debugging/DebugImages.h"
#include "Tools/Module/Module.h"
#include "Tools/RingBuffer.h"
#include "Tools/Streams/Eigen.h"
#include <fftw3.h>

MODULE(BeepRecognizer,
{,
  USES(GameInfo),
  REQUIRES(AudioData),
  REQUIRES(BHumanMessageOutputGenerator),
  REQUIRES(DamageConfigurationHead),
  REQUIRES(FrameInfo),
  REQUIRES(RawGameInfo),
  REQUIRES(RobotInfo),
  PROVIDES(Beep),
  LOADS_PARAMETERS(
  {,
    (unsigned) bufferSize, /**< The number of samples buffered per channel. */
    (unsigned) sampleRate, /**< The sample rate actually used. */
    (float) newSampleRatio, /**< The ratio of new samples buffered before recognition is tried again (0..1). */
    (float) minVolume, /**< The minimum volume that must be reached for accepting a whistle [0..1). */

    (int) numBands,
    (int) encodedBits,
    (float) signalBaseline,
    (float) baseFrequency,
    (float) bandWidth,
    (int) averagingBufferSize,
    (int) averagingThreshold,
  }),
});

class BeepRecognizer : public BeepRecognizerBase
{
  float signalWidth;
  std::vector<RingBuffer<long>> averagingBuffers;
  std::vector<std::vector<int>> averageActivations; 
  size_t spectrumSize;
  size_t samplesSize;
  std::vector<RingBuffer<AudioData::Sample>> buffers; /**< Sample buffers for all channels. */
  int samplesRequired = 0; /** The number of new samples required. */
  size_t sampleIndex = 0; /** Index of next sample to process for subsampling. */
  double* samples; /**< The samples after normalization. */
  fftw_complex* spectrum; /**< The spectrum of the samples. */
  fftw_plan fft; /**< The plan to compute the FFT. */
  Image<PixelTypes::Edge2Pixel> canvas; /**< Canvas for drawing spectra. */

  /**
   * This method is called when the representation provided needs to be updated.
   * @param theBeep The representation updated.
   */
  void update(Beep& theBeep) override;

  /**
   * Correlate the samples recorded with a signature spectrum.
   * @param buffer The samples recorded.
   * @return The correlation between signature and buffer. 0 if the volume was too low.
   */
  std::vector<long> decode(const RingBuffer<AudioData::Sample>& buffer);

public:
  BeepRecognizer();
  ~BeepRecognizer();
};

double getAmplitude(fftw_complex& value);
