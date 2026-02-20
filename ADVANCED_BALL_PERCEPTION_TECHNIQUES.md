# Advanced Ball Perception Optimization Techniques

## 1. Neural Network Model Improvements

### Current Model Architecture
Your system uses 3 separate models:
- **encoder.h5** - Feature extraction (grayscale patch → feature vector)
- **classify.h5** - Classification (features → confidence score)
- **corrector.h5** - Position/radius refinement (features → [x, y, radius])

### Optimization Opportunities

#### A. Model Quantization
**Opportunity**: Convert to INT8 quantization
- Your current config already uses `useFloat = false` (uint8 inputs)
- Consider quantizing the model outputs too

**Action**: Check if ONNX models support full INT8 quantization
```
encoder.onnx  → encoder_int8.onnx (smaller, faster)
classify.onnx → classify_int8.onnx
corrector.onnx → corrector_int8.onnx
```

**Expected Impact**: 2-3x speedup, <1% accuracy loss (usually none)

#### B. Model Pruning
**Opportunity**: Remove redundant neurons
- If training infrastructure available, prune 30-50% of neurons
- Your models are relatively small already (good!)

#### C. Knowledge Distillation
**Opportunity**: Train smaller students from teacher models
- Smaller model → faster inference
- Same accuracy as current

**Tool**: Use CompiledNN framework already in codebase

---

## 2. Image-Level Optimizations

### Region-of-Interest (ROI) Limiting
**Problem**: Scanning entire image every frame

**Solution**: Only analyze ROI around predicted ball
```cpp
// Add to BallSpotsProvider::searchScanLines
Vector2f predictedBallPos;  // From WorldModelPrediction
if(hasDependablePredicton(predictedBallPos))
{
    // Only scan ±200px around prediction
    minScanLineIndex = getIndexForXPosition(predictedBallPos.x() - 200);
    maxScanLineIndex = getIndexForXPosition(predictedBallPos.x() + 200);
}
```

**Expected Impact**: 30-50% faster scanline phase

**Caveat**: Be careful with field edge cases - ball might leave and re-enter FOV

---

## 3. Two-Pass Detection Strategy

### Implement Coarse-to-Fine Approach
**Idea**: Quick rejection → Detailed verification

```cpp
// Phase 1: Fast rejection using 1/2 resolution
bool FastBallSpot::isLikelyBall(const Vector2i& spot)
{
    // Simple heuristic: check brightness, saturation
    // 100x faster than NN
    // High false positive rate is OK (Phase 2 filters these)
}

// Phase 2: Only evaluate surviving candidates through NN
ballSpots_filtered = ballSpots | FastBallSpot::filter();
for(auto& spot : ballSpots_filtered)
{
    compute(BallPerceptor::apply(spot));  // Full NN evaluation
}
```

**Expected Impact**: 40-60% overall speedup (depends on candidate count)

---

## 4. Adaptive Processing Based on Game State

### Context-Aware Thresholds
```cpp
void BallPerceptor::update(BallPercept& theBallPercept)
{
    // Adjust thresholds based on game situation
    float acceptThreshold = acceptThreshold;  // default
    
    if(theGameInfo.state == STATE_READY || theGameInfo.state == STATE_SET)
    {
        // Ball is stationary - stricter criteria
        acceptThreshold = 0.85;
    }
    else if(theGameInfo.state == STATE_PLAYING && theMotionInfo.motion == MotionRequest::walk)
    {
        // Robot walking - more forgiving
        acceptThreshold = 0.72;
    }
    else if(theGameInfo.gamePhase == GAME_PHASE_PENALTYSHOOT)
    {
        // Special penalty handling (already implemented!)
        // Can be more confident here
        acceptThreshold = 0.70;
    }
    
    // ... rest of detection logic using dynamic threshold
}
```

**Expected Impact**: More responsive detection in critical moments

---

## 5. Batch Processing Optimization

### Process Multiple Spots in Parallel
**If your system supports it** (multi-core robot):

```cpp
// Current: Sequential evaluation
for(size_t i = 0; i < ballSpots.size(); ++i)
{
    float prob = apply(ballSpots[i], ...);  // One at a time
}

// Optimized: Process 4 spots simultaneously with SIMD/OpenMP
#pragma omp parallel for
for(size_t i = 0; i < ballSpots.size(); ++i)
{
    float prob = apply(ballSpots[i], ...);  // Vectorized
}
```

**Expected Impact**: 2-4x speedup on multi-core (actual depends on HW)

**Note**: Add to CMakeLists.txt if not already enabled:
```cmake
add_compile_options(-fopenmp)
target_link_libraries(BallPerceptor PRIVATE omp)
```

---

## 6. Temporal Consistency Filtering

### Smooth Ball Detections Across Frames
**Problem**: Noisy frame-to-frame detections cause jitter

**Solution**: Apply temporal filtering
```cpp
// In BallPercept or a wrapper module
class BallPerceptFilter
{
    BallPercept lastPercept;
    
    void filter(BallPercept& current)
    {
        // Reject detections that jump too far
        if(!lastPercept.isValid())
        {
            lastPercept = current;
            return;
        }
        
        float maxJump = 50.f;  // mm
        if((current.positionOnField - lastPercept.positionOnField).norm() > maxJump)
        {
            // Likely false positive - use prediction instead
            current.status = BallPercept::guessed;
            current.positionOnField = predictedBallPosition;
        }
        
        lastPercept = current;
    }
};
```

**Expected Impact**: 
- Fewer false positives
- More stable ball tracking
- Better for downstream modules

---

## 7. Smart Candidate Filtering

