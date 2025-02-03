/**
 * @file TIData.cpp
 * @author Andy Hobelsberger
 * @brief Implements methods of Objects in TIData.h (Such as PlaybackSequence)
 * @version 0.1
 * @date 2022-01-14
 * 
 */

#include "TIData.h"
#include <rapidcsv.h>
#include <iostream>
#include <fstream>
#include "Tools/Debugging/Debugging.h"
#include "Tools/Streams/AutoStreamable.h"
#include "Tools/Streams/Enum.h"

PlaybackSequence::PlaybackSequence(std::string file, bool isRelative) {
    try
    {
        if(isRelative) {
            fileName = TI_Directory + file;
        } else {
			fileName = file;
        }
        // Load the playback file
        rapidcsv::Document doc(fileName);

        // Parse the actions row by row
        for(size_t i = 0; i < doc.GetRowCount(); i++)
        {
            PlaybackAction act;
            act.skill = (PlaybackAction::Skills) TypeRegistry::getEnumValue(typeid(PlaybackAction::Skills).name(), doc.GetCell<std::string>("skillName", i));            
            act.maxTime = doc.GetCell<int>("maxTime", i);
            act.angleParam1 = doc.GetCell<float>("angleParam1", i);
            act.angleParam2 = doc.GetCell<float>("angleParam2", i);
            act.poseParam.translation.x() = doc.GetCell<float>("poseParam X", i);
            act.poseParam.translation.y() = doc.GetCell<float>("poseParam Y", i);
            act.poseParam.rotation = doc.GetCell<float>("poseParam R", i);
            act.vector3Param.x() = doc.GetCell<float>("vector3Param X", i);
            act.vector3Param.y() = doc.GetCell<float>("vector3Param Y", i);
            act.vector3Param.z() = doc.GetCell<float>("vector3Param Z", i);
            act.boolParam = doc.GetCell<int>("boolParam", i) != 0;
            act.floatParam = doc.GetCell<float>("floatParam", i);
            act.intParam = doc.GetCell<int>("intParam", i);
            act.stringParam = doc.GetCell<std::string>("stringParam", i);

            actions.push_back(act);
        }
    }
    catch(const std::exception& e)
    {
        OUTPUT_WARNING(e.what());
    }
}

void PlaybackSequence::save(std::string path){

    if(actions.empty()) {
        OUTPUT_WARNING("No Playback Data to save!");
        return;
    }

    if(path.empty()) {
        if(fileName.empty()) {
            OUTPUT_WARNING("File could no be saved: bad path");
        }
        path = fileName;
    }

    try
    {
        rapidcsv::Document doc;

        doc.SetColumnName(0, "skillName");
        doc.SetColumnName(1, "maxTime");
        doc.SetColumnName(2, "angleParam1");
        doc.SetColumnName(3, "angleParam2");
        doc.SetColumnName(4, "poseParam X");
        doc.SetColumnName(5, "poseParam Y");
        doc.SetColumnName(6, "poseParam R");
        doc.SetColumnName(7, "vector3Param X");
        doc.SetColumnName(8, "vector3Param Y");
        doc.SetColumnName(9, "vector3Param Z");
        doc.SetColumnName(10, "boolParam");
        doc.SetColumnName(11, "floatParam");
        doc.SetColumnName(12, "intParam");
        doc.SetColumnName(13, "stringParam");

        size_t i = 0;
        for(auto act : actions)
        {
            // NOTE: Data must be converted to basic C++ datatypes. Not even C strings are supported
            doc.SetCell(0, i, (std::string) TypeRegistry::getEnumName(typeid(PlaybackAction::Skills).name(), act.skill));
            doc.SetCell(1, i, act.maxTime);
            doc.SetCell(2, i, (float) act.angleParam1);
            doc.SetCell(3, i, (float) act.angleParam2);
            doc.SetCell(4, i, act.poseParam.translation.x());
            doc.SetCell(5, i, act.poseParam.translation.y());
            doc.SetCell(6, i, (float) act.poseParam.rotation);
            doc.SetCell(7, i, act.vector3Param.x());
            doc.SetCell(8, i, act.vector3Param.y());
            doc.SetCell(9, i, act.vector3Param.z());
            doc.SetCell(10, i, (int) act.boolParam); // Booleans unsupported, must convert to int type
            doc.SetCell(11, i, act.floatParam);
            doc.SetCell(12, i, act.intParam);
            doc.SetCell(13, i, act.stringParam);
            
            i++;
        }

        doc.Save(path);

        OUTPUT_TEXT("Playback Data saved under " << path);

    }
    catch(const std::exception& e)
    {
        OUTPUT_WARNING(e.what());
        return;
    }
}

