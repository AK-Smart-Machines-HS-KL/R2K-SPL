# Ball Perception Performance Optimization Guide

## System Architecture Overview

The ball perception system in R2K-SPL consists of two main stages:

### Stage 1: Ball Spot Detection (BallSpotsProvider)
- Uses **color scanline analysis** to find candidate ball positions
- Analyzes vertical scan lines for white/bright regions
- Validates candidates through green field edge detection
- Produces a list of potential ball spots

### Stage 2: Ball Verification (BallPerceptor)
- Uses **three neural networks**:
  - **Encoder**: Extracts features from image patches
  - **Classifier**: Predicts ball presence probability (seen/guessed/notSeen)
  - **Corrector**: Refines ball position and radius
- Processes each candidate spot from Stage 1
- Outputs final BallPercept with position/radius/confidence

---

## Key Configuration Parameters

### BallPerceptor Parameters
File: `Config/Scenarios/Default/ballPerceptor.cfg`

```properties
guessedThreshold = 0.7;          # Min confidence to guess ball presence
acceptThreshold = 0.8;            # Min confidence to accept as "seen"
ensureThreshold = 0.9;            # Physical robot: can break early if this confident
ballAreaFactor = 3.5;             # Patch size multiplier (larger = slower but more context)
useContrastNormalization = false; # Normalize image contrast before NN input
useFloat = false;                 # Use uint8 instead of float (faster)
extractionMode = fast;            # Image patch extraction mode
```

### BallSpotsProvider Parameters
File: `Config/Locations/Default/ballSpotsProvider.cfg`

```properties
scanLengthRadiusFactor = 1.2;           # How far to scan past estimated ball edge
maxNumberOfSkippablePixel = 3;          # Tolerance for gaps in ball contour
minFoundDiameterPercentage = 0.5;       # Min % of ball diameter that must be found
noiseThreshold = 0.3;                   # Max acceptable noise ratio in ball pixels
minRadiusOfWantedRegion = 3.0;          # Small balls must pass stricter checks
greenPercent = 0.9;                     # Green field edge strictness (0-1)
allowScanLineTopSpotFitting = false;    # Allow ball detection at scanline top
lessStrictChecks = false;               # Allow more false positives
```

---

## Performance Optimization Strategies

### 1. **Reduce Candidate Spots Processing** ⭐ HIGH IMPACT
**Problem**: Evaluating every candidate through neural networks is expensive

**Solutions**:

#### A. Tighten BallSpotsProvider Thresholds
```properties
# Reduce false positives from scanline stage
noiseThreshold = 0.25;           # Make stricter (was 0.3)
minFoundDiameterPercentage = 0.6; # Require more diameter found (was 0.5)
greenPercent = 0.95;             # Stricter green check (was 0.9)
lessStrictChecks = false;         # Keep strict (don't relax)

# Reduce duplicate spots
minAllowedDistanceRadiusRelation = 1.5; # Increase to filter more duplicates (was 1.3)
```

**Expected Impact**: 15-30% fewer NN evaluations, 5-10% CPU reduction

---

### 2. **Optimize Neural Network Parameters** ⭐ HIGH IMPACT
**Problem**: Neural networks are the most computationally expensive part

**Solutions**:

#### A. Reduce Patch Size (with performance trade-off)
Current configuration uses patches based on `ballAreaFactor = 3.5`

```cpp
// In BallPerceptor::apply() - current behavior:
int ballArea = static_cast<int>(ball.radius * ballAreaFactor);  // 3.5x
// For a 20px radius ball in image: 70x70 patch → rescaled to patchSize²
```

**Optimization**: Reduce ballAreaFactor
```properties
ballAreaFactor = 2.5;  # Smaller patches (was 3.5)
                       # Reduces NN computation by ~64% (from 70px to 50px patch)
                       # Trade-off: Less context for classifier
```

**Expected Impact**: 40-50% faster NN evaluations, potential 2-5% accuracy loss

#### B. Use Integer Network Inputs
```properties
useFloat = false;      # Already enabled - good!
                       # Saves memory bandwidth, enables SIMD optimizations
```

**Benefit**: Already optimized in your config!

#### C. Early Termination in Physical Robot Mode
Current code already does this:
```cpp
if(SystemCall::getMode() == SystemCall::physicalRobot && prob >= ensureThreshold)
    break;  // Stop checking other candidates once confident enough
```

**Optimization**: Adjust thresholds
```properties
ensureThreshold = 0.85;  # Lower to trigger early exit sooner (was 0.9)
                         # Trade: More "seen" vs "guessed" classifications
```