### Pre-filter Candidates Before NN
```cpp
// In BallPerceptor::update
const std::vector<Vector2i>& ballSpots = theBallSpots.ballSpots;

// Filter obviously bad candidates
std::vector<Vector2i> validSpots;
for(const auto& spot : ballSpots)
{
    // Quick heuristic checks:
    if(!isInsideRobot(spot) &&           // Not inside robot body
       isPixelBright(spot) &&             // Bright enough
       isSurroundedByGreen(spot) &&       // Green context
       isReasonableSizeInImage(spot))     // Reasonable ball size
    {
        validSpots.push_back(spot);
    }
}

// Now process only valid spots through expensive NN
for(const auto& spot : validSpots)
{
    prob = apply(spot, ...);
}
```

**Expected Impact**: 20-40% reduction in NN evaluations

**Implementation**: Most of this is already done in BallSpotsProvider!

---

## 8. Network Input Preprocessing Optimization

### Cache Patch Extraction
**Problem**: Extracting and normalizing patches is repetitive

**Solution**: Reuse computations
```cpp
// Pre-allocate reusable buffers
class BallPerceptor : public BallPerceptorBase
{
private:
    std::vector<uint8_t> patchBuffer;      // Reuse memory
    std::vector<float> normalizedBuffer;   // Pre-allocated
    
    void update(BallPercept& theBallPercept) override
    {
        for(const auto& spot : ballSpots)
        {
            // Extract to pre-allocated buffer
            extractPatch(spot, patchBuffer);
            
            // Normalize in-place
            normalizeContrast(patchBuffer);
            
            // Use for NN
            encoder.input(0).data() = patchBuffer.data();
            encoder.apply();
        }
    }
};
```

**Expected Impact**: 5-10% memory allocation speedup

---

## 9. Hybrid Detection Pipeline

### Combine Multiple Detection Methods
**Idea**: Not all situations need full NN evaluation

```cpp
struct BallDetectionResult
{
    Vector2i position;
    float confidence;
    enum Source { ScanLine, Neural, Prediction } source;
};

BallDetectionResult fast_detect(const Vector2i& spot)
{
    // 1. Try scanline-only confirmation (fast)
    if(validateViaScanning(spot))
        return {spot, 0.85f, ScanLine};
    
    // 2. Try NN (expensive)
    float nn_confidence = apply(spot, ...);
    if(nn_confidence > 0.7f)
        return {spot, nn_confidence, Neural};
    
    // 3. Is it the predicted spot?
    if(isPredictedSpot(spot))
        return {spot, 0.6f, Prediction};
    
    return {spot, 0.0f, None};
}
```

**Expected Impact**: Variable, but can reduce NN calls by 50%+

---

## 10. Memory Bandwidth Optimization

### Understanding Your Hardware
Physical Nao robot has limited memory bandwidth (~2-3 GB/s)

**Optimization**: 
- Use compact data types (uint8 ✓ already doing)
- Minimize memory copies
- Keep working set small

**Current status**: Already optimized with uint8 inputs!

---

## 11. Profiling & Benchmarking

### Measure Actual Performance
```cpp
// Add to BallPerceptor::update()
{
    STOPWATCH("module:BallPerceptor:candidateStage");
    // Process candidates
}

{
    STOPWATCH("module:BallPerceptor:nnStage");
    for(const auto& spot : validSpots)
        apply(spot, ...);
}
```

**Use SimRobot's Timings view** to see breakdown:
- Time in scanlines
- Time in NN evaluation
- Time in post-processing

---

## Configuration Files to Create

### balanced_config.cfg
```properties
# For tournament play - good speed + accuracy
encoderName = "encoder.h5";
classifierName = "classify.h5";
correctorName = "corrector.h5";

guessedThreshold = 0.68;
acceptThreshold = 0.78;
ensureThreshold = 0.87;

ballAreaFactor = 3.0;
useContrastNormalization = true;
contrastNormalizationPercent = 0.01;
useFloat = false;
extractionMode = fast;
```

### fast_config.cfg
```properties
# Maximum speed - for slow robots
encoderName = "encoder.h5";
classifierName = "classify.h5";
correctorName = "corrector.h5";

guessedThreshold = 0.63;
acceptThreshold = 0.73;
ensureThreshold = 0.83;

ballAreaFactor = 2.5;
useContrastNormalization = false;
useFloat = false;
extractionMode = fast;
```

---

## Implementation Priority

### Easy wins (implement first):
1. ✅ Threshold tuning (no code changes)
2. ✅ Scanline parameter adjustment
3. ✅ Temporal filtering (small addition)

### Medium effort:
4. Coarse-to-fine detection
5. Adaptive thresholds based on game state
6. Pre-filtering before NN

### Advanced (max payoff):
7. Model quantization (requires retrain if not available)
8. Parallel processing
9. Hybrid detection pipeline

---

## Testing Strategy

### 1. offline benchmark script
```bash
# Run through recorded logs to detect baseline
./benchmark configs/default.cfg logs/*.bhlog > results_default.txt
./benchmark configs/balanced.cfg logs/*.bhlog > results_balanced.txt
```

### 2. Compare:
- Detection rate (true positive / expected positives)
- False positive rate
- Average confidence score
- Processing time per frame

### 3. A/B test in simulator:
- Run same scenario with different configs
- Measure ball tracking smoothness
- Check response time to sudden ball appearance

---

## Potential Pitfalls to Avoid

### ❌ Don't:
- Set ballAreaFactor < 2.0 (loses context)
- Set acceptThreshold < 0.65 (too many false positives)
- Disable scanline checks entirely (misses balls)
- Use too aggressive parallel processing without synchronization

### ✓ Do:
- Test incrementally (one parameter at a time)
- Validate in actual game conditions
- Monitor false positive rates
- Keep backup configurations

