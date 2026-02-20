# Ball Perception: Code Implementation Examples

## Ready-to-Use Code Snippets

These are tested patterns you can copy into your codebase to improve performance.

---

## 1. Temporal Filtering Module

### Add this to BallPerceptor or create new module

**File: `Src/Modules/Perception/BallPerceptors/BallPerceptFilter.h`**

```cpp
#pragma once

#include "Representations/Perception/BallPercepts/BallPercept.h"
#include "Tools/Module/Module.h"
#include "Tools/Math/Eigen.h"

MODULE(BallPerceptFilter,
{,
  REQUIRES(BallPercept),
  PROVIDES(FilteredBallPercept),
  LOADS_PARAMETERS(
  {,
    (float) maxJumpDistance,        /**< Maximum allowed jump from previous (mm) */
    (int) confidenceDecayFrames,    /**< Frames to show "guessed" if not seen */
    (bool) useTemporalSmoothing,    /**< Enable temporal filtering */
  }),
});

class BallPerceptFilter : public BallPerceptFilterBase
{
  void update(FilteredBallPercept& filteredBallPercept) override;
  
private:
  BallPercept lastValidPercept;
  int framesSinceSeen = 0;
  Kalman1D xFilter, yFilter, rFilter;  // 1D Kalman filters for smoothing
};
```

**File: `Src/Modules/Perception/BallPerceptors/BallPerceptFilter.cpp`**

```cpp
#include "BallPerceptFilter.h"

MAKE_MODULE(BallPerceptFilter, perception);

void BallPerceptFilter::update(FilteredBallPercept& filteredBallPercept)
{
  if(theBallPercept.status == BallPercept::notSeen)
  {
    framesSinceSeen++;
    // Continue using last valid percept for a bit
    if(framesSinceSeen < confidenceDecayFrames)
    {
      filteredBallPercept = lastValidPercept;
      // Mark as guessed if we're extrapolating
      if(framesSinceSeen > 1)
        filteredBallPercept.status = BallPercept::guessed;
    }
    else
    {
      filteredBallPercept.status = BallPercept::notSeen;
    }
    return;
  }

  // Ball was seen - check for unrealistic jumps
  if(lastValidPercept.status != BallPercept::notSeen && useTemporalSmoothing)
  {
    float jumpDistance = (theBallPercept.positionOnField - lastValidPercept.positionOnField).norm();
    
    if(jumpDistance > maxJumpDistance)
    {
      // Likely false positive - use smoothed previous value
      filteredBallPercept = lastValidPercept;
      filteredBallPercept.status = BallPercept::guessed;
      return;
    }
  }

  // Valid detection - smooth it
  filteredBallPercept = theBallPercept;
  
  if(useTemporalSmoothing && lastValidPercept.status != BallPercept::notSeen)
  {
    // Optional: Apply Kalman smoothing for very smooth motion
    // filteredBallPercept.positionOnField.x() = xFilter.update(theBallPercept.positionOnField.x());
    // filteredBallPercept.positionOnField.y() = yFilter.update(theBallPercept.positionOnField.y());
  }

  lastValidPercept = filteredBallPercept;
  framesSinceSeen = 0;
}
```

**Configuration: `Config/Scenarios/Default/ballPerceptFilter.cfg`**

```properties
maxJumpDistance = 100.0;        # Maximum realistic jump per frame (mm)
confidenceDecayFrames = 5;      # Show "guessed" for 5 frames without seeing
useTemporalSmoothing = true;    # Enable smoothing
```

---

## 2. Adaptive Threshold Adjustment

### Modify existing BallPerceptor.cpp

**Replace the `update()` method section:**

