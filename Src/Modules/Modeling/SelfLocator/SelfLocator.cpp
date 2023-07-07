/**
 * @file SelfLocator.cpp
 *
 * Implements a class that performs self-localization
 *
 * @author <a href="mailto:tlaue@uni-bremen.de">Tim Laue</a>
 */

#include "SelfLocator.h"
#include "Platform/SystemCall.h"
#include "Tools/Debugging/Annotation.h"
#include "Tools/Math/Probabilistics.h"
#include "Tools/Math/Eigen.h"
#include <algorithm>

using namespace std;

SelfLocator::SelfLocator() : lastTimeFarFieldBorderSeen(0), lastTimeJumpSound(0),
  timeOfLastReturnFromPenalty(0), lastTimeNotInStandWalkKick(0),
  nextSampleNumber(0), idOfLastBestSample(-1), averageWeighting(0.5f), lastAlternativePoseTimestamp(0),
  nextManualPlacementPoseNumber(0), lastTimePenaltyMarkSeen(0), lastTimeCirclePerceptSeen(0),
  validitiesHaveBeenUpdated(false)
{
  // Create sample set with samples at the typical walk-in positions
  samples = new SampleSet<UKFRobotPoseHypothesis>(numberOfSamples);
  for(int i = 0; i < samples->size(); ++i)
    samples->at(i).init(getNewPoseAtWalkInPosition(), walkInPoseDeviation, nextSampleNumber++, 0.5f);
}

SelfLocator::~SelfLocator()
{
  delete samples;
}

void SelfLocator::compileRobotPose(RobotPoseComponent * comp) {
  comp->pose.translation.x() = theRobotPose.translation.x();
  comp->pose.translation.y() = theRobotPose.translation.y();
  comp->pose.rotation = theRobotPose.rotation;
}

void SelfLocator::update(RobotPose& robotPose)
{
#ifndef NDEBUG
  ASSERT(!std::isnan(theOdometer.distanceWalked));
  ASSERT(!std::isnan(static_cast<float>(theOdometer.odometryOffset.rotation)));
  ASSERT(!std::isnan(theOdometer.odometryOffset.translation.x()));
  ASSERT(!std::isnan(theOdometer.odometryOffset.translation.y()));
  ASSERT(!std::isnan(theSideInformation.largestXCoordinatePossible));
  if(theFrameInfo.getTimeSince(lastTimeCirclePerceptSeen) < 10 && lastTimeCirclePerceptSeen != 0)
  {
    ASSERT(!std::isnan(theCirclePercept.pos.x()));
    ASSERT(!std::isnan(theCirclePercept.pos.y()));
  }
  if(thePenaltyMarkPercept.wasSeen)
  {
    ASSERT(!std::isnan(thePenaltyMarkPercept.positionOnField.x()));
    ASSERT(!std::isnan(thePenaltyMarkPercept.positionOnField.y()));
  }
  if(theFieldBoundary.isValid)
  {
    for(const Vector2f& v : theFieldBoundary.boundaryOnField)
    {
      ASSERT(!std::isnan(v.x()));
      ASSERT(!std::isnan(v.y()));
    }
  }
#endif

  // Register RobotPose Compiler
  if(!messageCompilerRef) {
    messageCompilerRef = RobotPoseComponent::onCompile.add(std::bind(&SelfLocator::compileRobotPose, this, _1));
  }

  /* Initialize variable(s) */
  sampleSetHasBeenReset = false;
  validitiesHaveBeenUpdated = false;
  inverseCameraMatrix = theCameraMatrix.inverse();
  currentRotationDeviation = theMotionInfo.executedPhase == MotionPhase::stand ? robotRotationDeviationInStand : robotRotationDeviation;
  if(thePenaltyMarkPercept.wasSeen)
    lastTimePenaltyMarkSeen = theFrameInfo.time;
  if(theCirclePercept.wasSeen)
    lastTimeCirclePerceptSeen = theFrameInfo.time;

  /* Modify sample set according to certain changes of the game state:
   *  - Reset mirror in SET
   *  - Handling of penalty positions
   *  - ...
   */
  handleGameStateChanges();

  /* Move all samples according to the current odometry.
   */
  STOPWATCH("SelfLocator:motionUpdate")
  {
    motionUpdate();
  }

  /* Integrate perceptions
   *  - goal posts, lines, corners ...
   * and compute validity of each sample
   */
  STOPWATCH("SelfLocator:sensorUpdate")
  {
    sensorUpdate();
  }

  /* Check and plot current weightings */
  float minWeighting = 2.f;
  float maxWeighting = -1.f;
  float weightingSum = 0.f;
  for(int i = 0; i < numberOfSamples; ++i)
  {
    samples->at(i).computeWeightingBasedOnValidity(baseValidityWeighting);
    const float w = samples->at(i).weighting;
    weightingSum += w;
    if(w > maxWeighting)
      maxWeighting = w;
    if(w < minWeighting)
      minWeighting = w;
  }
  averageWeighting = weightingSum / numberOfSamples;
  PLOT("module:SelfLocator:minWeighting", minWeighting);
  PLOT("module:SelfLocator:maxWeighting", maxWeighting);
  PLOT("module:SelfLocator:averageWeighting", averageWeighting);

  /** Handle information about a possibly flipped pose */
  handleSideInformation();

  /* Actually: Hacks and workaround :-)
   *  - Treat keeper rotation problem
   */
  domainSpecificSituationHandling();

  /* Particle filter resampling step
   *  - not executed during penalty shootout!
   *  - not executed for a few seconds after returning from a penalty
   *  - only executed if validities have changed this frame
   */
  if(validitiesHaveBeenUpdated &&
     theFrameInfo.getTimeSince(timeOfLastReturnFromPenalty) > 4000 &&
     theGameInfo.gamePhase != GAME_PHASE_PENALTYSHOOT)
    resampling();

  /* Fill the RobotPose representation based on the current sample set
   */
  computeModel(robotPose);

  /* Replace a sample, if necessary
   *   This step is done at the end to make sure that the new sample
   *   gets the full motion and sensor update steps before being involved
   *   in any pose computations.
   */
  if(sensorResetting(robotPose))
  {
    PLOT("module:SelfLocator:sampleResetting", 6.f);
  }
  else
  {
    PLOT("module:SelfLocator:sampleResetting", 0.f);
  }

  /* Finally, update internal variables, debug and draw stuff.
   */
  DEBUG_RESPONSE("module:SelfLocator:templates_only")
  {
    if(theAlternativeRobotPoseHypothesis.isValid)
    {
      for(int i = 0; i < numberOfSamples; ++i)
      {
        UKFRobotPoseHypothesis newSample;
        if(theSideInformation.robotMustBeInOwnHalf)
          newSample.init(getNewPoseBasedOnObservations(true, theWorldModelPrediction.robotPose), defaultPoseDeviation, nextSampleNumber++, 0.5f);
        else
          newSample.init(getNewPoseBasedOnObservations(false, theWorldModelPrediction.robotPose), defaultPoseDeviation, nextSampleNumber++, 0.5f);
        samples->at(i) = newSample;
      }
    }
  }

  if((theWorldModelPrediction.robotPose.translation - robotPose.translation).norm() > positionJumpNotificationDistance &&
     theGameInfo.state == STATE_PLAYING && !theSideInformation.mirror)
  {
    if(theFrameInfo.getTimeSince(lastTimeJumpSound) > 1337)
    {
      SystemCall::say("Jump");
      lastTimeJumpSound = theFrameInfo.time;
    }
    robotPose.timestampLastJump = theFrameInfo.time;
    ANNOTATION("SelfLocator", "Robot position has jumped!");
  }

  MODIFY("representation:RobotPose", robotPose);
  draw(robotPose);
  DEBUG_DRAWING("cognition:Odometry", "drawingOnField")
  {
    Pose2f origin = robotPose + theOdometryData.inverse();
    ORIGIN("cognition:Odometry", origin.translation.x(), origin.translation.y(), origin.rotation);
  }
}