void PlaybackSequence::clear() {
    fileName = "";
    actions.clear();
}

WorldData::WorldData(std::string file, bool isRelative) {
    try
    {
        if(isRelative) {
            fileName = TI_Directory + file;
        } else {
			fileName = file;
        }
        // Load the playback file
        rapidcsv::Document doc(fileName);

         for(size_t i = 0; i < doc.GetRowCount(); i++) {
            // Read the data from the csv file
            WorldModel world;
            // use name from path instead?
            world.gameState = doc.GetCell<int>("gameState", i);
            world.setPlay= doc.GetCell<int>("setPlay", i);
            world.timeLeft = doc.GetCell<uint32_t>("timeLeft", i);
            world.robotNumber = doc.GetCell<int>("robotNumber", i);
            world.robotPose.translation.x() = doc.GetCell<float>("robotPose X", i);
            world.robotPose.translation.y() = doc.GetCell<float>("robotPose Y", i);
            world.robotPose.rotation = doc.GetCell<float>("robotPose R", i);
            world.distanceToGoal = doc.GetCell<float>("distanceToGoal", i);
            world.ballIsNear = doc.GetCell<int>("ballIsNear", i) == 1;
            world.ballPosition.x() = doc.GetCell<float>("ballPosition X", i);
            world.ballPosition.y() = doc.GetCell<float>("ballPosition Y", i);
            world.ballDistanceToBot = doc.GetCell<float>("ballDistanceToBot", i);
            world.ballDistanceToGoal = doc.GetCell<float>("ballDistanceToGoal", i);
            
            // Populate the list of team-positions
            for(int j = 1; j <= 5; j++)
            {
                Vector2f pos;
                std::string cell = "team" + std::to_string(j) + " ";
                pos.x() = doc.GetCell<float>(cell + "X", i);
                pos.y() = doc.GetCell<float>(cell + "Y", i);
                world.teamList.push_back(pos);
            }

            // Populate the list of opponent-positions
            for(int j = 1; j <= 5; j++)
            {
                Vector2f pos;
                std::string cell = "opponent" + std::to_string(j) + " ";
                pos.x() = doc.GetCell<float>(cell + "X", i);
                pos.y() = doc.GetCell<float>(cell + "Y", i);
                world.opponentList.push_back(pos);
            }
            models.push_back(world);
        }

        // Check to avoid Access violation
        if(!models.empty()) {
            trigger = models.back();
        }

    }
    catch(const std::exception& e)
    {
        OUTPUT_WARNING(e.what());
        return;
    }
}

