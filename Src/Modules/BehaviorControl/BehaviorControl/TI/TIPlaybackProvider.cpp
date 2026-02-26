#include "TIPlaybackProvider.h"
#include "Modules/BehaviorControl/BehaviorControl/BehaviorControl.h"
#include "Platform/File.h"
#include <iostream>
#include <fstream>
#include <rapidcsv.h>
#include "Tools/RingBuffer.h"
#include "Tools/Math/Geometry.h"
#include "Tools/Debugging/DebugDrawings.h"
#include "Tools/Debugging/DebugDrawings3D.h"

MAKE_MODULE(TIPlaybackProvider, behaviorControl);

bool isNumber(const std::string &str)
{
    for (char const &c : str)
    {
        if (std::isdigit(c) == 0)
            return false;
    }
    return true;
}

TIPlaybackProvider::TIPlaybackProvider()
{
}



void TIPlaybackProvider::update(TIPlaybackSequences &playbackData)
{
  // activate with:
  // dr debugDrawing3d:representation:TIPlaybackProvider 

  DEBUG_DRAWING3D("representation:TIPlaybackProvider", "field");

  // Guard against empty models before drawing
  if (!playbackData.models.empty())
  {
    for (WorldData model : playbackData.models)
    {
      // OUTPUT_TEXT(model.trigger.robotPose.translation.x() << " " << model.trigger.robotPose.translation.y());
      // if(-1 == model.fileName.find("Standard"))
      if (model.trigger.setPlay == SET_PLAY_CORNER_KICK)
        CYLINDER3D("representation:TIPlaybackProvider", model.trigger.robotPose.translation.x(), model.trigger.robotPose.translation.y(), -1.f, 0.f, 0.f, 0.f, (int)model.trigger.ballDistanceToBot/10, 2,  ColorRGBA::yellow);
      else
        CYLINDER3D("representation:TIPlaybackProvider", model.trigger.robotPose.translation.x(), model.trigger.robotPose.translation.y(), -1.f, 0.f, 0.f, 0.f, std::max(50,(int)model.trigger.ballDistanceToBot/10), 2, ColorRGBA::gray);
    }
  }

  if (!playbackData.loaded) {
      loadTeachInData(playbackData);
      enforceConsistency(playbackData);
      printLoadedData(playbackData);
      playbackData.loaded = true;
  }

  // dr loadTeachInData
  DEBUG_RESPONSE_ONCE("loadTeachInData")
  {
      loadTeachInData(playbackData);
      enforceConsistency(playbackData);
      printLoadedData(playbackData);
  }
}

void TIPlaybackProvider::printLoadedData(TIPlaybackSequences &playbackData)
{
    if (theRobotInfo.number != 1) return; // do not tell this info 5 times
    
    OUTPUT_TEXT("Worldmodel: (count: " << static_cast<int>(playbackData.models.size()) << ")");

    // Print the names of all loaded worldmodels to the console
    for (const WorldData& model : playbackData.models)
    {
        if (model.trigger.robotPose.translation.x() != 0.0f || model.trigger.robotPose.translation.y() != 0.0f)
        {
            OUTPUT_TEXT("  " << model.fileName << " -> " << model.trigger.robotPose.translation.x() << " " << model.trigger.robotPose.translation.y());
        }
    }

    OUTPUT_TEXT("------- (count: " << static_cast<int>(playbackData.data.size()) << ")");
    OUTPUT_TEXT("Playback:");

    // Print the names of all loaded playbacks to the console
    for (const PlaybackSequence& data : playbackData.data)
    {
        if (!data.fileName.empty())
        {
            OUTPUT_TEXT("  " << data.fileName << " (actions: " << static_cast<int>(data.actions.size()) << ")");
        }
    }
}

void TIPlaybackProvider::loadTeachInData(TIPlaybackSequences &playbackData)
{
    try
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
                if (theRobotInfo.number == 1)  // do not tell this info 5 times
                  OUTPUT_TEXT("Loading: " + name);

                std::string fullPath = teachInDir + name;

                // Determine if the csv is a worldmodel or playback file
                bool isPlayback = (file.find("worldmodel") == std::string::npos);

                // Attempt to load and parse each csv
                bool wasLoaded = isPlayback ? loadPlayback(playbackData, name, fullPath) : loadWorldModel(playbackData, name, fullPath);

                // Loading failed
                if (!wasLoaded)
                {
                    if (theRobotInfo.number == 1)  // do not tell this info 5 times
                        OUTPUT_ERROR(name + " is corrupted.");
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        OUTPUT_ERROR("TIPlaybackProvider::loadTeachInData failed: " << e.what());
    }
}

void TIPlaybackProvider::enforceConsistency(TIPlaybackSequences &playbackData)
{
    // Guard against empty data structures
    if (playbackData.models.empty())
    {
        if (theRobotInfo.number == 1)
            OUTPUT_TEXT("TI: No worldmodels loaded, skipping consistency check");
        return;
    }

    if (playbackData.data.empty())
    {
        OUTPUT_WARNING("TI: Worldmodels loaded but no playback data found. Clearing all worldmodels.");
        playbackData.models.clear();
        return;
    }

    std::vector<std::string> matches;
    for (WorldData &data : playbackData.models)
    {
        // Verify fileName is not empty before processing
        if (data.fileName.empty())
        {
            OUTPUT_WARNING("TI: Encountered empty fileName in worldmodel, marking for removal");
            matches.push_back(data.fileName);
            continue;
        }

        // find the last instance of the word worldmodel in the filename
        std::string name = data.fileName;
        size_t rpos = name.rfind("worldmodel");

        // if it was found we replace worldmodel with playback
        if (rpos != std::string::npos)
            name.replace(rpos, 10, "playback");

        // search for the file inside the playback stack
        auto result = std::find_if(playbackData.data.begin(), playbackData.data.end(), [name](PlaybackSequence current)
                                   { return (current.fileName == name); });

        // result was found -> skip
        if (result != playbackData.data.end())
            continue;

        // no corresponding playback exists -> mark for removal
        matches.push_back(data.fileName);
    }

    // remove the marked worldmodels
    if (!matches.empty())
    {
        playbackData.models.erase(std::remove_if(playbackData.models.begin(), playbackData.models.end(), [matches](WorldData current)
                                                       { return (std::find(matches.begin(), matches.end(), current.fileName) != matches.end()); }),
                                        playbackData.models.end());
    }
}

bool TIPlaybackProvider::loadWorldModel(TIPlaybackSequences &playbackData, std::string name, std::string path)
{
    try
    {
        WorldData data = WorldData(path, false);

        // We dont want any empty playbacks -> abort
        if (data.models.empty())
            return false;

        // World Track is valid -> store it
        playbackData.models.push_back(data);
    }
    catch (const std::exception &e)
    {
        OUTPUT_WARNING(e.what());
        return false;
    }
    return true;
}

bool TIPlaybackProvider::loadPlayback(TIPlaybackSequences& playbackData, std::string name, std::string path)
{
  try
  {
    PlaybackSequence data = PlaybackSequence(path, false);

    // We dont want any empty playbacks -> abort
    if (data.actions.empty())
      return false;

    // Playback is valid -> store it
    playbackData.data.push_back(data);
  }
  catch (const std::exception& e)
  {
    OUTPUT_WARNING(e.what());
    return false;
  }
  return true;
}