void SelfLocator::update(SelfLocalizationHypotheses& selfLocalizationHypotheses)
{
  selfLocalizationHypotheses.hypotheses.resize(numberOfSamples);
  for(int i = 0; i < numberOfSamples; ++i)
  {
    SelfLocalizationHypotheses::Hypothesis& h = selfLocalizationHypotheses.hypotheses[i];
    h.pose = samples->at(i).getPose();
    h.validity = samples->at(i).validity;
    Matrix3f cov = samples->at(i).getCov();
    h.xVariance = cov(0, 0);
    h.yVariance = cov(1, 1);
    h.xyCovariance = cov(1, 0);
    h.rotVariance = cov(2, 2);
  }
}

void SelfLocator::computeModel(RobotPose& robotPose)
{
  UKFRobotPoseHypothesis& bestSample = getMostValidSample();
  Pose2f resultPose = bestSample.getPose();
  // Override side information for testing in the opponent half of a field only
  if(theSideInformation.robotMustBeInOpponentHalf && resultPose.translation.x() < 0) // TL: This appears a bit too simple. TODO: Make better.
  {
    resultPose = Pose2f(pi) + resultPose;
  }
  robotPose = resultPose;
  Matrix3f cov = bestSample.getCov();
  robotPose.covariance = cov;
  if(theSideInformation.mirror)
    robotPose.timestampLastJump = theFrameInfo.time;
  idOfLastBestSample = bestSample.id;
  // Finally, set the quality information:
  float validityOfBestHypothesis = bestSample.validity;
  setLocalizationQuality(robotPose, validityOfBestHypothesis);
}

void SelfLocator::setLocalizationQuality(RobotPose& robotPose, float validityOfBestHypothesis)
{
  // Overall, the probability distribution seems to be OK. Quality depends on deviation and validity:
  if(sampleSetIsUnimodal(robotPose))
  {
    const float translationalStandardDeviation = robotPose.getTranslationalStandardDeviation();
    const float rotationalStandardDeviation = std::sqrt(robotPose.covariance(2,2));
    if(validityOfBestHypothesis >= minValidityForSuperbLocalizationQuality &&
       translationalStandardDeviation < maxTranslationDeviationForSuperbLocalizationQuality &&
       rotationalStandardDeviation < maxRotationalDeviationForSuperbLocalizationQuality)
      robotPose.quality = RobotPose::superb;
    else
      robotPose.quality = RobotPose::okay;
  }
  // Meow, there are samples at different places (e.g. after returning from a penalty or
  // after some sensor resetting of samples (as the robot appeared to have a bad localization)
  else
  {
    robotPose.quality = RobotPose::poor;
  }
}