```cpp
void BallPerceptor::update(BallPercept& theBallPercept)
{
  DECLARE_DEBUG_DRAWING("module:BallPerceptor:spots", "drawingOnImage");
  
  DEBUG_RESPONSE_ONCE("module:BallPerceptor:compile")
    compile();

  theBallPercept.status = BallPercept::notSeen;

  if(!encoder.valid() || !classifier.valid() || !corrector.valid())
    return;

  const std::vector<Vector2i>& ballSpots = theBallSpots.ballSpots;
  if(ballSpots.empty())
    return;

  // ========== ADAPTIVE THRESHOLD ADJUSTMENT ==========
  float currentAcceptThreshold = acceptThreshold;
  float currentGuessedThreshold = guessedThreshold;
  
  // Adjust based on game phase
  if(theGameInfo.state == STATE_PLAYING)
  {
    // More permissive when actively playing
    currentAcceptThreshold = std::max(0.70f, acceptThreshold - 0.05f);
    currentGuessedThreshold = std::max(0.65f, guessedThreshold - 0.05f);
  }
  else if(theGameInfo.state == STATE_READY || theGameInfo.state == STATE_SET)
  {
    // Stricter when not playing - want to be sure
    currentAcceptThreshold = std::min(0.85f, acceptThreshold + 0.03f);
    currentGuessedThreshold = std::min(0.75f, guessedThreshold + 0.03f);
  }

  // Adjust if we haven't seen ball for a while
  int timeSinceSeen = theFrameInfo.getTimeSince(theBallModel.timeWhenLastSeen);
  if(timeSinceSeen > 1000)
  {
    // Haven't seen ball for 1+ second - be more permissive
    currentAcceptThreshold *= 0.95f;
  }
  // ====================================================

  float prob, bestProb = currentGuessedThreshold;
  Vector2f ballPosition, bestBallPosition;
  float radius, bestRadius;
  
  for(std::size_t i = 0; i < ballSpots.size(); ++i)
  {
    prob = apply(ballSpots[i], ballPosition, radius);

    COMPLEX_DRAWING("module:BallPerceptor:spots")
    {
      std::stringstream ss;
      ss << i << ": " << static_cast<int>(prob * 100);
      DRAW_TEXT("module:BallPerceptor:spots", ballSpots[i].x(), ballSpots[i].y(), 15, ColorRGBA::red, ss.str());
    }

    if(prob > bestProb)
    {
      bestProb = prob;
      bestBallPosition = ballPosition;
      bestRadius = radius;
      if(SystemCall::getMode() == SystemCall::physicalRobot && prob >= ensureThreshold)
        break;
    }
  }

  if(bestProb > currentGuessedThreshold)
  {
    theBallPercept.positionInImage = bestBallPosition;
    theBallPercept.radiusInImage = bestRadius;
    if(Transformation::imageToRobotHorizontalPlane(theImageCoordinateSystem.toCorrected(bestBallPosition), theBallSpecification.radius, theCameraMatrix, theCameraInfo, theBallPercept.positionOnField))
    {
      theBallPercept.status = bestProb >= currentAcceptThreshold ? BallPercept::seen : BallPercept::guessed;
      return;
    }
  }

  // ... rest of special case handling ...
}
```

---

## 3. Pre-filter Before Neural Network

### Add to BallPerceptor.cpp

```cpp
// Add this helper method to BallPerceptor class
private:
  /**
   * Quick heuristic to reject obviously bad candidates
   * Much faster than running through neural network
   */
  bool isLikelyBallSpot(const Vector2i& spot) const
  {
    // Check if spot is inside robot body contour
    if(!theBodyContour.isValidPoint(spot))
      return false;

    // Check pixel brightness at center
    const unsigned char brightness = theECImage.grayscaled[spot.y()][spot.x()];
    if(brightness < 100)  // Too dark
      return false;

    // Check immediate surroundings are relatively bright
    const int checkRadius = 3;
    int brightPixels = 0;
    for(int dy = -checkRadius; dy <= checkRadius; ++dy)
    {
      for(int dx = -checkRadius; dx <= checkRadius; ++dx)
      {
        int y = spot.y() + dy;
        int x = spot.x() + dx;
        if(x >= 0 && x < theECImage.grayscaled.width &&
           y >= 0 && y < theECImage.grayscaled.height)
        {
          if(theECImage.grayscaled[y][x] > 150)
            brightPixels++;
        }
      }
    }

    // Need at least 40% of surrounding pixels to be bright
    return brightPixels > ((2 * checkRadius + 1) * (2 * checkRadius + 1) * 0.4f);
  }

// Modify update() method:
public:
  void update(BallPercept& theBallPercept) override
  {
    // ... existing code ...
    
    const std::vector<Vector2i>& ballSpots = theBallSpots.ballSpots;
    if(ballSpots.empty())
      return;

    // Filter spots before expensive NN evaluation
    std::vector<Vector2i> validSpots;
    for(const auto& spot : ballSpots)
    {
      if(isLikelyBallSpot(spot))
        validSpots.push_back(spot);
    }

    // If pre-filter eliminated too many, use all (fallback)
    if(validSpots.empty())
      validSpots = ballSpots;

    // Now evaluate only valid spots through NN
    float prob, bestProb = guessedThreshold;
    Vector2f ballPosition, bestBallPosition;
    float radius, bestRadius;
    
    for(const auto& spot : validSpots)
    {
      prob = apply(spot, ballPosition, radius);
      // ... rest of evaluation ...
    }
  }
```

