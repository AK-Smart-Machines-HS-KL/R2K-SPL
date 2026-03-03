#include "TIPlaybackProvider.h"
#include "Modules/BehaviorControl/BehaviorControl/BehaviorControl.h"
#include "Platform/File.h"
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
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
      std::vector<std::string> inconsistentFiles = enforceConsistency(playbackData);
      
      // Report inconsistencies once per session (robot #1 only)
      static bool hasReportedInconsistencies = false;
      if (!inconsistentFiles.empty() && theRobotInfo.number == 1 && !hasReportedInconsistencies)
      {
          OUTPUT_WARNING("TI: Removed " << static_cast<int>(inconsistentFiles.size()) << " inconsistent file(s):");
          for (const auto& file : inconsistentFiles)
          {
              OUTPUT_WARNING("  - " << file);
          }
          hasReportedInconsistencies = true;
      }
      
      printLoadedData(playbackData);
      playbackData.loaded = true;
  }
}

void TIPlaybackProvider::printLoadedData(TIPlaybackSequences &playbackData)
{
    if (theRobotInfo.number != 1) return; // do not tell this info 5 times
    
    // Summary output
    OUTPUT_TEXT("TI: Loaded " << static_cast<int>(playbackData.models.size()) << " worldmodels, " 
      << static_cast<int>(playbackData.data.size()) << " playback sequences");
    
    // Special detailed logging for PENALTY_DRIBBLE sequences
    for (const PlaybackSequence& data : playbackData.data)
    {
        if (data.fileName.find("PENALTY_DRIBBLE") != std::string::npos)
        {
            OUTPUT_TEXT("TI: PENALTY_DRIBBLE sequence loaded: " << data.fileName << " (" << static_cast<int>(data.actions.size()) << " actions)");
            for (size_t i = 0; i < data.actions.size(); ++i)
            {
                const PlaybackAction& action = data.actions[i];
                // Output skill enum value and maxTime
                OUTPUT_TEXT("  [" << static_cast<int>(i) << "] skill=" << static_cast<int>(action.skill) << " maxTime=" << action.maxTime << "ms");
            }
        }
    }
    
    // Detailed output only via debug request
    DECLARED_DEBUG_RESPONSE("TIPlaybackProvider:detailed");
    DEBUG_RESPONSE("TIPlaybackProvider:detailed")
    {
        OUTPUT_TEXT("Worldmodels:");
        for (const WorldData& model : playbackData.models)
        {
            if (model.trigger.robotPose.translation.x() != 0.0f || model.trigger.robotPose.translation.y() != 0.0f)
            {
                OUTPUT_TEXT("  " << model.fileName << " @ (" << model.trigger.robotPose.translation.x() << ", " << model.trigger.robotPose.translation.y() << ")");
            }
        }
        OUTPUT_TEXT("Playback sequences:");
        for (const PlaybackSequence& data : playbackData.data)
        {
            if (!data.fileName.empty())
            {
                OUTPUT_TEXT("  " << data.fileName << " (" << static_cast<int>(data.actions.size()) << " actions)");
            }
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

        // Global tracking for duplicate detection across all subdirectories
        std::map<std::string, std::string> filenameToPath;

        // Count files for summary
        int filesProcessed = 0;
        int filesFailed = 0;
        int duplicatesFound = 0;

        // First pass: detect duplicates across all subdirectories
        for (const std::string& dir : subDirs)
        {
            // Get a list of all files inside each directory
            std::list<std::string> files = File::getFiles(teachInDir + dir);
            for (const std::string& file : files)
            {
                // Skip non-CSV files
                if (file.find(".csv") == std::string::npos)
                    continue;

                // Check if this filename has been seen before (global duplicate)
                auto existingFile = filenameToPath.find(file);
                if (existingFile != filenameToPath.end())
                {
                    duplicatesFound++;
                    // Report duplicates once per session (robot #1 only)
                    static bool hasReportedDuplicates = false;
                    if (theRobotInfo.number == 1 && !hasReportedDuplicates)
                    {
                        OUTPUT_ERROR("TI: Duplicate filename detected! " << file << " exists in both " 
                                     << existingFile->second << " and " << dir << ". Using first occurrence.");
                        hasReportedDuplicates = true;
                    }
                }
                else
                {
                    filenameToPath[file] = dir;
                }
            }
        }

        // Second pass: load files, skipping duplicates
        for (const std::string& dir : subDirs)
        {
            // Get a list of all files inside each directory
            std::list<std::string> files = File::getFiles(teachInDir + dir);
            for (const std::string& file : files)
            {
                // Skip non-CSV files
                if (file.find(".csv") == std::string::npos)
                    continue;

                // Skip if this is a duplicate (not the first occurrence)
                if (filenameToPath[file] != dir)
                {
                    filesProcessed++;
                    continue;
                }

                std::string name = dir + "/" + file;
                filesProcessed++;

                std::string fullPath = teachInDir + name;

                // Determine if the csv is a worldmodel or playback file
                bool isPlayback = (file.find("worldmodel") == std::string::npos);

                // Attempt to load and parse each csv
                bool wasLoaded = isPlayback ? loadPlayback(playbackData, name, fullPath) : loadWorldModel(playbackData, name, fullPath);

                // Loading failed
                if (!wasLoaded)
                {
                    filesFailed++;
                    OUTPUT_ERROR(name + " is corrupted.");
                }
            }
        }

        // Output summary only via debug request
        DECLARED_DEBUG_RESPONSE("TIPlaybackProvider:fileLoadSummary");
        DEBUG_RESPONSE("TIPlaybackProvider:fileLoadSummary")
        {
            if (theRobotInfo.number == 1)
            {
                OUTPUT_TEXT("TI: Processed " << filesProcessed << " files (" << filesFailed << " failed, " 
                           << duplicatesFound << " duplicates skipped)");
            }
        }
    }
    catch (const std::exception& e)
    {
        OUTPUT_ERROR("TIPlaybackProvider::loadTeachInData failed: " << e.what());
    }
}

std::vector<std::string> TIPlaybackProvider::enforceConsistency(TIPlaybackSequences &playbackData)
{
    std::vector<std::string> removedFiles;

    // Guard against empty data structures
    if (playbackData.models.empty() && playbackData.data.empty())
    {
        DECLARED_DEBUG_RESPONSE("TIPlaybackProvider:consistency");
        DEBUG_RESPONSE("TIPlaybackProvider:consistency")
        {
            if (theRobotInfo.number == 1)
                OUTPUT_TEXT("TI: No TeachIn data loaded, skipping consistency check");
        }
        return removedFiles;
    }

    // Check for worldmodels without matching playbacks
    std::vector<std::string> orphanedWorldmodels;
    for (const WorldData &data : playbackData.models)
    {
        // Verify fileName is not empty before processing
        if (data.fileName.empty())
        {
            removedFiles.push_back("(empty filename)");
            orphanedWorldmodels.push_back(data.fileName);
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
        removedFiles.push_back(data.fileName);
        orphanedWorldmodels.push_back(data.fileName);
    }

    // Check for playbacks without matching worldmodels
    std::vector<std::string> orphanedPlaybacks;
    for (const PlaybackSequence &data : playbackData.data)
    {
        // Verify fileName is not empty before processing
        if (data.fileName.empty())
        {
            removedFiles.push_back("(empty filename)");
            orphanedPlaybacks.push_back(data.fileName);
            continue;
        }

        // find the last instance of the word playback in the filename
        std::string name = data.fileName;
        size_t rpos = name.rfind("playback");

        // if it was found we replace playback with worldmodel
        if (rpos != std::string::npos)
            name.replace(rpos, 8, "worldmodel");

        // search for the file inside the worldmodel stack
        auto result = std::find_if(playbackData.models.begin(), playbackData.models.end(), [name](WorldData current)
                                   { return (current.fileName == name); });

        // result was found -> skip
        if (result != playbackData.models.end())
            continue;

        // no corresponding worldmodel exists -> mark for removal
        removedFiles.push_back(data.fileName);
        orphanedPlaybacks.push_back(data.fileName);
    }

    // remove the marked worldmodels
    if (!orphanedWorldmodels.empty())
    {
        playbackData.models.erase(std::remove_if(playbackData.models.begin(), playbackData.models.end(), [orphanedWorldmodels](WorldData current)
                                                       { return (std::find(orphanedWorldmodels.begin(), orphanedWorldmodels.end(), current.fileName) != orphanedWorldmodels.end()); }),
                                        playbackData.models.end());
    }

    // remove the marked playbacks
    if (!orphanedPlaybacks.empty())
    {
        playbackData.data.erase(std::remove_if(playbackData.data.begin(), playbackData.data.end(), [orphanedPlaybacks](PlaybackSequence current)
                                                     { return (std::find(orphanedPlaybacks.begin(), orphanedPlaybacks.end(), current.fileName) != orphanedPlaybacks.end()); }),
                                      playbackData.data.end());
    }

    return removedFiles;
}

bool TIPlaybackProvider::loadWorldModel(TIPlaybackSequences &playbackData, std::string name, std::string path)
{
    try
    {
        WorldData data = WorldData(path, false);

        // We dont want any empty playbacks -> abort
        if (data.models.empty())
            return false;

        // Validate trigger point data bounds
        const WorldModel& trigger = data.trigger;
        
        // Check gameState and setPlay enums are within valid range (0-5)
        if (trigger.gameState < 0 || trigger.gameState > 5)
        {
            OUTPUT_WARNING(name << ": Invalid gameState " << trigger.gameState << " (must be 0-5)");
            return false;
        }
        if (trigger.setPlay < 0 || trigger.setPlay > 5)
        {
            OUTPUT_WARNING(name << ": Invalid setPlay " << trigger.setPlay << " (must be 0-5)");
            return false;
        }

        // Check coordinate bounds (SPL field: 9000mm x 6000mm, so ±4500 x ±3000)
        const float MAX_X = 4500.0f;
        const float MAX_Y = 3000.0f;
        const float MAX_R = M_PI;
        
        if (std::abs(trigger.robotPose.translation.x()) > MAX_X)
        {
            OUTPUT_WARNING(name << ": Robot X coordinate " << trigger.robotPose.translation.x() 
                          << " exceeds bounds [" << -MAX_X << ", " << MAX_X << "]");
            return false;
        }
        if (std::abs(trigger.robotPose.translation.y()) > MAX_Y)
        {
            OUTPUT_WARNING(name << ": Robot Y coordinate " << trigger.robotPose.translation.y() 
                          << " exceeds bounds [" << -MAX_Y << ", " << MAX_Y << "]");
            return false;
        }
        if (std::abs(trigger.robotPose.rotation) > MAX_R)
        {
            OUTPUT_WARNING(name << ": Robot rotation " << trigger.robotPose.rotation 
                          << " exceeds bounds [" << -MAX_R << ", " << MAX_R << "]");
            return false;
        }

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