bool SelfLocator::sampleSetIsUnimodal(const RobotPose& robotPose)
{
  // This is just some kind of temporary hack.
  // However, I have discovered a truly convincing solution for this problem which this comment is too small to contain. TL
  const float maxDistanceDeviation = 2019.f;
  const Angle maxRotationDeviation(84_deg);
  const Angle robotPoseRotation(robotPose.rotation);
  const float sqrMaxDistanceDeviation = maxDistanceDeviation * maxDistanceDeviation;
  for(int i = 0; i < numberOfSamples; ++i)
  {
    const Pose2f& p = samples->at(i).getPose();
    if((robotPose.translation - p.translation).squaredNorm() > sqrMaxDistanceDeviation)
      return false;
    if(robotPoseRotation.diffAbs(Angle(p.rotation)) > maxRotationDeviation)
      return false;;
  }
  return true;
}

void SelfLocator::motionUpdate()
{
  // This is a nasty workaround but should help us in cases of bad/slow assistant referees:
  // If the robot returns from a penalty and is not walking, any rotation (that might come from
  // the z-axis gyro as the robot is turned too late!) is ignored!
  float odometryRotation = theOdometer.odometryOffset.rotation;
  if(theFrameInfo.getTimeSince(timeOfLastReturnFromPenalty) < 10000 && theMotionInfo.executedPhase != MotionPhase::walk)
    odometryRotation = 0.f;

  const float transX = theOdometer.odometryOffset.translation.x();
  const float transY = theOdometer.odometryOffset.translation.y();
  const float dist = theOdometer.odometryOffset.translation.norm();
  const float angle = abs(odometryRotation);
  const float angleWeightNoise = theMotionInfo.executedPhase == MotionPhase::walk ? movedAngleWeightRotationNoise : movedAngleWeightRotationNoiseNotWalking;

  // Precalculate rotational error that has to be adapted to all samples
  const float rotError = max(dist * movedDistWeightRotationNoise, angle * angleWeightNoise);

  // pre-calculate translational error that has to be adapted to all samples
  const float transXError = max(abs(transX * majorDirTransWeight), abs(transY * minorDirTransWeight));
  const float transYError = max(abs(transY * majorDirTransWeight), abs(transX * minorDirTransWeight));

  // update samples
  for(int i = 0; i < numberOfSamples; ++i)
  {
    const Vector2f transOffset((transX - transXError) + (2 * transXError) * Random::uniform(),
                               (transY - transYError) + (2 * transYError) * Random::uniform());
    const float rotationOffset = odometryRotation + Random::uniform(-rotError, rotError);

    samples->at(i).motionUpdate(Pose2f(rotationOffset, transOffset), filterProcessDeviation, odometryDeviation, odometryRotationDeviation);
  }
}

void SelfLocator::sensorUpdate()
{
  // Do not update during certain motions as the percepts might have a low quality
  if(currentMotionIsUnsafe())
    return;
  // In the penalty shootout, the goalkeeper should not perform any real localization
  if(theGameInfo.gamePhase == GAME_PHASE_PENALTYSHOOT && theGameInfo.kickingTeam != theOwnTeamInfo.teamNumber)
    return;

  // Perform integration of measurements:
  bool useLines = true;
  bool useLandmarks = true;
  bool usePoses = true;
  MODIFY("module:SelfLocator:useLines", useLines);
  MODIFY("module:SelfLocator:useLandmarks", useLandmarks);
  MODIFY("module:SelfLocator:usePoses", usePoses);
  std::vector<RegisteredAbsolutePoseMeasurement> absolutePoseMeasurements;
  std::vector<RegisteredLandmark> landmarks;
  std::vector<RegisteredLine> lines;
  for(int i = 0; i < numberOfSamples; ++i)
  {
    float numerator = 0.f;
    float denominator = 0.f;
    const Pose2f samplePose = samples->at(i).getPose();
    if(usePoses && thePerceptRegistration.totalNumberOfAvailableAbsolutePoseMeasurements > 0)
    {
      thePerceptRegistration.registerAbsolutePoseMeasurements(samplePose, absolutePoseMeasurements);
      for(const auto& pose : absolutePoseMeasurements)
        samples->at(i).updateByPose(pose, theCameraMatrix, inverseCameraMatrix, currentRotationDeviation, theFieldDimensions);
      numerator += validityFactorPoseMeasurement * (static_cast<float>(absolutePoseMeasurements.size()) / thePerceptRegistration.totalNumberOfAvailableAbsolutePoseMeasurements);
      denominator += validityFactorPoseMeasurement;
    }
    if(useLandmarks && thePerceptRegistration.totalNumberOfAvailableLandmarks > 0)
    {
      thePerceptRegistration.registerLandmarks(samplePose, landmarks);
      for(const auto& landmark : landmarks)
        samples->at(i).updateByLandmark(landmark);
      numerator += validityFactorLandmarkMeasurement * (static_cast<float>(landmarks.size()) / thePerceptRegistration.totalNumberOfAvailableLandmarks);
      denominator += validityFactorLandmarkMeasurement;
    }
    if(useLines && thePerceptRegistration.totalNumberOfAvailableLines > 0)
    {
      thePerceptRegistration.registerLines(samplePose, lines);
      for(const auto& line : lines)
      {
        if(line.partOfCenterCircle) // This is not a classic line and is thus treated as a different kind of measurement
          samples->at(i).updateByLineOnCenterCircle(line, theFieldDimensions.centerCircleRadius);
        else // Normal line
          samples->at(i).updateByLine(line);
      }
      if(considerLinesForValidityComputation)
      {
        int numberOfLinesForValidityComputation = thePerceptRegistration.totalNumberOfAvailableLines - thePerceptRegistration.totalNumberOfIgnoredLines;
        if(numberOfLinesForValidityComputation > 0)
        {
          numerator += validityFactorLineMeasurement * static_cast<float>(lines.size()) / numberOfLinesForValidityComputation;
          denominator += validityFactorLineMeasurement;
        }
      }
    }
    // Update validities, if any features have been observed (no matter, if they have actually been used):
    if(denominator != 0.f)
    {
      const float currentValidity = numerator / denominator;
      samples->at(i).updateValidity(numberOfConsideredFramesForValidity, currentValidity);
      validitiesHaveBeenUpdated = true;
    }
  }

  // Apply side information:
  if(theGameInfo.gamePhase != GAME_PHASE_PENALTYSHOOT)
  {
    for(int i = 0; i < numberOfSamples; ++i)
    {
      if(samples->at(i).getPose().translation.x() > theSideInformation.largestXCoordinatePossible)
        samples->at(i).invalidate();
    }
  }

  // Check, if sample is still on the carpet
  for(int i = 0; i < numberOfSamples; ++i)
  {
    const Vector2f& position = samples->at(i).getPose().translation;
    if(!theFieldDimensions.isInsideCarpet(position))
      samples->at(i).invalidate();
  }
}