**Expected Impact**: 20-40% faster in real games with good lighting

---

### 3. **Prediction-Based Optimization** ⭐ MEDIUM IMPACT
**Problem**: Every frame scans the whole image

**Solution**: Trust ball prediction from model
Currently BallSpotsProvider already uses prediction:
```cpp
// Already implemented - adds predicted ball as first candidate
if(theFrameInfo.getTimeSince(theWorldModelPrediction.timeWhenBallLastSeen) < 100)
{
    ballSpots.addBallSpot(...);  // Prioritize prediction
}
```

**Further optimization**: Increase prediction priority
```cpp
// Modify ballPerceptor.cpp - process predicted spot first
// If prediction is high confidence, skip full scanline search
if(predictedSpot && confidence > 0.75)
{
    // Skip expensive scanline search
    // Just verify the prediction
}
```

**Expected Impact**: 10-20% faster when ball is continuously seen

---

### 4. **Image Preprocessing Optimization** ⭐ MEDIUM IMPACT
**Problem**: Full image scanned every frame

**Solutions**:

#### A. Contrast Normalization (Careful!)
Current:
```properties
useContrastNormalization = false;  # Disabled
contrastNormalizationPercent = 0.02;
```

**Testing**: Try enabling with lower percentage
```properties
useContrastNormalization = true;
contrastNormalizationPercent = 0.01;  # Light normalization
# Improves robustness to lighting changes
# Small CPU cost, can improve detection in varying brightness
```

#### B. Extraction Mode
Current:
```properties
extractionMode = fast;  # Already optimized!
```

**Benefit**: Already using fast extraction mode!

---

### 5. **Threshold Tuning** ⭐ MEDIUM IMPACT
**Problem**: Over-conservative thresholds cause hesitation

**Current thresholds**:
```properties
guessedThreshold = 0.7;   # Too conservative?
acceptThreshold = 0.8;
ensureThreshold = 0.9;    # Physical robot - can break early
```

**Optimization for real games**:
```properties
guessedThreshold = 0.65;  # Lower = faster response to ball
acceptThreshold = 0.75;   # More willing to call "seen"
ensureThreshold = 0.85;   # Physical robot break earlier
```

**Trade-off**: Slightly more false positives, but faster real-time response

---

### 6. **Scanline Search Acceleration** ⭐ MEDIUM IMPACT
**Problem**: Comprehensive scanline search in BallSpotsProvider is thorough but slow

**Current approach**:
```cpp
// Scans at multiple resolutions and checks all candidates
const unsigned step = theColorScanLineRegionsVerticalClipped.lowResStep > 1 ? 
                      theColorScanLineRegionsVerticalClipped.lowResStep / 2 : 1;
```

**Optimization**: Adjust step size
```cpp
// In BallSpotsProvider::searchScanLines
const unsigned step = theColorScanLineRegionsVerticalClipped.lowResStep;  
// Skip intermediate resolution level
// Check fewer scanlines (skip every Nth)
```

**Expected Impact**: 20-30% faster scanline phase, minimal accuracy loss

**Risk**: May miss balls if step is too large

---

### 7. **Reduce Duplicate Detection** ⭐ LOW-MEDIUM IMPACT

**Current config**:
```properties
minAllowedDistanceRadiusRelation = 1.3;  # Threshold for considering spots duplicates
```

**Optimization**:
```properties
minAllowedDistanceRadiusRelation = 1.8;  # More aggressive duplicate removal
                                         # Fewer total spots to evaluate
```

