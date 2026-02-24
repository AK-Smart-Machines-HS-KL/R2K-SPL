# Reducing False Positives: Robot Feet Misdetection

## Problem Analysis

**Why robot feet are detected as balls:**

1. **White regions on feet** - Joint covers, shoes, ankle parts are white
2. **Sharp contrast** - Against dark ground or shadows
3. **Circular appearance** - Joint components can look round in images
4. **Scanline matches** - BallSpotsProvider finds white regions on vertical scanlines
5. **Classifier weakness** - NN trained without enough foot examples as negatives

**Current false positive sources:**
```
Robot foot → BallSpotsProvider finds white region
         ↓
         → Added to candidate list
         ↓
         → BallPerceptor extracts patch
         ↓
         → Classifier says "maybe ball" (60-75% confidence)
         ↓
         → False positive detected
```

---

## Solution 1: Filtering at BallSpotsProvider Level (RECOMMENDED) ⭐

### A. Reject Candidates Too Close to Robot

**File: [Config/Locations/Default/ballSpotsProvider.cfg](Config/Locations/Default/ballSpotsProvider.cfg)**

The BodyContour is already used but can be more aggressive:

```properties
# Current settings
minRadiusOfWantedRegion = 3.0;

# ADD THESE NEW PARAMETERS (requires code change):
minDistanceFromRobotBody = 50;    # Reject spots within 50px of robot
rejectFeetRegions = true;          # Special filtering for feet
feetNoiseThreshold = 0.25;         # Stricter for foot regions
```

### B. Implement Context-Aware Rejection

**Add to BallSpotsProvider.cpp:**

```cpp
// In BallSpotsProvider::searchScanLines() method

// Add this check before adding candidate to ballSpots
bool isCandidateTooCloseToRobot(const Vector2i& spot, 
                                const BodyContour& theBodyContour,
                                const CameraInfo& theCameraInfo) const
{
    // Check if spot is on or very near robot contour
    // Robot feet typically at specific y-range
    const int minYForFeet = theCameraInfo.height - 150;  // Lower 150 pixels
    
    if(spot.y() > minYForFeet)
    {
        // In foot region - use stricter validation
        // Check horizontal symmetry (feet are symmetric)
        // Check contour distance
        
        if(theBodyContour.isValidPoint(spot))
            return false;  // Inside robot body - reject
    }
    
    return true;  // Safe to consider
}

// Usage in searchScanLines:
for(...) {
    if(!isCandidateTooCloseToRobot(candidateSpot, theBodyContour, theCameraInfo))
        continue;  // Skip this candidate
    
    // ... rest of validation
}
```

---

## Solution 2: Improve BallPerceptor Classifier (MEDIUM EFFORT)

### A. Retrain Classifier on Foot Negatives

**Collect foot patches and retrain:**

```bash
# 1. Extract foot patches from real games
python3 Tools/DataCollection/extract_foot_patches.py \
  --log-dir recorded_games/ \
  --output-dir dataset/foot_negatives/ \
  --num-samples 5000

# 2. Add to training dataset as hard negatives
cp dataset/foot_negatives/*.jpg dataset/negative/

# 3. Retrain classifier with foot examples
python3 Tools/Training/train_classifier.py \
  --dataset-dir dataset_with_feet/ \
  --encoder ./encoder_trained.h5 \
  --output ./classifier_trained_with_feet.h5
```

**Python script to extract foot patches:**

**File: `Tools/DataCollection/extract_foot_patches.py`**