bool SelfLocator::currentMotionIsUnsafe()
{
  // Walking and standing is OK. Everything else (kicking, falling, getting up, ...) probably not.
  if(!theMotionInfo.isMotion(bit(MotionPhase::walk) | bit(MotionPhase::stand)))
    return true;
  // If the robot turns too fast, things go wrong.
  // TODO: Move hardcoded number to parameter. Values is guessed by Philip.
  if(std::abs(theGyroState.mean.z()) > 50_deg)
    return true;
  return false;
}

bool SelfLocator::sensorResetting(const RobotPose& robotPose)
{
  if(theGameInfo.gamePhase == GAME_PHASE_PENALTYSHOOT) // Don't replace samples in penalty shootout
    return false;
  if(theSideInformation.mirror)                          // Don't replace samples in mirror cycle
    return false;
  if(timeOfLastReturnFromPenalty != 0 && theFrameInfo.getTimeSince(timeOfLastReturnFromPenalty) < 7000)
    return false;
  if(theFallDownState.state != FallDownState::upright
     && theFallDownState.state != FallDownState::squatting)  // Don't replace samples, if robot is not upright (e.g. lying or staggering)
    return false;
  if(sampleSetHasBeenReset)
    return false;

  // Can we perform the resetting of a sample?
  if(theAlternativeRobotPoseHypothesis.isValid &&
     theAlternativeRobotPoseHypothesis.numOfContributingObservations >= minNumberOfObservationsForResetting &&
     theAlternativeRobotPoseHypothesis.timeOfLastPerceptionUpdate > lastAlternativePoseTimestamp)
  {
    // Should we perform the resetting of a sample?
    bool alternativeIsCompatibleToRobotPose = alternativeRobotPoseIsCompatibleToPose(robotPose);
    if(theAlternativeRobotPoseHypothesis.isInOwnHalf)
    {
      if(alternativeIsCompatibleToRobotPose)
        return false; // Everything is OK, no reseting required
    }
    else // mirrored alternative has also to be considered
    {
      const Pose2f mirrorPose = Pose2f(pi) + robotPose;
      bool mirrorIsCompatibleToRobotPose = alternativeRobotPoseIsCompatibleToPose(mirrorPose);
      if(mirrorIsCompatibleToRobotPose || alternativeIsCompatibleToRobotPose)
        return false; // Everything is OK, no resetting required
    }
    // Resetting seems to be required:
    float resettingValidity = max(0.5f, averageWeighting); // TODO: Recompute?
    int worstSampleIdx = 0;
    float worstSampleValidity = samples->at(0).validity;
    for(int i = 1; i < numberOfSamples; ++i)
    {
      if(samples->at(i).validity < worstSampleValidity)
      {
        worstSampleIdx = i;
        worstSampleValidity = samples->at(i).validity;
      }
    }
    UKFRobotPoseHypothesis newSample;
    if(theSideInformation.robotMustBeInOwnHalf)
      newSample.init(getNewPoseBasedOnObservations(true, theWorldModelPrediction.robotPose), defaultPoseDeviation, nextSampleNumber++, resettingValidity);
    else
      newSample.init(getNewPoseBasedOnObservations(false, theWorldModelPrediction.robotPose), defaultPoseDeviation, nextSampleNumber++, resettingValidity);
    samples->at(worstSampleIdx) = newSample;
    lastAlternativePoseTimestamp = theAlternativeRobotPoseHypothesis.timeOfLastPerceptionUpdate;
    return true;
  }
  return false;
}

