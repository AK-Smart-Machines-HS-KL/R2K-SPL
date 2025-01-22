#include "WhistleRecognizer.h"
#include "Platform/SystemCall.h"
#include "Platform/Thread.h"
#include "Tools/Debugging/Annotation.h"
#include "Tools/Debugging/DebugDrawings.h"
#include "Tools/Math/BHMath.h"
#include <algorithm>
#include <limits>
#include <type_traits>
#include <cstring>

MAKE_MODULE(WhistleRecognizer, modeling);

static DECLARE_SYNC;

WhistleRecognizer::WhistleRecognizer()
{
  canvas.setResolution(bufferSize + 1, bufferSize * 2 / 3);

  // Load whistle signatures
  for (const std::string& fileName : whistles)
  {
    InBinaryFile stream("Whistles/" + fileName + ".dat");
    ASSERT(stream.exists());
    signatures.emplace_back();
    stream >> signatures.back();
  }

  // Allocate memory for FFTW plans
  samples = fftw_alloc_real(bufferSize * 2);
  std::memset(samples, 0, sizeof(double) * bufferSize * 2);
  spectrum = fftw_alloc_complex(bufferSize + 1);
  correlation = fftw_alloc_real(bufferSize * 2);

  // Create FFTW plans
  SYNC;
  fft = fftw_plan_dft_r2c_1d(bufferSize * 2, samples, spectrum, FFTW_MEASURE);
  ifft = fftw_plan_dft_c2r_1d(bufferSize * 2, spectrum, correlation, FFTW_MEASURE);

  // Dimitri: Hier ist es nicht sicher ob die FFTW_Plans erstellt werden deswegen hier eine Abfrage um sicher zu gehen das dies geschieht
  if (!fft || !ifft)
  {
    OUTPUT_TEXT("Failed to create FFTW plans.");
  }
}

WhistleRecognizer::~WhistleRecognizer()
{
  SYNC;
  fftw_destroy_plan(ifft);
  fftw_destroy_plan(fft);
  fftw_free(correlation);
  fftw_free(spectrum);
  fftw_free(samples);
}