```python
#!/usr/bin/env python3
"""Extract foot patches from recordings for use as hard negatives"""

import cv2
import json
from pathlib import Path
import numpy as np

def extract_foot_patches(log_file: str, output_dir: Path, 
                        patch_size: int = 64, num_samples: int = 100):
    """
    Extract patches from robot feet regions in recordings
    """
    # Load log file (assuming it contains frame data and robot pose)
    with open(log_file, 'r') as f:
        frames = json.load(f)
    
    extracted = 0
    output_dir.mkdir(parents=True, exist_ok=True)
    
    for frame_idx, frame in enumerate(frames):
        if extracted >= num_samples:
            break
        
        image = cv2.imread(frame['image_path'], cv2.IMREAD_GRAYSCALE)
        if image is None:
            continue
        
        # Get robot pose (from frame metadata)
        robot_pose = frame['robot_pose']
        camera_matrix = frame['camera_matrix']
        
        # Project robot feet to image
        feet_positions = project_feet_to_image(robot_pose, camera_matrix)
        
        for foot_pos in feet_positions:
            x, y = int(foot_pos[0]), int(foot_pos[1])
            half = patch_size // 2
            
            # Check bounds
            if (x - half < 0 or x + half >= image.shape[1] or
                y - half < 0 or y + half >= image.shape[0]):
                continue
            
            # Extract patch
            patch = image[y-half:y+half, x-half:x+half]
            
            # Save as negative example
            filename = output_dir / f"foot_negative_{extracted:06d}.jpg"
            cv2.imwrite(str(filename), patch)
            
            extracted += 1
    
    print(f"Extracted {extracted} foot patches")

def project_feet_to_image(robot_pose, camera_matrix):
    """Project robot foot positions to image coordinates"""
    # This is pseudo-code - implement based on your coordinate system
    # Typically: forward kinematics → camera projection
    feet_3d = [
        robot_pose.translation + Vector3f(100, 50, 0),   # Left foot
        robot_pose.translation + Vector3f(100, -50, 0),  # Right foot
    ]
    
    feet_2d = []
    for foot_3d in feet_3d:
        foot_2d = camera_matrix.project(foot_3d)
        if 0 <= foot_2d[0] < 640 and 0 <= foot_2d[1] < 480:
            feet_2d.append(foot_2d)
    
    return feet_2d
```

### B. Add Context Feature to Classifier

Modify the classifier to take additional context:

```python
# Enhanced classifier with context
def build_context_aware_classifier(encoder_output_size=128, 
                                  context_features=10):
    """
    Classifier that considers:
    - Image patch (from encoder)
    - Distance from robot body
    - Expected ball size at this distance
    - Position in image
    """
    
    inputs = [
        keras.layers.Input(shape=(encoder_output_size,), name='patch_features'),
        keras.layers.Input(shape=(context_features,), name='context'),
    ]
    
    # Process patch features
    x1 = keras.layers.Dense(64, activation='relu')(inputs[0])
    x1 = keras.layers.Dropout(0.2)(x1)
    
    # Process context
    x2 = keras.layers.Dense(32, activation='relu')(inputs[1])
    x2 = keras.layers.Dropout(0.2)(x2)
    
    # Combine
    combined = keras.layers.Concatenate()([x1, x2])
    combined = keras.layers.Dense(64, activation='relu')(combined)
    combined = keras.layers.Dropout(0.3)(combined)
    
    output = keras.layers.Dense(1, activation='sigmoid')(combined)
    
    classifier = keras.models.Model(inputs=inputs, outputs=output)
    return classifier

# Context features:
context = np.array([
    distance_from_robot_center,
    is_below_robot_height,
    expected_ball_radius_pixels,
    is_on_ground_level,
    nearby_white_region_count,
    gradient_roundness_score,
    color_variance,
    saturation_level,
    edge_sharpness,
    symmetry_score,
])
```

---

## Solution 3: Temporal Filtering (QUICK & EFFECTIVE) ⚡

### Reject Detections That Violate Motion Models

**Add to BallPerceptor or create filter module:**

```cpp
class FootFalsePositiveFilter
{
public:
    bool isFalsePositive(const BallPercept& current,
                        const Vector2f& robotSpeed,
                        const float gameTime) const
    {
        // If ball was in robot feet last frame,
        // unlikely to suddenly jump away
        
        if(!lastPercept.isValid())
        {
            lastPercept = current;
            return false;
        }
        
        Vector2f delta = current.positionOnField - lastPercept.positionOnField;
        
        // Check 1: Unrealistic jump for stationary robot
        if(robotSpeed.norm() < 50.f)  // Robot nearly stationary
        {
            float maxJump = 100.f;  // mm
            if(delta.norm() > maxJump)
            {
                // Likely false positive
                return true;
            }
        }
        
        // Check 2: Ball appears suddenly in foot region then disappears
        static int footFrameCounter = 0;
        const float footY = 300.f;  // Field y-coordinate of feet
        
        if(current.positionOnField.y() > footY - 100 &&
           current.positionOnField.y() < footY + 100)
        {
            footFrameCounter++;
            if(footFrameCounter > 3)
            {
                // Been in foot region >3 frames - probably false
                return true;
            }
        }
        else
        {
            footFrameCounter = 0;
        }
        
        // Check 3: Unrealistic motion
        if(lastPercept.isValid())
        {
            Vector2f velocity = delta / deltaT;
            if(velocity.norm() > 1000.f)  // 1 m/s - unrealistic for ball at feet
            {
                return true;
            }
        }
        
        lastPercept = current;
        return false;
    }

private:
    mutable BallPercept lastPercept;
    mutable int frameCounter = 0;
};
```