bool SelfLocator::alternativeRobotPoseIsCompatibleToPose(const Pose2f& robotPose)
{
  const float xDev = std::abs(robotPose.translation.x() - theAlternativeRobotPoseHypothesis.pose.translation.x());
  const float yDev = std::abs(robotPose.translation.y() - theAlternativeRobotPoseHypothesis.pose.translation.y());
  const float rDev = std::abs(Angle::normalize(robotPose.rotation - theAlternativeRobotPoseHypothesis.pose.rotation));
  return xDev < translationalDeviationForResetting && yDev < translationalDeviationForResetting &&
         rDev < rotationalDeviationForResetting;
}

void SelfLocator::resampling()
{
  if(averageWeighting == 0.f)
    return;
  // actual resampling step:
  UKFRobotPoseHypothesis* oldSet = samples->swap();
  const float weightingBetweenTwoDrawnSamples = averageWeighting;
  float nextPos(Random::uniform() * weightingBetweenTwoDrawnSamples);
  float currentSum(0);

  // resample:
  int replacements(0);
  int j(0);
  for(int i = 0; i < numberOfSamples; ++i)
  {
    currentSum += oldSet[i].weighting;
    int replicationCount(0);
    while(currentSum > nextPos && j < numberOfSamples)
    {
      samples->at(j) = oldSet[i];
      if(replicationCount) // An old sample becomes copied multiple times: we need new identifier for the new instances
      {
        samples->at(j).id = nextSampleNumber++;
        replacements++;
      }
      replicationCount++;
      j++;
      nextPos += weightingBetweenTwoDrawnSamples;
    }
  }
  int missingSamples = numberOfSamples - j;
  // fill up missing samples (could happen in rare cases due to numerical imprecision / rounding / whatever) with new poses:
  for(; j < numberOfSamples; ++j)
  {
    if(theAlternativeRobotPoseHypothesis.isValid) // Try to use the currently best available alternative
    {
      const Pose2f pose = getNewPoseBasedOnObservations(false, theWorldModelPrediction.robotPose);
      samples->at(j).init(pose, defaultPoseDeviation, nextSampleNumber++, averageWeighting);
      ANNOTATION("SelfLocator", "Missing sample was replaced by alternative hypothesis! Current number of samples: " << j);
    }
    else if(j > 0) // if no alternative is available, just use the first sample
    {
      const Pose2f pose = samples->at(0).getPose();
      samples->at(j).init(pose, defaultPoseDeviation, nextSampleNumber++, averageWeighting);
      ANNOTATION("SelfLocator", "Missing sample was replaced by sample #0! Current number of samples: " << j);
    }
    else
    {
      ASSERT(false); // <- If we reach this line, we have a serious problem. Should not happen ;-)
    }
  }
  ASSERT(allSamplesIDsAreUnique());
  PLOT("module:SelfLocator:missingSamples", missingSamples);
  PLOT("module:SelfLocator:sampleReplacements", replacements);
}

void SelfLocator::handleSideInformation()
{
  if(!theSideInformation.mirror || sampleSetHasBeenReset)
    return;
  for(int i = 0; i < numberOfSamples; ++i)
  {
    samples->at(i).mirror();
  }
  ANNOTATION("SelfLocator", "Mirrrrrrooaaaarred!");
}