void WhistleRecognizer::update(Whistle& theWhistle)
{
  DECLARE_PLOT("module:WhistleRecognizer:correlation0");
  DECLARE_PLOT("module:WhistleRecognizer:correlation1");
  DECLARE_PLOT("module:WhistleRecognizer:correlation2");
  DECLARE_PLOT("module:WhistleRecognizer:correlation3");
  DECLARE_PLOT("module:WhistleRecognizer:correlation4");
  DECLARE_PLOT("module:WhistleRecognizer:correlation5");
  DECLARE_PLOT("module:WhistleRecognizer:samples0");
  DECLARE_PLOT("module:WhistleRecognizer:samples1");
  DECLARE_PLOT("module:WhistleRecognizer:samples2");
  DECLARE_PLOT("module:WhistleRecognizer:samples3");

  // Remember if sound was playing during this team communication cycle
  soundWasPlaying = false;

  // Empty buffers when entering a state where it should be recorded.
  const bool shouldRecord = (theGameInfo.state == STATE_SET || theGameInfo.state == STATE_PLAYING) && !soundWasPlaying;
  if (!hasRecorded && shouldRecord)
    buffers.clear();
  hasRecorded = shouldRecord;

  // Adapt number of channels to audio data.
  buffers.resize(theAudioData.channels);
  for (auto& buffer : buffers)
    buffer.reserve(bufferSize);

  // Append current samples to buffers and sample down if necessary
  ASSERT(theAudioData.sampleRate % sampleRate == 0);
  const size_t stepSize = theAudioData.sampleRate / sampleRate * theAudioData.channels;
  for (; sampleIndex < theAudioData.samples.size(); sampleIndex += stepSize)
  {
    --samplesRequired;
    for (size_t channel = 0; channel < theAudioData.channels; ++channel)
      buffers[channel].push_front(theAudioData.samples[sampleIndex + channel]);
  }
  sampleIndex -= theAudioData.samples.size();

  // Compute first channel index to access damage configuration.
  const int firstBuffer = theDamageConfigurationHead.audioChannelsDefect[0] ? 1 : 0;

  // No whistles can be detected while sound is playing.
  if (soundWasPlaying)
    theWhistle.channelsUsedForWhistleDetection = 0;

  // Count number of channels if they were set to zero and no sound is playing.
  if (!theWhistle.channelsUsedForWhistleDetection && !soundWasPlaying)
    for (size_t i = 0; i < buffers.size(); ++i)
      if (!theDamageConfigurationHead.audioChannelsDefect[i])
        ++theWhistle.channelsUsedForWhistleDetection;

  std::string selectedName = "newWhistle";
  MODIFY("module:WhistleRecognizer:select", selectedName);
  auto selectedIter = std::find_if(signatures.begin(), signatures.end(),
    [&selectedName](const Signature& signature) {return signature.name == selectedName; });

  // Record a whistle.
  DEBUG_RESPONSE_ONCE("module:WhistleRecognizer:record");
  {
    if (buffers[firstBuffer].full())
    {
      Signature signature;
      signature.selfCorrelation = correlate(signature.spectrum, buffers[firstBuffer], true);
      if (signature.selfCorrelation > 0)
      {
        signature.name = selectedName;
        if (selectedIter == signatures.end())
        {
          signatures.emplace_back();
          selectedIter = signatures.end() - 1;
        }
        *selectedIter = signature;
        OutBinaryFile stream("Whistles/" + selectedName + ".dat");
        if (stream.exists())
        {
          stream << *selectedIter;
          OUTPUT_TEXT("Recorded whistle " << selectedName << " with selfCorrelation = " << signature.selfCorrelation);
        }
      }
    }
  }

  for (size_t i = 0; i < buffers.size(); ++i)
    if (!buffers[i].empty())
      switch (i)
      {
      case 0: PLOT("module:WhistleRecognizer:samples0", buffers[i].back()); break;
      case 1: PLOT("module:WhistleRecognizer:samples1", buffers[i].back()); break;
      case 2: PLOT("module:WhistleRecognizer:samples2", buffers[i].back()); break;
      case 3: PLOT("module:WhistleRecognizer:samples3", buffers[i].back()); break;
      }

  // Correlate all channels with all signatures or only one if selectedName matches a whistle.
  if (shouldRecord && buffers[firstBuffer].full() && samplesRequired <= 0)
  {
    COMPLEX_IMAGE("module:WhistleRecognizer:spectra")
    {
      std::memset(canvas[0], 128, sizeof(PixelTypes::Edge2Pixel) * canvas.width * canvas.height);
      if (selectedIter != signatures.end())
        for (unsigned x = 0; x < selectedIter->spectrum.size(); ++x)
        {
          const Vector2d& complex = selectedIter->spectrum[x];
          const unsigned amplitude = std::min(static_cast<unsigned>(complex.norm()), canvas.height);
          if (amplitude > 0)
          {
            const PixelTypes::Edge2Pixel pixel(static_cast<char>(128 + 127 * complex.x() / amplitude),
              static_cast<char>(128 + 127 * complex.y() / amplitude));
            for (size_t y = 0; y < amplitude; ++y)
              canvas[y][x] = pixel;
          }
        }
    }

    const Signature* bestSignature = nullptr;

    for (auto& signature : signatures)
      if (selectedIter == signatures.end() || &signature == &*selectedIter)
      {
        size_t defects = 0;
        float correlation = 0.f;
        float bestChannelCorrelation = 0.f;

        for (size_t i = 0; i < buffers.size(); ++i)
          if (theDamageConfigurationHead.audioChannelsDefect[i] || !buffers[i].full())
            ++defects;
          else
          {
            const float channelCorrelation = correlate(signature.spectrum, buffers[i]);
            bestChannelCorrelation = std::max(bestChannelCorrelation, channelCorrelation);
            correlation += channelCorrelation;
          }

        if (defects < buffers.size())
        {
          correlation /= static_cast<float>(buffers.size() - defects) * signature.selfCorrelation * minCorrelation;
          if (correlation >= bestCorrelation)
          {
            theWhistle.confidenceOfLastWhistleDetection = correlation;
            theWhistle.channelsUsedForWhistleDetection = static_cast<unsigned char>(buffers.size() - defects);
            bestCorrelation = correlation;
            signature.timestamp = theFrameInfo.time;
            bestSignature = &signature;
          }

          switch (&signature - signatures.data())
          {
          case 0: PLOT("module:WhistleRecognizer:correlation0", correlation); break;
          case 1: PLOT("module:WhistleRecognizer:correlation1", correlation); break;
          case 2: PLOT("module:WhistleRecognizer:correlation2", correlation); break;
          case 3: PLOT("module:WhistleRecognizer:correlation3", correlation); break;
          case 4: PLOT("module:WhistleRecognizer:correlation4", correlation); break;
          default: PLOT("module:WhistleRecognizer:correlation5", correlation); break;
          }
        }
      }

    if (bestSignature)
    {
      if (theFrameInfo.getTimeSince(theWhistle.lastTimeWhistleDetected) > minAnnotationDelay)
        ANNOTATION("WhistleRecognizer", bestSignature->name << " with " << static_cast<int>(bestCorrelation * 100.f) << "%");
      theWhistle.lastTimeWhistleDetected = theFrameInfo.time;
      bestSignatures.push_back(*bestSignature);
    }

    samplesRequired = static_cast<unsigned>(bufferSize);
  }

  // Reset best correlation after it was sent in two network packets.
  if (theFrameInfo.getTimeSince(theWhistle.lastTimeWhistleDetected) >= accumulationDuration)
  {
    theWhistle.lastTimeWhistleDetected = theWhistle.lastTimeWhistleDetected;
    bestCorrelation = 1.f;
    soundWasPlaying = SystemCall::soundIsPlaying();
  }

  DEBUG_RESPONSE_ONCE("module:WhistleRecognizer:detectNow");
  {
    theWhistle.lastTimeWhistleDetected = theFrameInfo.time;
    theWhistle.confidenceOfLastWhistleDetection = 2.f;
  }

  SEND_DEBUG_IMAGE("module:WhistleRecognizer:spectra", canvas, PixelTypes::Edge2);

  // Analyze whistle events when the state changes to PLAYING
  if (theGameInfo.state == STATE_PLAYING)
  {
    const Signature* bestSigTime = nullptr;
    float bestSigCorrelation = 0.f;
    unsigned bestTimeDifference = std::numeric_limits<unsigned>::max(); // Initialize with a large value

    // Check if bestSignatures is not empty
    if (!bestSignatures.empty())
    {
      // Assign the first element of bestSignatures to bestSigTime
      bestSigTime = &bestSignatures[0];
    }

    for (const auto& bestSig : bestSignatures)
    {
      unsigned timeDifference = std::abs(static_cast<int>(theWhistle.lastTimeWhistleDetected - bestSig.timestamp));
      if (timeDifference < bestTimeDifference || (timeDifference == bestTimeDifference && bestSig.selfCorrelation > bestSigCorrelation))
      {
        bestSigCorrelation = bestSig.selfCorrelation;
        bestSigTime = &bestSig;
      }
    }

    if (bestSigTime)
    {
      OutBinaryFile stream("Whistles/TrueWhistle.dat");
      if (stream.exists())
      {
        stream << *bestSigTime;
        OUTPUT_TEXT("TrueWhistle detected at " << bestSigTime->timestamp << " seconds of timedifference with correlation " << bestSigTime->selfCorrelation);
      }
    }

    if (theFrameInfo.getTimeSince(theWhistle.lastTimeWhistleDetected) >= accumulationDuration)
    {
      bestSigTime = nullptr;
      bestSigCorrelation = 0.f;
    }
  }
}