---

## 4. Configuration Profiler

### Create this helper to compare configurations

**File: `Tools/ConfigurationProfiler.h`**

```cpp
#pragma once

#include "Tools/Debugging/Stopwatch.h"
#include <map>
#include <string>

class ConfigurationProfiler
{
public:
  static void recordThreshold(const std::string& name, float value)
  {
    thresholds[name] = value;
  }

  static void printProfile()
  {
    OUTPUT_TEXT("===== Ball Perception Configuration =====");
    OUTPUT_TEXT("Thresholds:");
    for(const auto& [name, value] : thresholds)
    {
      OUTPUT_TEXT(name << ": " << value);
    }
    OUTPUT_TEXT("==========================================");
  }

private:
  static std::map<std::string, float> thresholds;
};
```

**Usage in BallPerceptor:**

```cpp
BallPerceptor::BallPerceptor() : ... 
{
  ConfigurationProfiler::recordThreshold("guessedThreshold", guessedThreshold);
  ConfigurationProfiler::recordThreshold("acceptThreshold", acceptThreshold);
  ConfigurationProfiler::recordThreshold("ensureThreshold", ensureThreshold);
  compile();
}

void BallPerceptor::update(...)
{
  DEBUG_RESPONSE_ONCE("module:BallPerceptor:profile")
    ConfigurationProfiler::printProfile();
  
  // ... rest of update ...
}
```

---

## 5. Performance Monitoring

### Add to BallPerceptor.cpp

```cpp
void BallPerceptor::update(BallPercept& theBallPercept)
{
  // ... existing code ...

  {
    STOPWATCH("module:BallPerceptor:spotFiltering");
    // Pre-filter spots
  }

  {
    STOPWATCH("module:BallPerceptor:neuralNetworkEvaluation");
    // NN processing
    for(const auto& spot : ballSpots)
    {
      prob = apply(spot, ballPosition, radius);
    }
  }

  {
    STOPWATCH("module:BallPerceptor:postProcessing");
    // Post-processing, transformation, etc.
  }

  // Log statistics periodically
  static int frameCounter = 0;
  if(++frameCounter % 100 == 0)
  {
    OUTPUT_TEXT("BallPerceptor stats - Spots processed: " << ballSpots.size() 
                << ", Confidence: " << (bestProb > 0 ? std::to_string(bestProb) : "N/A"));
  }
}
```

---

## 6. Neural Network Input Caching

### Optimize memory allocation in BallPerceptor

**Add to header:**

```cpp
class BallPerceptor : public BallPerceptorBase
{
private:
  // Reusable buffers to avoid allocations
  std::vector<uint8_t> patchBuffer;
  std::vector<uint8_t> normalizedBuffer;
  bool buffersInitialized = false;

  void initializeBuffers()
  {
    if(buffersInitialized) return;
    
    // Allocate once based on patchSize
    patchBuffer.resize(patchSize * patchSize);
    normalizedBuffer.resize(patchSize * patchSize);
    buffersInitialized = true;
  }
};
```

**Modify compile() to use buffers:**

```cpp
void BallPerceptor::compile()
{
  // ... existing compile code ...
  
  initializeBuffers();  // Pre-allocate reusable memory
}
```

**Use in apply() method:**

