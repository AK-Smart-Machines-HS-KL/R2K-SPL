#include "TIPlaybackProvider.h"
#include "Modules/BehaviorControl/BehaviorControl/BehaviorControl.h"
#include "Platform/File.h"
#include <iostream>
#include <fstream>
#include <Tools/rapidcsv/rapidcsv.h>
#include "Tools/RingBuffer.h"
#include "Tools/Math/Geometry.h"

MAKE_MODULE(TIPlaybackProvider, behaviorControl);

bool isNumber(const std::string& str)
{
    for (char const& c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}

/**
 * This function paste the ring buffer into a .csv file format with rapidcsv library.
 * Then it save the file below the given path
 * Throws error if the path is not found
 * Shoud return true if everything went well
 */

void TIPlaybackProvider::update(TIPlayback& worldModelPlayback)
{
    // dr loadTeachInData
    DEBUG_RESPONSE_ONCE("loadTeachInData")
    {
        loadTeachInData(worldModelPlayback);
        enforceConsistency(worldModelPlayback);
        printLoadedData(worldModelPlayback);
    }
}

void TIPlaybackProvider::printLoadedData(TIPlayback& worldModelPlayback)
{
    OUTPUT_TEXT("Worldmodel:");

    // Print the names of all loaded worldmodels to the console
    for (WorldData model : worldModelPlayback.models)
    {
        //OUTPUT_TEXT(model.fileName);
    }

    OUTPUT_TEXT("-------------");
    OUTPUT_TEXT("Playback:");

    // Print the names of all loaded playbacks to the console
    for (PlaybackData data : worldModelPlayback.data)
    {
        OUTPUT_TEXT(data.fileName);
    }
}

void TIPlaybackProvider::loadTeachInData(TIPlayback& worldModelPlayback)
{
    // Get all sub-directories inside the TeachIn directory
    std::string teachInDir = std::string(File::getBHDir()) + "/Config/TeachIn/";
    std::list<std::string> subDirs = File::getSubDirs(teachInDir);

    for (std::string dir : subDirs)
    {
        // Get a list of all files inside each directory
        std::list<std::string> files = File::getFiles(teachInDir + dir);
        for (std::string file : files)
        {
            std::string name = dir + "/" + file;
            OUTPUT_TEXT("Loading: " + name);

            std::string fullPath = teachInDir + name;

            // Determine if the csv is a worldmodel or playback file
            bool isPlayback = (file.find("worldmodel") == std::string::npos);

            // Attempt to load and parse each csv
            bool wasLoaded = isPlayback ?
                loadPlayback(worldModelPlayback, name, fullPath) :
                loadWorldModel(worldModelPlayback, name, fullPath);

            // Loading failed                 
            if (!wasLoaded) OUTPUT_ERROR(name + " is corrupted.");
        }
    }
}

void TIPlaybackProvider::enforceConsistency(TIPlayback& worldModelPlayback)
{
    std::vector<std::string> matches;
    for (WorldData& data : worldModelPlayback.models)
    {

        // find the last instance of the word worldmodel in the filename
        std::string name = data.fileName;
        size_t rpos = name.rfind("worldmodel");

        // if it was found we replace worldmodel with playback
        if (rpos != std::string::npos) name.replace(rpos, 10, "playback");

        // search for the file inside the playback stack
        auto result = std::find_if(worldModelPlayback.data.begin(), worldModelPlayback.data.end(), [name](PlaybackData current)
            {
                return (current.fileName == name);
            });

        // result was found -> skip
        if (result != worldModelPlayback.data.end()) continue;

        // no corresponding playback exists -> mark for removal
        matches.push_back(data.fileName);
    }

    // remove the marked worldmodels
    worldModelPlayback.models.erase(std::remove_if(worldModelPlayback.models.begin(), worldModelPlayback.models.end(), [matches](WorldData current)
        {
            return (std::find(matches.begin(), matches.end(), current.fileName) != matches.end());
        }), worldModelPlayback.models.end());
}

bool TIPlaybackProvider::loadWorldModel(TIPlayback& store, std::string name, std::string path)
{
    try
    {
        WorldData data = WorldData(path, false);

        // We dont want any empty playbacks -> abort
        if (data.models.empty()) return false;

        // World Track is valid -> store it 
        store.models.push_back(data);
    }
    catch (const std::exception& e)
    {
        OUTPUT_WARNING(e.what());
        return false;
    }
    return true;
}

bool TIPlaybackProvider::loadPlayback(TIPlayback& worldModelPlayback, std::string name, std::string path)
{
    try
    {
        PlaybackData data = PlaybackData(path, false);

        // We dont want any empty playbacks -> abort
        if (data.actions.empty()) return false;

        // Playback is valid -> store it 
        worldModelPlayback.data.push_back(data);
    }
    catch (const std::exception& e)
    {
        OUTPUT_WARNING(e.what());
        return false;
    }
    return true;
}