---

## Solution 4: Feature-Based Filtering (ADVANCED)

### Analyze Patch Features to Detect Feet

```cpp
// Add to BallPerceptor::apply() method

bool isFeetLikePattern(const Vector2i& spot,
                       const ECImage& theECImage,
                       int patchSize) const
{
    // Extract features of the patch
    
    // Feature 1: Gaussian blur variance
    // Feet have more high-frequency content (multiple edges)
    // Ball is smooth (low frequency)
    
    // Feature 2: Color variance
    // Feet: white + black mixed
    // Ball: uniform white
    
    // Feature 3: Edge count
    // Feet: many sharp edges
    // Ball: circular edge
    
    // Feature 4: Horizontal symmetry
    // Feet: not symmetric
    // Ball: mostly symmetric
    
    // Extract statistics
    int half = patchSize / 2;
    int left_brightness = 0, right_brightness = 0;
    
    for(int y = -half + 5; y < half - 5; ++y)
    {
        for(int x = -half + 5; x < half - 5; ++x)
        {
            if(x < 0)
                left_brightness += theECImage.grayscaled[spot.y() + y][spot.x() + x];
            else
                right_brightness += theECImage.grayscaled[spot.y() + y][spot.x() + x];
        }
    }
    
    // Large difference = likely feet (asymmetric)
    float asymmetry = std::abs(left_brightness - right_brightness) / 
                     (float)(left_brightness + right_brightness);
    
    if(asymmetry > 0.3)  // Highly asymmetric
        return true;  // Probably feet
    
    return false;
}

// Use in apply() method:
float BallPerceptor::apply(const Vector2i& ballSpot, ...)
{
    // ... existing code ...
    
    // Before NN evaluation
    if(isFeetLikePattern(ballSpot, theECImage, patchSize))
    {
        return -1.f;  // Reject immediately
    }
    
    // ... continue with NN ...
}
```

---

## Solution 5: Training Data Improvement (BEST LONG-TERM)

### Retrain with Extensive Negative Examples

**Best approach: Include many foot patches in negative training data**

```bash
# Workflow
1. Collect many foot patches from games
2. Add to negative training set (target: 10,000+ foot samples)
3. Retrain classifier
4. Now classifier is aware of foot patterns

Expected improvement: 50-80% reduction in foot false positives
```

**Dataset structure:**
```
dataset/
├── negative/
│   ├── field_background_*.jpg (5000 samples)
│   ├── robot_body_*.jpg (2000 samples)
│   ├── robot_foot_*.jpg (3000 samples)  ← ADD THESE
│   └── other_robots_*.jpg (2000 samples)
└── positive/
    └── ball_*.jpg (10000 samples)
```

---

## Solution 6: Post-Processing Filtering

### Geometric Constraints

```cpp
// Add to BallPerceptor::update() after candidate evaluation

bool isGeometricallyInvalid(const BallPercept& percept,
                           const RobotPose& theRobotPose,
                           const CameraInfo& theCameraInfo) const
{
    // Constraint 1: Ball shouldn't be inside robot body
    if(percept.positionOnField.norm() < 150.f)
    {
        // Inside robot's body radius
        return true;
    }
    
    // Constraint 2: Ball shouldn't be directly below robot
    // (unless we're picking it up, which is rare)
    Vector2f robotToGround = percept.positionOnField;
    if(std::abs(robotToGround.x()) < 80.f &&
       robotToGround.y() > -50.f && robotToGround.y() < 50.f)
    {
        // Could be feet region
        
        // Additional check: ball should be visible
        // If confidence is marginal + in foot region = reject
        if(/* confidence is low */)
            return true;
    }
    
    return false;
}

// Use in update():
if(isGeometricallyInvalid(theBallPercept, theRobotPose, theCameraInfo))
{
    theBallPercept.status = BallPercept::notSeen;
    return;
}
```

---

## Recommended Implementation Plan

### Phase 1: IMMEDIATE (30 minutes)
Implement at BallSpotsProvider level:

