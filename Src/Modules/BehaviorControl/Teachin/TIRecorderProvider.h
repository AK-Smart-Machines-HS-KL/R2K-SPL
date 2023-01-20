/**
 * @file TIRecorder.h
 * @author Andy Hobelsberger
 * @brief
 * @version 1.0
 * @date 2022-01-05
 *
 */

#pragma once
#include "Tools/Module/Module.h"
#include "Modules/BehaviorControl/BehaviorControl/BehaviorControl.h"

#include "Representations/BehaviorControl/TIRecorder.h"
#include "Representations/Infrastructure/GroundTruthWorldState.h"
#include "Representations/Modeling/ObstacleModel.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Configuration/FieldDimensions.h"

MODULE(TIRecorderProvider,
    { ,

      REQUIRES(FrameInfo),
      REQUIRES(FieldBall),
      REQUIRES(FieldDimensions),
      REQUIRES(GroundTruthWorldState),
      REQUIRES(GameInfo),
      REQUIRES(RobotInfo),
      REQUIRES(ObstacleModel),
      PROVIDES(TIRecorder),
      DEFINES_PARAMETERS(
      {,
        (int)(500) worldRecordInterval,
        (float)(10) secondsToRecord,
        (int)(1000) ballIsNearTreshold,
      }),
    });

class TIRecorderProvider : public TIRecorderProviderBase
{
public:
    TIRecorderProvider();
    ~TIRecorderProvider() = default;

private:
    void update(TIRecorder& record);

    void pushAction(playbackAction& action, TIRecorder& record);
    void start(TIRecorder& record);
    void stop(TIRecorder& record);
    void save(TIRecorder& record);
    WorldModel getWorldModel();

    RingBuffer<WorldModel> worldData;

    uint32_t lastWorldRecord = 0;
    uint32_t lastActionRecord = 0;
    int freeIdxOff = 0; // offset for determining the next free TI Index
};

void copyWorldBuf(RingBuffer<WorldModel>& from, std::vector<WorldModel>& to);