float WhistleRecognizer::correlate(std::vector<Vector2d>& signature, const RingBuffer<AudioData::Sample>& buffer, bool record)
{
  // Compute volume of samples.
  float volume = 0;
  // Increased correlation threshold
  const float minCorrelationThreshold = 0.7f;

  for (AudioData::Sample sample : buffer)
    volume = std::max(volume, std::abs(static_cast<float>(sample)));

  // Abort if not loud enough.
  const float minVolumeThreshold = 0.5f; // Increased volume threshold
  if (volume == 0 || (!record && volume < (std::is_same<AudioData::Sample, short>::value ? std::numeric_limits<short>::max() : 1) * minVolumeThreshold))
    return 0.f;

  // Apply a simple low-pass filter to reduce high-frequency noise
  const float alpha = 0.2f; // Smoothing factor (adjustable)
  std::vector<AudioData::Sample> filteredSamples(buffer.size());
  filteredSamples[0] = buffer[0];
  for (size_t i = 1; i < buffer.size(); ++i)
    filteredSamples[i] = alpha * buffer[i] + (1 - alpha) * filteredSamples[i - 1];

  // Copy filtered samples to FFTW input and normalize them.
  const double factor = 1.0 / volume;
  for (size_t i = 0; i < buffer.size(); ++i)
    samples[i] = filteredSamples[i] * factor;

  // samples -> spectrum
  fftw_execute(fft);

  COMPLEX_IMAGE("module:WhistleRecognizer:spectra");
  {
    for (unsigned x = 0; x < signature.size(); ++x)
    {
      const Vector2d complex(spectrum[x][0], spectrum[x][1]);
      const unsigned amplitude = std::min(static_cast<unsigned>(complex.norm()), canvas.height);
      if (amplitude > 0)
      {
        const PixelTypes::Edge2Pixel pixel(static_cast<char>(128 + 127 * complex.x() / amplitude),
          static_cast<char>(128 + 127 * complex.y() / amplitude));
        for (size_t y = 0; y < amplitude; ++y)
          canvas[canvas.height - 1 - y][x] = pixel;
      }
    }
  }

  if (record)
  {
    // Store conjugate spectrum as signature and self-correlate input.
    signature.resize(bufferSize + 1);
    for (size_t i = 0; i < signature.size(); ++i)
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
    for (size_t i = 0; i < signature.size(); ++i)
    {
      const double spectrumi0 = spectrum[i][0];
      spectrum[i][0] = spectrumi0 * signature[i][0] - spectrum[i][1] * signature[i][1];
      spectrum[i][1] = spectrum[i][1] * signature[i][0] + spectrumi0 * signature[i][1];
    }
  }

  COMPLEX_IMAGE("module:WhistleRecognizer:spectra")
  {
    for (unsigned x = 0; x < signature.size(); ++x)
    {
      const Vector2d complex(spectrum[x][0], spectrum[x][1]);
      const unsigned amplitude = std::min(static_cast<unsigned>(std::sqrt(complex.norm())), canvas.height);
      if (amplitude > 0)
      {
        const PixelTypes::Edge2Pixel pixel(static_cast<char>(128 + 127 * complex.x() / amplitude),
          static_cast<char>(128 + 127 * complex.y() / amplitude));
        for (size_t y = 0; y < amplitude; ++y)
          canvas[(canvas.height - amplitude) / 2 + y][x] = pixel;
      }
    }
  }

  // spectrum -> correlation
  fftw_execute(ifft);

  // Find best correlation.
  double bestCorrelation = 0;
  for (size_t i = 0; i < bufferSize * 2; ++i)
    if (std::abs(correlation[i]) > bestCorrelation)
      bestCorrelation = std::abs(correlation[i]);

  return (std::sqrt(bestCorrelation) / bufferSize / 2) > minCorrelationThreshold ? static_cast<float>(std::sqrt(bestCorrelation) / bufferSize / 2) : 0.f;
}