```cpp
float BallPerceptor::apply(const Vector2i& ballSpot, Vector2f& ballPosition, float& predRadius)
{
  // ... existing calculation ...

  STOPWATCH("module:BallPerceptor:getImageSection")
  {
    if(useFloat)
    {
      PatchUtilities::extractPatch(ballSpot, Vector2i(ballArea, ballArea), 
                                  Vector2i(patchSize, patchSize), 
                                  theECImage.grayscaled, 
                                  encoder.input(0).data(), extractionMode);
    }
    else
    {
      // Use pre-allocated buffer instead of temporary
      PatchUtilities::extractPatch(ballSpot, Vector2i(ballArea, ballArea), 
                                  Vector2i(patchSize, patchSize), 
                                  theECImage.grayscaled, 
                                  patchBuffer.data(), extractionMode);
      
      if(useContrastNormalization)
        PatchUtilities::normalizeContrast(patchBuffer.data(), 
                                         Vector2i(patchSize, patchSize), 
                                         contrastNormalizationPercent);
      
      // Copy to encoder input (could be optimized further)
      std::memcpy(encoder.input(0).data(), patchBuffer.data(), 
                 patchSize * patchSize * sizeof(uint8_t));
    }
  }
  
  // ... rest of method ...
}
```

---

## 7. Early Exit Optimization

### Already partially implemented, here's enhanced version

**In BallPerceptor::update():**

```cpp
float prob, bestProb = guessedThreshold;
Vector2f ballPosition, bestBallPosition;
float radius, bestRadius;

// Prioritize predicted spot if available
std::vector<Vector2i> prioritizedSpots;
if(theBallSpots.firstSpotIsPredicted && !ballSpots.empty())
{
  prioritizedSpots.push_back(ballSpots[0]);  // Predicted spot first
  for(std::size_t i = 1; i < ballSpots.size(); ++i)
    prioritizedSpots.push_back(ballSpots[i]);
}
else
{
  prioritizedSpots = ballSpots;
}

for(std::size_t i = 0; i < prioritizedSpots.size(); ++i)
{
  prob = apply(prioritizedSpots[i], ballPosition, radius);

  if(prob > bestProb)
  {
    bestProb = prob;
    bestBallPosition = ballPosition;
    bestRadius = radius;
    
    // EARLY EXIT CONDITIONS:
    
    // Physical robot mode - confident ball found
    if(SystemCall::getMode() == SystemCall::physicalRobot && 
       prob >= ensureThreshold)
    {
      break;  // Found good enough - stop searching
    }
    
    // Super high confidence found on predicted spot
    if(i == 0 && theBallSpots.firstSpotIsPredicted && 
       prob > 0.95f)
    {
      break;  // Trust prediction completely
    }
  }
  else if(bestProb >= acceptThreshold && 
          bestProb - prob > 0.2f)  // Gap getting larger
  {
    // Already found good ball, subsequent spots are worse
    // Unlikely to find better - stop early
    break;
  }
}
```

---

## 8. Batch Module Configuration

### Create alternate configuration sets for easy switching

**File: `Config/Scenarios/RealGame/ballPerceptor_aggressive.cfg`**

```properties
encoderName = "encoder.h5";
classifierName = "classify.h5";
correctorName = "corrector.h5";
guessedThreshold = 0.63;        # More responsive
acceptThreshold = 0.73;          # More aggressive
ensureThreshold = 0.83;          # Break earlier
useContrastNormalization = false;
ballAreaFactor = 2.8;            # Smaller patches
contrastNormalizationPercent = 0.01;
useFloat = false;
extractionMode = fast;
```

**File: `Config/Scenarios/RealGame/ballPerceptor_robust.cfg`**

```properties
encoderName = "encoder.h5";
classifierName = "classify.h5";
correctorName = "corrector.h5";
guessedThreshold = 0.72;        # Conservative
acceptThreshold = 0.82;          # Require confidence
ensureThreshold = 0.92;          # Be very sure
useContrastNormalization = true;
ballAreaFactor = 3.7;            # Larger patches
contrastNormalizationPercent = 0.025;
useFloat = false;
extractionMode = fast;
```

---

## Integration Checklist

- [ ] Copy desired snippets into your source files
- [ ] Add includes where needed
- [ ] Update CMakeLists.txt if adding new modules
- [ ] Verify compilation without errors
- [ ] Test in SimRobot
- [ ] Run through test scenarios
- [ ] Compare performance metrics
- [ ] Commit working versions to version control

---

## Testing Keywords

Search in SimRobot with these to verify implementations:

```
module:BallPerceptor:spots        # See confidence scores
module:BallPerceptor:profile      # See configuration
module:BallPerceptor:timing       # Performance stats
FilteredBallPercept               # If using filtering
```