void WorldData::save(std::string path){

    if(models.empty()) {
        OUTPUT_WARNING("No World Data to save! Is the Recorder Module running?");
        return;
    }

    if(path.empty()) {
        if(fileName.empty()) {
            OUTPUT_WARNING("File could no be saved: bad path");
        }
        path = fileName;
    }

    try
    {
        rapidcsv::Document doc;

        size_t i = 0;
        doc.SetColumnName(i, "gameState"); i++;
        doc.SetColumnName(i, "setPlay"); i++;
        doc.SetColumnName(i, "timeLeft"); i++;
        doc.SetColumnName(i, "robotNumber"); i++;
        doc.SetColumnName(i, "robotPose X"); i++;
        doc.SetColumnName(i, "robotPose Y"); i++;
        doc.SetColumnName(i, "robotPose R"); i++;
        doc.SetColumnName(i, "distanceToGoal"); i++;
        doc.SetColumnName(i, "ballIsNear"); i++;
        doc.SetColumnName(i, "ballPosition X"); i++;
        doc.SetColumnName(i, "ballPosition Y"); i++;
        doc.SetColumnName(i, "ballDistanceToBot"); i++;
        doc.SetColumnName(i, "ballDistanceToGoal"); i++;

        const int nRobots = 5;

        // Own Robots
        for (size_t j = 1; j <= nRobots; j++)
        {
            doc.SetColumnName(i, "team" + std::to_string(j) + " X"); i++;
            doc.SetColumnName(i, "team" + std::to_string(j) + " Y"); i++;
        }
        
        // Opponent Robots
        for (size_t j = 1; j <= nRobots; j++)
        {
            doc.SetColumnName(i, "opponent" + std::to_string(j) + " X"); i++;
            doc.SetColumnName(i, "opponent" + std::to_string(j) + " Y"); i++;
        }

        i = 0;
        for(auto model : models)
        {
            doc.InsertRow<int>(i);
            size_t col = 0;
            doc.SetCell(col, i, model.gameState); col++;
            doc.SetCell(col, i, model.setPlay); col++;
            doc.SetCell(col, i, model.timeLeft); col++;
            doc.SetCell(col, i, model.robotNumber); col++;
            doc.SetCell(col, i, model.robotPose.translation.x()); col++;
            doc.SetCell(col, i, model.robotPose.translation.y()); col++;
            doc.SetCell(col, i, (float) model.robotPose.rotation); col++;
            doc.SetCell(col, i, model.distanceToGoal); col++;
            doc.SetCell(col, i, (int) model.ballIsNear); col++;
            doc.SetCell(col, i, model.ballPosition.x()); col++;
            doc.SetCell(col, i, model.ballPosition.y()); col++;
            doc.SetCell(col, i, model.ballDistanceToBot); col++;
            doc.SetCell(col, i, model.ballDistanceToGoal); col++;

            // Own Robots
            for(size_t j = 0; j < nRobots; j++)
            {
                if(j >= model.teamList.size()) {  //check if list is long enough, enter 0 if not
                    doc.SetCell(col + 2*j + 0, i, 0); 
                    doc.SetCell(col + 2*j + 1, i, 0);
                    continue;
                }
                doc.SetCell(col + 2*j + 0, i, model.teamList[j].x()); 
                doc.SetCell(col + 2*j + 1, i, model.teamList[j].y());
            }

            // Opponent Robots
            col += 2*nRobots;
            for(size_t j = 0; j < nRobots; j++)
            {
                if(j >= model.opponentList.size()) {  //check if list is long enough, enter 0 if not
                    doc.SetCell(col + 2*j + 0, i, 0); 
                    doc.SetCell(col + 2*j + 1, i, 0);
                    continue;
                } 
                doc.SetCell(col + 2*j + 0, i, model.opponentList[j].x()); 
                doc.SetCell(col + 2*j + 1, i, model.opponentList[j].y());
            }

            i++;
        }

        doc.Save(path);

        OUTPUT_TEXT("World Data saved under " << path);

    }
    catch(const std::exception& e)
    {
        OUTPUT_WARNING(e.what());
        return;
    }
}

void WorldData::clear() {
    fileName = "";
    models.clear();
}

// Chainable Setters
PlaybackAction& PlaybackAction::setSkill(PlaybackAction::Skills val) {skill = val; return *this;}
PlaybackAction& PlaybackAction::setAngle1(const Angle& val) {angleParam1 = val; return *this;}
PlaybackAction& PlaybackAction::setAngle2(const Angle& val) {angleParam2 = val; return *this;}
PlaybackAction& PlaybackAction::setPose(const Pose2f& val) {poseParam = val; return *this;}
PlaybackAction& PlaybackAction::setVector(const Vector3f& val) {vector3Param = val; return *this;}
PlaybackAction& PlaybackAction::setBool(bool val) {boolParam = val; return *this;}
PlaybackAction& PlaybackAction::setFloat(float val) {floatParam = val; return *this;}
PlaybackAction& PlaybackAction::setInt(int val) {intParam = val; return *this;}
PlaybackAction& PlaybackAction::setString(const std::string& val) {stringParam = val; return *this;}