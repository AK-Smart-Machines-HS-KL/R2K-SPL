/**
 * @file TIRecorder.cpp
 * @author Andy Hobelsberger
 * @brief
 * @version 1.0
 * @date 2022-01-05
 *
 */

#include "TIRecorderProvider.h"
#include "Tools/Math/Geometry.h"
#include <Tools/rapidcsv/rapidcsv.h>
#include "Platform/File.h"
#include <sys/types.h>
#include <sys/stat.h>

const std::string TIRecordingDir = TI_Directory + "Recording/";

MAKE_MODULE(TIRecorderProvider, behaviorControl);

TIRecorderProvider::TIRecorderProvider() {
    int bufSize = (int)(secondsToRecord / ((float)worldRecordInterval / 1000.f));
    worldData = RingBuffer<WorldModel>(bufSize);

    struct stat info;
    ASSERT(stat(TIRecordingDir.c_str(), &info) == 0); // ensure that TIRecordingDir exists
    ASSERT(info.st_mode & S_IFDIR); // ensure that TIRecordingDir is a Directory
}

void TIRecorderProvider::update(TIRecorder& record) {
    if (!record.start) {
        record.start = [&]() {start(record); };
        record.stop = [&]() {stop(record); };
        record.changeAction = [&](playbackAction& action) {pushAction(action, record); };
        record.save = [&]() {save(record); };
    }

    if (!record.recording)
    {
        if (theFrameInfo.getTimeSince(lastWorldRecord) > worldRecordInterval) {
            worldData.push_front(std::move(getWorldModel()));
            if (lastWorldRecord != 0) {
                lastWorldRecord += worldRecordInterval; // ensures that record interval is kept 
            }
            else {
                lastWorldRecord = theFrameInfo.time; // first time setup
            }
        }
    }
}

void TIRecorderProvider::pushAction(playbackAction& action, TIRecorder& record) {
    if (!record.data.actions.empty()) {
        record.data.actions.back().maxTime = theFrameInfo.getTimeSince(lastActionRecord);
    }
    lastActionRecord = theFrameInfo.time;
    record.data.actions.push_back(action);
}

void TIRecorderProvider::start(TIRecorder& record) {
    record.clear();
    copyWorldBuf(worldData, record.worldData.models);
    record.recording = true;
    OUTPUT_TEXT("Recording Started");
}

void TIRecorderProvider::stop(TIRecorder& record) {
    record.data.actions.back().maxTime = theFrameInfo.getTimeSince(lastActionRecord);
    record.recording = false;
    OUTPUT_TEXT("Recording Stopped");
}

void TIRecorderProvider::save(TIRecorder& record) {
    if (!record.recording) {

        if (record.data.actions.empty()) {
            OUTPUT_TEXT("Can not save. No Actions recorded yet!");
            return;
        }
        else if (record.worldData.models.empty()) {
            OUTPUT_TEXT("Can not save. No World Data recorded!");
            return;
        }

        struct stat info;
        uint32_t idx = 0;
        std::string worldPath;
        std::string playbackPath;

        do {
            worldPath = TIRecordingDir + "worldmodel." + std::to_string(idx) + ".csv";
            playbackPath = TIRecordingDir + "playback." + std::to_string(idx) + ".csv";
            idx++;
        } while (
            stat(playbackPath.c_str(), &info) == 0 // playback file already exists 
            || stat(worldPath.c_str(), &info) == 0 // worldmodel file already exists
            );

        record.worldData.save(worldPath);
        record.data.save(playbackPath);
        record.clear();
    }
    else { OUTPUT_TEXT("Stop The recording before saving!"); }
}

WorldModel TIRecorderProvider::getWorldModel() {
    WorldModel model{};

    model.robotPose = Pose2f(theGroundTruthWorldState.ownPose);
    model.timeLeft = theGameInfo.secsRemaining;
    model.robotNumber = theRobotInfo.number;
    model.setPlay = theGameInfo.getPlay();
    model.gameState = theGameInfo.getState();
    model.ballPosition = theGroundTruthWorldState.balls.size() == 0 ?
        Vector2f(0, 0) :
        Vector2f(theGroundTruthWorldState.balls[0].position.x(), theGroundTruthWorldState.balls[0].position.y()); /* */
    model.ballIsNear = ballIsNearTreshold > Geometry::distance(model.robotPose.translation, model.ballPosition);
    model.ballDistanceToGoal = Geometry::distance(Vector2f(theFieldDimensions.xPosOpponentGoalPost, theFieldDimensions.yPosCenterGoal), model.ballPosition);
    model.ballDistanceToBot = Geometry::distance(model.robotPose.translation, model.ballPosition);
    model.distanceToGoal = Geometry::distance(model.robotPose.translation, Vector2f(theFieldDimensions.xPosOpponentGoalPost, theFieldDimensions.yPosCenterGoal));
    for (auto teamplayer : theGroundTruthWorldState.firstTeamPlayers) {
        model.teamList.push_back(teamplayer.pose.translation);
    }

    for (auto opponent : theGroundTruthWorldState.secondTeamPlayers) {
        model.opponentList.push_back(opponent.pose.translation);
    }

    return model;
}

void copyWorldBuf(RingBuffer<WorldModel>& from, std::vector<WorldModel>& to) {
    to.reserve(from.capacity());
    to.clear();
    for (auto& model : from)
    {
        to.push_back(model);
    }
}