void SelfLocator::handleGameStateChanges()
{
  if(theGameInfo.gamePhase == GAME_PHASE_PENALTYSHOOT)
  {
    // penalty shoot: if game state switched to playing reset samples to start position
    if((theExtendedGameInfo.gameStateLastFrame != STATE_PLAYING && theGameInfo.state == STATE_PLAYING) ||
       (theExtendedGameInfo.penaltyLastFrame != PENALTY_NONE && theRobotInfo.penalty == PENALTY_NONE))
    {
      for(int i = 0; i < samples->size(); ++i)
        samples->at(i).init(getNewPoseAtPenaltyShootoutPosition(), penaltyShootoutPoseDeviation, nextSampleNumber++, 1.f);
      sampleSetHasBeenReset = true;
    }
  }
  // If the robot has been lifted during SET, reset samples to manual positioning line positions
  else if(theExtendedGameInfo.manuallyPlaced && theGameInfo.state == STATE_SET)
  {
    for(int i = 0; i < samples->size(); ++i)
    {
      samples->at(i).init(getNewPoseAtManualPlacementPosition(), manualPlacementPoseDeviation, nextSampleNumber++, 0.5f);
    }
    sampleSetHasBeenReset = true;
    timeOfLastReturnFromPenalty = theFrameInfo.time;
  }
  // If a penalty is over, reset samples to reenter positions
  else if(theExtendedGameInfo.returnFromGameControllerPenalty || theExtendedGameInfo.returnFromManualPenalty || theExtendedGameInfo.startingCalibration)
  {
    int startOfSecondHalfOfSampleSet = samples->size() / 2;
    // The first half of the new sample set is left of the own goal ...
    for(int i = 0; i < startOfSecondHalfOfSampleSet; ++i)
      samples->at(i).init(getNewPoseReturnFromPenaltyPosition(true), returnFromPenaltyPoseDeviation, nextSampleNumber++, 0.5f);
    // ... and the second half of new sample set is right of the own goal.
    for(int i = startOfSecondHalfOfSampleSet; i < samples->size(); ++i)
      samples->at(i).init(getNewPoseReturnFromPenaltyPosition(false), returnFromPenaltyPoseDeviation, nextSampleNumber++, 0.5f);
    sampleSetHasBeenReset = true;
    timeOfLastReturnFromPenalty = theFrameInfo.time;
  }
  // Normal game is about to start: We start on the sidelines looking at our goal: (this is for checking in TeamCom)
  else if((theExtendedGameInfo.gameStateLastFrame != STATE_INITIAL && theGameInfo.state == STATE_INITIAL) ||
          // Normal game really starts: We start on the sidelines looking at our goal: (this is for actual setup)
          (theExtendedGameInfo.gameStateLastFrame == STATE_INITIAL && theGameInfo.state == STATE_READY))
  {
    for(int i = 0; i < samples->size(); ++i)
      samples->at(i).init(getNewPoseAtWalkInPosition(), walkInPoseDeviation, nextSampleNumber++, 0.5f);
    sampleSetHasBeenReset = true;
  }
  /* For testing purposes in simulator */
  else if(theStaticInitialPose.isActive && theStaticInitialPose.jump)
  {
    for(int i = 0; i < samples->size(); ++i)
      samples->at(i).init(theStaticInitialPose.staticPoseOnField, manualPlacementPoseDeviation, nextSampleNumber++, 0.5f);
    sampleSetHasBeenReset = true;
  }
  if(sampleSetHasBeenReset)
  {
    idOfLastBestSample = -1;
  }
}

UKFRobotPoseHypothesis& SelfLocator::getMostValidSample()
{
  float validityOfLastBestSample = -1.f;
  UKFRobotPoseHypothesis* lastBestSample = 0;
  if(idOfLastBestSample != -1)
  {
    for(int i = 0; i < numberOfSamples; ++i)
    {
      if(samples->at(i).id == idOfLastBestSample)
      {
        validityOfLastBestSample = samples->at(i).validity;
        lastBestSample = &(samples->at(i));
        break;
      }
    }
  }
  UKFRobotPoseHypothesis* returnSample = &(samples->at(0));
  float maxValidity = -1.f;
  float minVariance = 0.f; // Initial value does not matter
  for(int i = 0; i < numberOfSamples; ++i)
  {
    const float val = samples->at(i).validity;
    if(val > maxValidity)
    {
      maxValidity = val;
      minVariance = samples->at(i).getCombinedVariance();
      returnSample = &(samples->at(i));
    }
    else if(val == maxValidity)
    {
      float variance = samples->at(i).getCombinedVariance();
      if(variance < minVariance)
      {
        maxValidity = val;
        minVariance = variance;
        returnSample = &(samples->at(i));
      }
    }
  }
  if(lastBestSample && returnSample->validity <= validityOfLastBestSample * 1.1f) // Bonus for stability
    return *lastBestSample;
  else
    return *returnSample;
}

void SelfLocator::domainSpecificSituationHandling()
{
  // Keep track of the time during which the robot was in "normal" and stable operation, i.e. not jumping or getting up or stuff like that
  if(theMotionInfo.executedPhase != MotionPhase::stand && theMotionInfo.executedPhase != MotionPhase::walk && theMotionInfo.executedPhase != MotionPhase::kick)
    lastTimeNotInStandWalkKick = theFrameInfo.time;

  // Calculate distance to furthest point on field boundary
  float maxDistance = 0.f;
  for(const Vector2f& p : theFieldBoundary.boundaryOnField)
  {
    if(p.norm() > maxDistance)
      maxDistance = p.norm();
  }
  if(maxDistance > goalieFieldBorderDistanceThreshold)
    lastTimeFarFieldBorderSeen = theFrameInfo.time;

  // Currently, this method only handles the case that the
  // keeper is turned by 180 degrees but assumes to stand correctly
  if(!goalieActivateTwistHandling ||
     !theRobotInfo.isGoalkeeper() ||
     theGameInfo.state != STATE_PLAYING ||
     theGameInfo.gamePhase == GAME_PHASE_PENALTYSHOOT)
    return;
  // The robot is in its goal area and assumes to look at the opponent half
  // and guards its goal.
  if(theRobotPose.translation.x() < theFieldDimensions.xPosOwnGoalArea + 200 &&
     abs(theRobotPose.translation.y()) < theFieldDimensions.yPosLeftGoalArea &&
     abs(theRobotPose.rotation) < 45_deg &&
     theFrameInfo.getTimeSince(lastTimeNotInStandWalkKick) > goalieTwistNoStandWalkKickTimeout &&
     theFieldBoundary.isValid)
  {
    // It has not seen important stuff for a long time but the field border appears
    // to be close. This means that it is probably twisted:
    if(theFrameInfo.getTimeSince(theBallModel.timeWhenLastSeen) > goalieNoPerceptsThreshold &&
       lastTimeCirclePerceptSeen != 0 && lastTimePenaltyMarkSeen != 0 &&
       theFrameInfo.getTimeSince(lastTimeCirclePerceptSeen) > goalieNoPerceptsThreshold &&
       theFrameInfo.getTimeSince(lastTimePenaltyMarkSeen) > goalieNoPerceptsThreshold &&
       theFrameInfo.getTimeSince(theFieldFeatureOverview.combinedStatus.lastSeen) > goalieNoPerceptsThreshold &&
       lastTimeFarFieldBorderSeen != 0 &&
       theFrameInfo.getTimeSince(lastTimeFarFieldBorderSeen) > goalieNoFarFieldBorderThreshold)
    {
      ANNOTATION("SelfLocator", "Goalie Twist!");
      for(int i = 0; i < numberOfSamples; ++i)
        samples->at(i).twist();
    }
  }
}

