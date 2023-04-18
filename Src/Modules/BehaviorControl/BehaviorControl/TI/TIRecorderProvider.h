/**
 * @file TIRecordingProvider.cpp
 * @author Wilhelm Simus (R2K)
 * @brief This file is responsible for recording teach-in related data such as worldmodels and playback files
 *
 * @version 1.0
 * @date 2023-04-18
 *
 */

#pragma once
#include "Tools/Module/Module.h"
#include "Modules/BehaviorControl/BehaviorControl/BehaviorControl.h"
#include "Representations/BehaviorControl/TI/TIRecorderData.h"
#include "Representations/Infrastructure/GroundTruthWorldState.h"
#include "Representations/Modeling/ObstacleModel.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/Configuration/FieldDimensions.h"

MODULE(TIRecorderProvider,
{,

  REQUIRES(FrameInfo),
  REQUIRES(FieldBall),
  REQUIRES(FieldDimensions),
  REQUIRES(GroundTruthWorldState),
  REQUIRES(GameInfo),
  REQUIRES(RobotInfo),
  REQUIRES(ObstacleModel),
  PROVIDES(TIRecorderData),
  DEFINES_PARAMETERS(
  {,
    (int) (500) worldRecordInterval,
    (float) (10) secondsToRecord,
    (int) (1000) ballIsNearTreshold,
  }),
});

class TIRecorderProvider : public TIRecorderProviderBase
{
  public:
  TIRecorderProvider();
  ~TIRecorderProvider() = default;

  private:
  void update(TIRecorderData& record);
  
  void pushAction(PlaybackAction& action, TIRecorderData& record);
  void start(TIRecorderData& record);
  void stop(TIRecorderData& record);
  void save(TIRecorderData& record);
  WorldModel getWorldModel();

  RingBuffer<WorldModel> worldData;

  uint32_t lastWorldRecord = 0;
  uint32_t lastActionRecord = 0;
  int freeIdxOff = 0; // offset for determining the next free TI Index
};

void copyWorldBuf(RingBuffer<WorldModel>& from, std::vector<WorldModel>& to);
