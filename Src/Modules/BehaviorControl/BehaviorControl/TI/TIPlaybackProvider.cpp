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

  for (WorldData model : playbackData.models)
  {


    // OUTPUT_TEXT(model.trigger.robotPose.translation.x() << " " << model.trigger.robotPose.translation.y());
    // if(-1 == model.fileName.find("Standard"))
    if (model.trigger.setPlay == SET_PLAY_CORNER_KICK)
      CYLINDER3D("representation:TIPlaybackProvider", model.trigger.robotPose.translation.x(), model.trigger.robotPose.translation.y(), -1.f, 0.f, 0.f, 0.f, (int)model.trigger.ballDistanceToBot/10, 2,  ColorRGBA::yellow);

      // RECTANGLE("representation:TIPlaybackProvider", 100, 100, 200, 200, 20, Drawings::solidPen, ColorRGBA::yellow);
    else
      CYLINDER3D("representation:TIPlaybackProvider", model.trigger.robotPose.translation.x(), model.trigger.robotPose.translation.y(), -1.f, 0.f, 0.f, 0.f, std::max(50,(int)model.trigger.ballDistanceToBot/10), 2, ColorRGBA::gray);
      // CYLINDER3D("representation:TIPlaybackProvider", model.trigger.robotPose.translation.x(), model.trigger.robotPose.translation.y(), 100, 0.f, 0.f, 0.f, 50, 2, ColorRGBA::red);
  }
  /* CIRCLE("representation:TIPlaybackProvider", "",
    position.x(), position.y(), 45, 0, // pen width
    Drawings::solidPen, ColorRGBA::black,
    Drawings::solidBrush, violet);
    */
  /*

  COMPLEX_DRAWING("module:TIPlaybackProvider:points") {
    const Vector3f ballPos3d = Vector3f(100.0f, 200.0f, 0.0f);
    SPHERE3D("module:TIPlaybackProvider:points", ballPos3d.x(), ballPos3d.y(), 350.f, 350.f, ColorRGBA(128, 64, 0));
  }

  DECLARE_DEBUG_DRAWING("module:TIPlaybackProvider", "drawingOnField");
  DEBUG_DRAWING3D("module:TIPlaybackProvider", "field") {
    const Vector3f ballPos3d = Vector3f(100.0f, 200.0f, 0.0f);
    SPHERE3D("representation:TIPlaybackProvider", ballPos3d.x(), ballPos3d.y(), 350.f, 350.f, ColorRGBA(128, 64, 0));
  }

  */
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
    OUTPUT_TEXT("Worldmodel:");

    // Print the names of all loaded worldmodels to the console
    for (WorldData model : playbackData.models)
    {
        // OUTPUT_TEXT(model.fileName);
      OUTPUT_TEXT(model.trigger.robotPose.translation.x() << " " << model.trigger.robotPose.translation.y());

    }

    OUTPUT_TEXT("-------------");
    OUTPUT_TEXT("Playback:");

    // Print the names of all loaded playbacks to the console
    for (PlaybackSequence data : playbackData.data)
    {
        OUTPUT_TEXT(data.fileName);
    }
}

void TIPlaybackProvider::loadTeachInData(TIPlaybackSequences &playbackData)
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
            if (theRobotInfo.number == 1)  // do not tell this info 5 timesOUTPUT_TEXT("Loading: " + name);
              OUTPUT_TEXT("Loading: " + name);

            std::string fullPath = teachInDir + name;

            // Determine if the csv is a worldmodel or playback file
            bool isPlayback = (file.find("worldmodel") == std::string::npos);

            // Attempt to load and parse each csv
            bool wasLoaded = isPlayback ? loadPlayback(playbackData, name, fullPath) : loadWorldModel(playbackData, name, fullPath);

            // Loading failed
            if (!wasLoaded)
                { if (theRobotInfo.number == 1)  // do not tell this info 5 timesOUTPUT_TEXT("Loading: " + name);
                    OUTPUT_ERROR(name + " is corrupted.");}
        }
    }
}

void TIPlaybackProvider::enforceConsistency(TIPlaybackSequences &playbackData)
{
    std::vector<std::string> matches;
    for (WorldData &data : playbackData.models)
    {

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
    playbackData.models.erase(std::remove_if(playbackData.models.begin(), playbackData.models.end(), [matches](WorldData current)
                                                   { return (std::find(matches.begin(), matches.end(), current.fileName) != matches.end()); }),
                                    playbackData.models.end());
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