void SelfLocator::draw(const RobotPose& robotPose)
{
  // Visualizes effect of current function's parameters
  DEBUG_DRAWING("module:SelfLocator:isMirrorCloser", "drawingOnField")
  {
    const float stepSize = 500.f;
    const float degStepSize = 30.f;
    const float length = stepSize / 3.f;
    for(float x = theFieldDimensions.xPosOwnFieldBorder; x <= theFieldDimensions.xPosOpponentFieldBorder; x += stepSize)
    {
      for(float y = theFieldDimensions.yPosRightFieldBorder; y <= theFieldDimensions.yPosLeftFieldBorder; y += stepSize)
      {
        for(float rot = 0.f; rot < 360.f; rot += degStepSize)
        {
          Pose2f samplePose(Angle::fromDegrees(rot), x, y);
          ColorRGBA col = isMirrorCloser(samplePose, robotPose) ? ColorRGBA(255, 0, 0) : ColorRGBA(0, 0, 0);
          Vector2f vec(length, 0.f);
          vec.rotate(Angle::fromDegrees(rot));
          LINE("module:SelfLocator:isMirrorCloser", x, y, x + vec.x(), y + vec.y(), 20, Drawings::solidPen, col);
        }
      }
    }
  }
  // Visualize hypothetical percept registration for the current pose
//  std::vector<RegisteredAbsolutePoseMeasurement> absolutePoseMeasurements; TODO: Draw
  std::vector<RegisteredLandmark> landmarks;
  std::vector<RegisteredLine> lines;
//  thePerceptRegistration.registerAbsolutePoseMeasurements(robotPose, absolutePoseMeasurements); TODO: Draw
  thePerceptRegistration.registerLandmarks(robotPose, landmarks);
  thePerceptRegistration.registerLines(robotPose, lines);

  DEBUG_DRAWING("module:SelfLocator:perceptRegistrationWorld", "drawingOnField")
  {
    THREAD("module:SelfLocator:perceptRegistrationWorld", theCameraInfo.camera == CameraInfo::upper ? "Upper" : "Lower");
    for(const auto& line : lines)
    {
      const ColorRGBA col = line.partOfCenterCircle ? ColorRGBA(128, 255, 0, 180) : ColorRGBA(128, 0, 255, 180);
      // Associated model line in absolute field coordinates:
      LINE("module:SelfLocator:perceptRegistrationWorld", line.modelStart.x(), line.modelStart.y(), line.modelEnd.x(), line.modelEnd.y(),
           80, Drawings::solidPen, col);
    }
    for(const auto& landmark : landmarks)
    {
      // Associated landmark position in absolute field coordinates:
      CIRCLE("module:SelfLocator:perceptRegistrationWorld", landmark.model.x(), landmark.model.y(),
             200, 20, Drawings::noPen, ColorRGBA::blue, Drawings::solidPen, ColorRGBA::blue);
    }
  }
  DEBUG_DRAWING("module:SelfLocator:perceptRegistrationRobot", "drawingOnField")
  {
    THREAD("module:SelfLocator:perceptRegistrationRobot", theCameraInfo.camera == CameraInfo::upper ? "Upper" : "Lower");
    for(const auto& line : lines)
    {
      // Covariance of center of field line (has to be drawn relative to the robot)
      COVARIANCE_ELLIPSES_2D("module:SelfLocator:perceptRegistrationRobot", line.covPerceptCenter, line.perceptCenter);
    }
    ColorRGBA c99(255,255,100,100);
    ColorRGBA c95(255,128,50,100);
    ColorRGBA c68(255,100,100,100);
    for(const auto& landmark : landmarks)
    {
      // Drawing of covariance (has to be drawn relative to the robot)
      COVARIANCE_ELLIPSES_2D_OWN_COLORS("module:SelfLocator:perceptRegistrationRobot", landmark.covPercept, landmark.percept, c99, c95, c68);
    }
  }
}

bool SelfLocator::allSamplesIDsAreUnique()
{
  for(int i = 0; i < numberOfSamples - 1; ++i)
  {
    for(int j = i + 1; j < numberOfSamples; ++j)
    {
      if(samples->at(i).id == samples->at(j).id)
        return false;
    }
  }
  return true;
}

