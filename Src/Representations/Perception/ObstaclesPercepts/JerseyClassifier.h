/**
 * @file JerseyClassifier.h
 *
 * This file declares a representation which contains functionality to detect
 * jerseys that players wear.
 *
 * @author Arne Hasselbring
 * @author Laura Hammerschmidt (HUETCHEN addition)
 */

#pragma once

#include "Tools/Function.h"
#include "Tools/Streams/AutoStreamable.h"
#include "Representations/Perception/ObstaclesPercepts/ObstaclesFieldPercept.h"
#include "Representations/Perception/ObstaclesPercepts/ObstaclesImagePercept.h"

STREAMABLE(JerseyClassifier,
{
  /**
   * Detect the jersey color.
   * @param obstacleInImage The obstacle as it was detected in the image.
   * @param obstacleOnField The fields detectedJersey and ownTeam will be updated if a
   *                        jersey color was detected.
   */
  FUNCTION(void(const ObstaclesImagePercept::Obstacle& obstacleInImage, ObstaclesFieldPercept::Obstacle& obstacleOnField)) detectJersey;

  //HUETCHEN addition: gibt erkannte jerseyColor zurück
  FUNCTION(int(const ObstaclesImagePercept::Obstacle& obstacleInImage)) detectJerseyColor,
});