```properties
# Config changes only - no code
# Make scanline validation stricter for foot regions

# File: Config/Locations/Default/ballSpotsProvider.cfg
noiseThreshold = 0.25;           # Lower from 0.3 (stricter)
minFoundDiameterPercentage = 0.6; # Increase from 0.5 (stricter)
greenPercent = 0.95;             # Increase from 0.9 (stricter)
```

**Expected impact:** 20-30% foot false positive reduction

### Phase 2: SHORT TERM (1-2 hours)
Add temporal filtering to BallPerceptor:

```cpp
// Add motion validation
// Reject detections with unrealistic jumps
// Reject detections oscillating at robot feet
```

**Expected impact:** 40-50% foot false positive reduction

### Phase 3: MEDIUM TERM (1 week)
Collect foot patches and retrain classifier:

```bash
# Extract foot negatives from game logs
# Retrain classifier with foot awareness
# Convert and deploy
```

**Expected impact:** 70-80% foot false positive reduction

### Phase 4: LONG TERM (2-3 weeks)
Add context-aware features:

```cpp
// Enhanced classifier considers:
// - Position in image
// - Distance from robot
// - Expected ball size
// - Surrounding features
```

**Expected impact:** 85-90% foot false positive reduction

---

## Quick Config Change (FASTEST)

Edit `Config/Locations/Default/ballSpotsProvider.cfg`:

```properties
# BEFORE
minFoundDiameterPercentage = 0.5;
noiseThreshold = 0.3;
greenPercent = 0.9;
lessStrictChecks = false;

# AFTER (stricter = fewer foot false positives)
minFoundDiameterPercentage = 0.6;
noiseThreshold = 0.25;
greenPercent = 0.93;
lessStrictChecks = false;
```

Test in SimRobot:
- Do foot regions still get detected?
- Did valid ball detection rate change?

---

## Recommended Solution Priority

| Solution | Time | Impact | Difficulty |
|----------|------|--------|-----------|
| 1. Config tightening | 30 min | 20-30% | None |
| 2. Temporal filtering | 2 hours | 40-50% | Low |
| 3. Retrain classifier | 1 week | 70-80% | Medium |
| 4. Context features | 2 weeks | 85-90% | High |

**My recommendation:** Start with #1 (config) + #2 (temporal filter) for 50-60% improvement in ~2.5 hours.

---

## Testing the Improvements

### Test Scenario
```
1. Stand in front of Nao
2. Let Nao see your feet
3. Count false positives in debug output
4. Apply fix
5. Repeat test
6. Measure improvement
```

### Metrics to Track
```cpp
// Add to BallPerceptor
static int foot_false_positives = 0;
static int total_detections = 0;

void BallPerceptor::update(BallPercept& theBallPercept)
{
    // ...
    
    if(is_foot_false_positive_detected)
        foot_false_positives++;
    
    total_detections++;
    
    if(total_detections % 100 == 0)
    {
        OUTPUT_TEXT("False positive rate: " 
                   << foot_false_positives << "/" << total_detections);
    }
}
```

---

## Code to Add Right Now

### Simplest Improvement: Stricter Ground-Level Filtering

```cpp
// Add to BallSpotsProvider::searchScanLines()
// Before: ballSpots.ballSpots.emplace_back(circle.center.cast<int>());

// Check if candidate is suspiciously low (foot region)
if(lowestYOfCurrentArea > theCameraInfo.height * 0.85f)
{
    // In lower 15% of image (foot region)
    // Apply extra strict validation
    
    if(foundDiameterPercentage < 0.65f)  // Require 65% instead of 50%
        goto noSpot;
    
    if(noise > 0.25f)  // Require cleaner pattern
        goto noSpot;
}
```

---

## Expected Results

**Before:** Feet detected as balls ~5-10% of frames  
**After Phase 1 (config):** ~3-5% false positives  
**After Phase 2 (temporal):** ~1-2% false positives  
**After Phase 3 (retrain):** <0.5% false positives  

---

## Final Recommendation

**Do this now (15 minutes):**
1. Edit `ballSpotsProvider.cfg` - make thresholds stricter
2. Test in SimRobot with robot feet visible
3. Measure false positive reduction

**If that helps but not enough (2 hours):**
4. Add temporal filtering code

**For best results (1 week):**
5. Collect foot patches and retrain classifier

The combination of stricter config + temporal filtering should eliminate most foot false positives.