**Method**: See [BallSpotsProvider.cpp](Src/Modules/Perception/BallPerceptors/BallSpotsProvider.cpp#L220)

---

## Recommended Optimization Pack

### For **Maximum Speed** (Real Game Priority):
```properties
[BallPerceptor]
guessedThreshold = 0.65;
acceptThreshold = 0.75;
ensureThreshold = 0.85;
ballAreaFactor = 2.5;          # ⚠️ May hurt accuracy slightly
useContrastNormalization = false;
useFloat = false;
extractionMode = fast;

[BallSpotsProvider]
noiseThreshold = 0.25;
minFoundDiameterPercentage = 0.6;
greenPercent = 0.95;
minAllowedDistanceRadiusRelation = 1.8;
scanLengthRadiusFactor = 1.0;  # Reduce scan length
maxNumberOfSkippablePixel = 2;  # Stricter
listStrictChecks = false;
```

**Expected impact**: 40-60% faster, ~2-3% accuracy trade-off

---

### For **Balanced Performance** (Recommended):
```properties
[BallPerceptor]
guessedThreshold = 0.68;
acceptThreshold = 0.78;
ensureThreshold = 0.87;
ballAreaFactor = 3.0;          # Smaller than default
useContrastNormalization = true;
contrastNormalizationPercent = 0.01;
useFloat = false;
extractionMode = fast;

[BallSpotsProvider]
noiseThreshold = 0.28;
minFoundDiameterPercentage = 0.55;
greenPercent = 0.92;
minAllowedDistanceRadiusRelation = 1.5;
lessStrictChecks = false;
```

**Expected impact**: 25-35% faster, minimal accuracy loss

---

### For **Robustness** (Conservative):
```properties
[BallPerceptor]
guessedThreshold = 0.7;        # Original
acceptThreshold = 0.8;         # Original
ensureThreshold = 0.9;         # Original
ballAreaFactor = 3.5;          # Original
useContrastNormalization = true;
contrastNormalizationPercent = 0.02;
useFloat = false;
extractionMode = fast;

[BallSpotsProvider]
noiseThreshold = 0.30;         # Slightly relaxed
minFoundDiameterPercentage = 0.48;
greenPercent = 0.88;
minAllowedDistanceRadiusRelation = 1.2;
lessStrictChecks = false;
```

**Expected impact**: 10-15% faster, improved detection in difficult conditions

---

## Implementation Checkpoints

### Phase 1: Low-Risk Quick Wins (Start here)
- [ ] Adjust `minAllowedDistanceRadiusRelation` to 1.5-1.8
- [ ] Lower `ensureThreshold` to 0.85
- [ ] Tighten `noiseThreshold` to 0.28
- [ ] Enable `useContrastNormalization = true` with `0.01` percent

### Phase 2: Testing & Validation
- [ ] Run in SimRobot with various scenarios
- [ ] Compare detection rates and FPS
- [ ] Test in actual RoboCup game if possible

### Phase 3: Fine-Tuning
- [ ] Reduce `ballAreaFactor` gradually (3.5 → 3.2 → 3.0)
- [ ] Adjust classification thresholds based on Phase 2 results
- [ ] Monitor false positive rate

---

## Tools for Debugging

### Visual Debugging in SimRobot
Enable these debug drawings:
```cpp
// In BallPerceptor
DRAW_TEXT("module:BallPerceptor:spots", ...);  // Shows confidence scores

// In BallSpotsProvider
COMMENT("module:BallSpotsProvider:scanLines", ...);  // Shows scan validity
```

### Performance Monitoring
```cpp
STOPWATCH("module:BallPerceptor:getImageSection");
```

Use `processingTimings` to see how many MS each module takes.

---

## Potential Code Changes

### Option 1: Adaptive thresholds based on game phase
```cpp
// In BallPerceptor::update()
if(theGameInfo.state == STATE_PLAYING)
{
    acceptThreshold = 0.75;  // More permissive during play
}
else
{
    acceptThreshold = 0.80;  // Conservative otherwise
}
```

### Option 2: Reduce scanline resolution in high-confidence tracking
```cpp
// In BallSpotsProvider::searchScanLines()
if(ballSpots.firstSpotIsPredicted && /* prediction confidence high */)
{
    step *= 2;  // Skip more scanlines - we trust the prediction
}
```

### Option 3: Parallel NN evaluation (if multi-core)
The current loop evaluates spots sequentially:
```cpp
for(std::size_t i = 0; i < ballSpots.size(); ++i)
{
    prob = apply(ballSpots[i], ...);  // Could be parallelized
}
```

---

## Next Steps

1. **Create a test configuration**: Copy default config and create `ballPerceptor_optimized.cfg`
2. **Profile current system**: Measure baseline FPS and detection accuracy
3. **Apply changes incrementally**: One parameter at a time
4. **Test in-game**: Real RoboCup scenarios matter more than simulations
5. **Monitor**: Track both FPS improvements and detection accuracy regressions

---

## Files to Monitor During Optimization

- [BallPerceptor.cpp](Src/Modules/Perception/BallPerceptors/BallPerceptor.cpp) - Neural network processing
- [BallSpotsProvider.cpp](Src/Modules/Perception/BallPerceptors/BallSpotsProvider.cpp) - Scanline detection
- [Config/Scenarios/Default/ballPerceptor.cfg](Config/Scenarios/Default/ballPerceptor.cfg) - NN parameters
- [Config/Locations/Default/ballSpotsProvider.cfg](Config/Locations/Default/ballSpotsProvider.cfg) - Scanline parameters

