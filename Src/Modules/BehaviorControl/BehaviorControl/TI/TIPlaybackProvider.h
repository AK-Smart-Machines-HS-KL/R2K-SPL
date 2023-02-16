/**
 * @file TIPlaybackProvider.cpp
 * @author Jonas Lambing (R2K)
 * @brief This file is responsible for loading teach-in related data such as worldmodels and playback files
 *
 * All of the data being loaded here is located in <BHuman Dir>/Config/TeachIn.
 *
 * Each worldmodel file has a corresponding playback file, which contains information
 * about the state of the simulator (ball position, bot position, team / opponent positions etc.).
 *
 * Playback and worldmodel files are loaded by iterating through sub-directories inside the TeachIn folder.
 * The names of the sub-directories are mnemonic prefixes as specified by the human operator while recording.
 *
 * Examples for prefixes: Testing, Passingchallenge, etc.
 *
 * The structure is as follows:
 *
 * - TeachIn
 *      - Testing
 *          playback.0001.csv
 *          worldmodel.0001.csv
 *          playback.0002.csv
 *          worldmodel.0002.csv
 *          ...
 *      - Passingchallenge
 *          playback.0001.csv
 *          worldmodel.0001.csv
 *          playback.0002.csv
 *          worldmodel.0002.csv
 *          ...
 *
 * Read and write operations check for validity of skillnames and format issues to prevent errors
 * by manual editing.
 *
 * @version 1.0
 * @date 2021-08-20
 *
 */

#pragma once
#include "Tools/Module/Module.h"
#include "Representations/BehaviorControl/TI/TIPlaybackData.h"
#include "Representations/Modeling/ObstacleModel.h"
#include "Representations/Communication/GameInfo.h"
#include "Representations/Communication/RobotInfo.h"
#include "Tools/RingBuffer.h"

MODULE(TIPlaybackProvider,
       {
           ,
           REQUIRES(GameInfo),
           REQUIRES(RobotInfo),
           REQUIRES(ObstacleModel),
           PROVIDES(TIPlaybackSequences),
       });

class TIPlaybackProvider : public TIPlaybackProviderBase
{
public:

    TIPlaybackProvider();

    void update(TIPlaybackSequences &worldModelPlayback) override;

    /**
     * @brief This function prints all loaded worldmodel and playback files
     */
    void printLoadedData(TIPlaybackSequences &worldModelPlayback);

    /**
     * @brief This function attempts to load all worldmodels and their corresponding playback files
     * @param worldModelPlayback The worldModelPlayback aggregates both structures
     */
    void loadTeachInData(TIPlaybackSequences &worldModelPlayback);

    /**
     * @brief This function checks if each worldmodel has a corresponding playback, if not the worldmodel is removed from the representation
     * @param worldModelPlayback The worldModelPlayback aggregates both structures
     */
    void enforceConsistency(TIPlaybackSequences &worldModelPlayback);

    /**
     * @brief This function loads a worldmodel file
     * @param worldModelPlayback The worldModelPlayback aggregates both structures
     * @param name The name of the file on disk
     * @param path The full path of the file on disk
     * @return true if the worldmodel was successfully loaded
     */
    bool loadWorldModel(TIPlaybackSequences &worldModelPlayback, std::string name, std::string path);

    /**
     * @brief This function loads a playback file
     * @param worldModelPlayback The worldModelPlayback aggregates both structures
     * @param name The name of the file on disk
     * @param path The full path of the file on disk
     * @return true if the playback was successfully loaded
     */
    bool loadPlayback(TIPlaybackSequences &worldModelPlayback, std::string name, std::string path);
};