Pose2f SelfLocator::getNewPoseBasedOnObservations(bool forceOwnHalf, const Pose2f& lastRobotPose) const
{
  const Pose2f& p = theAlternativeRobotPoseHypothesis.pose;
  if(forceOwnHalf)
  {
    if(p.translation.x() <= 0.f)
      return p;
    else
      return Pose2f(pi) + p;
  }
  else
  {
    if(isMirrorCloser(p, lastRobotPose))
      return Pose2f(pi) + p;
    else
      return p;
  }
}

Pose2f SelfLocator::getNewPoseReturnFromPenaltyPosition(bool leftSideOfGoal)
{
  // Special stuff for some demos:
  if(demoUseCustomReturnFromPenaltyPoses)
  {
    if(theRobotInfo.isGoalkeeper())
      return demoCustomReturnFromPenaltyPoseGoalie;
    else
      return demoCustomReturnFromPenaltyPoseFieldPlayer;
  }
  // Testing purposes: Static start position
  if(theStaticInitialPose.isActive && (theExtendedGameInfo.penaltyLastFrame == PENALTY_MANUAL || theExtendedGameInfo.penaltyLastFrame == PENALTY_SPL_PLAYER_PUSHING))
  {
    return theStaticInitialPose.staticPoseOnField;
  }
  // Normal stuff:
  float xPosition = Random::triangular(theFieldDimensions.xPosOwnPenaltyMark - returnFromPenaltyMaxXOffset,
                                       theFieldDimensions.xPosOwnPenaltyMark,
                                       theFieldDimensions.xPosOwnPenaltyMark + returnFromPenaltyMaxXOffset);
  if(leftSideOfGoal)
    return Pose2f(-pi_2, xPosition, theFieldDimensions.yPosLeftSideline);
  else
    return Pose2f(pi_2, xPosition, theFieldDimensions.yPosRightSideline);
}

Pose2f SelfLocator::getNewPoseAtWalkInPosition()
{
  const int effectivePlayerNumber = theOwnTeamInfo.getSubstitutedPlayerNumber(theRobotInfo.number);
  if(effectivePlayerNumber >= 1 && effectivePlayerNumber <= 6)
  {
    const SetupPoses::SetupPose& p = theSetupPoses.getPoseOfRobot(effectivePlayerNumber);
    Pose2f result;
    result.translation = p.position;
    result.rotation    = (p.turnedTowards - p.position).angle();
    return result;
  }
  else
  {
    ASSERT(false);   // This point will be reached as soon as we have higher numbers -> The assert will remind us that work has to be done ;-)
    return Pose2f(); // Dummy return
  }
}

Pose2f SelfLocator::getNewPoseAtManualPlacementPosition()
{
  // Goalie
  if(theRobotInfo.isGoalkeeper())
  {
    return Pose2f(0.f, theFieldDimensions.xPosOwnGroundLine + 52.f, 0.f);
  }
  else
  {
    Pose2f result;
    switch(nextManualPlacementPoseNumber)
    {
      case 0:
        result = Pose2f(0.f, theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber ? -theFieldDimensions.centerCircleRadius - 127.f : theFieldDimensions.xPosOwnPenaltyArea + 127.f, 0.f);
        break;
      case 1:
        result = Pose2f(0.f, theFieldDimensions.xPosOwnPenaltyMark, 0.f);
        break;
      case 2:
        result = Pose2f(0.f, theFieldDimensions.xPosOwnPenaltyMark, (theFieldDimensions.yPosLeftPenaltyArea + theFieldDimensions.yPosLeftSideline) / 2.f);
        break;
      case 3:
        result = Pose2f(0.f, theFieldDimensions.xPosOwnPenaltyMark, (theFieldDimensions.yPosRightPenaltyArea + theFieldDimensions.yPosRightSideline) / 2.f);
        break;
    }
    nextManualPlacementPoseNumber = (nextManualPlacementPoseNumber + 1) % 4;
    return result;
  }
}

Pose2f SelfLocator::getNewPoseAtPenaltyShootoutPosition()
{
  if(theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber)
  {
    //striker pose (edge of penalty area, looking towards opponent goal)
    return Pose2f(0.f, theFieldDimensions.xPosPenaltyStrikerStartPosition, 0.f);
  }
  else
  {
    //goalie pose (in the center of the goal, looking towards the field's center)
    return Pose2f(0.f, theFieldDimensions.xPosOwnGroundLine, 0.f);
  }
}

bool SelfLocator::isMirrorCloser(const Pose2f& currentPose, const Pose2f& lastPose) const
{
  const Vector2f& translation = currentPose.translation;
  Vector2f rotationWeight(std::max(theFieldDimensions.yPosLeftSideline * 1.1f - std::min(std::abs(translation.x()), std::abs(lastPose.translation.x())), 0.f), 0);
  Vector2f opponentGoal(theFieldDimensions.xPosOpponentGoalPost, 0.f);
  const Vector2f rotation = Pose2f(Geometry::angleTo(currentPose, opponentGoal)) * rotationWeight;
  const Vector2f lastRotation = Pose2f(Geometry::angleTo(lastPose, opponentGoal)) * rotationWeight;
  bool result = (lastPose.translation - translation).norm() + (lastRotation - rotation).norm() >
                (lastPose.translation + translation).norm() + (lastRotation + rotation).norm();
  return result;
}

MAKE_MODULE(SelfLocator, modeling);
