/**
 * @file JerseyClassifierProvider.cpp
 *
 * This file implements a module that provides functionality to classify jerseys
 * by sampling pixels in an estimated area and classifying each of them as
 * belonging to the own team or to the opponent team.
 *
 * @author Thomas Röfer (the algorithm)
 * @author Lukas Malte Monnerjahn (the algorithm)
 * @author Arne Hasselbring (the module)
 * @author Laura Hammerschmidt (HUETCHEN addition)
 */

#include "JerseyClassifierProvider.h"
#include "Tools/Debugging/DebugDrawings.h"
#include "Tools/Math/Transformation.h"
#include <algorithm>
#include <cmath>
#include <cmath>
#include <map>

MAKE_MODULE(JerseyClassifierProvider, perception);

void JerseyClassifierProvider::update(JerseyClassifier& jerseyClassifier)
{
  DECLARE_DEBUG_DRAWING("module:JerseyClassifierProvider:jerseyRange", "drawingOnImage");
  DECLARE_DEBUG_DRAWING("module:JerseyClassifierProvider:jersey", "drawingOnImage");
  DECLARE_DEBUG_DRAWING("module:JerseyClassifierProvider:jerseyWeights", "drawingOnImage");
  DECLARE_DEBUG_DRAWING("module:JerseyClassifierProvider:jerseyClassification", "drawingOnImage");

  jerseyClassifier.detectJersey = [this](const ObstaclesImagePercept::Obstacle& obstacleInImage, ObstaclesFieldPercept::Obstacle& obstacleOnField)
  {
    return detectJersey(obstacleInImage, obstacleOnField);
  };

   //HUETCHEN addition start1
   // Gibt die erkannte Jersey-Farbe zurück
  jerseyClassifier.detectJerseyColor = [this](const ObstaclesImagePercept::Obstacle& obstacleInImage) -> int
  {
    return detectJerseyColor(obstacleInImage);
  };
  //HUETCHEN addition end1
}

/**
 * Hue values for the different team colors. The indices are those of the TEAM_
 * constants defined in RoboCupGameControlData.h.
 */
static int jerseyHues[] =
{
  255 * 7 / 8, // blue + cyan
  255 * 2 / 8, // red
  255 * 4 / 8, // yellow
  0, // black (unused)
  0, // white (unused)
  255 * 5 / 8, // green
  255 * 3 / 8, // orange
  255 * 1 / 8, // purple
  255 * 7 / 16, // brown
  0, // gray (unused)
};

void JerseyClassifierProvider::detectJersey(const ObstaclesImagePercept::Obstacle& obstacleInImage, ObstaclesFieldPercept::Obstacle& obstacleOnField) const
{
  Vector2f lowerInImage;
  Vector2f upperInImage;
  // obstacleInImage distance is imprecise and only used if part of the robot is in lower image
  float distance = obstacleInImage.distance * 1000;
  if(obstacleInImage.bottomFound)
    distance = obstacleOnField.center.norm() + theRobotDimensions.footLength * 0.5f;
  RECTANGLE("module:JerseyClassifierProvider:jerseyClassification", obstacleInImage.left, obstacleInImage.top, obstacleInImage.right, obstacleInImage.bottom, 3, Drawings::solidPen, ColorRGBA::violet);
  DRAW_TEXT("module:JerseyClassifierProvider:jerseyClassification", obstacleInImage.left, obstacleInImage.top - 24, 10, ColorRGBA::black, "distance: " << distance);
  // Determine jersey region.
  const Vector2f centerOnField = obstacleOnField.center.normalized(distance);
  if(Transformation::robotToImage(Vector3f(centerOnField.x(), centerOnField.y(), jerseyYRange.min),
                                  theCameraMatrix, theCameraInfo, lowerInImage)
     && Transformation::robotToImage(Vector3f(centerOnField.x(), centerOnField.y(), jerseyYRange.max),
                                     theCameraMatrix, theCameraInfo, upperInImage))
  {
    lowerInImage = theImageCoordinateSystem.fromCorrected(lowerInImage);
    if(lowerInImage.y() >= jerseyMinYSamples)
    {
      upperInImage = theImageCoordinateSystem.fromCorrected(upperInImage);
      const int obstacleTop = std::max(0, obstacleInImage.top);
      if(upperInImage.y() < obstacleTop)
      {
        const float interpolationFactor = (static_cast<float>(obstacleTop) - lowerInImage.y()) / (lowerInImage.y() - upperInImage.y());
        upperInImage = Vector2f(lowerInImage.x() + (lowerInImage.x() - upperInImage.x()) * interpolationFactor, obstacleTop);
        //lowerInImage + (lowerInImage - upperInImage) * (static_cast<float>(obstacleTop) - lowerInImage.y()) / (lowerInImage.y() - upperInImage.y());
      }
      if(lowerInImage.y() > static_cast<float>(theCameraInfo.height - 1))
      {
        const float interpolationFactor = (static_cast<float>(theCameraInfo.height - 1) - upperInImage.y()) / (lowerInImage.y() - upperInImage.y());
        lowerInImage = Vector2f(upperInImage.x() + (lowerInImage.x() - upperInImage.x()) * interpolationFactor, static_cast<float>(theCameraInfo.height - 1));
        //upperInImage + (lowerInImage - upperInImage) * (static_cast<float>(theCameraInfo.height - 1) - upperInImage.y()) / (lowerInImage.y() - upperInImage.y());
      }

      const int width = obstacleInImage.right - obstacleInImage.left + 1;
      const float ySteps = std::min(lowerInImage.y() - upperInImage.y(), jerseyMaxYSamples);
      if(ySteps < jerseyMinYSamples)
      {
        obstacleOnField.type = ObstaclesFieldPercept::unknown;
        DRAW_TEXT("module:JerseyClassifierProvider:jerseyClassification", obstacleInImage.left, obstacleInImage.top + 12, 10, ColorRGBA::black, "class: unknown");
        return;
      }
      const float yStep = (lowerInImage.y() - upperInImage.y()) / ySteps;
      const float xyStep = (lowerInImage.x() - upperInImage.x()) / ySteps;
      float left = std::max(upperInImage.x() - static_cast<float>(width) * 0.5f, 0.f);
      float right = std::min(upperInImage.x() + static_cast<float>(width) * 0.5f, static_cast<float>(theCameraInfo.width) - 0.5f);
      const float xSteps = std::min((right - left), jerseyMaxXSamples);
      const float xStep = (right - left) / xSteps;
      // this is the maximum number, could be less if the examined robot is near the image's edge
      const float examinedPixels = xSteps * ySteps;

      // The maximum brightness is needed to determine black, white and gray relative to it.
      unsigned char maxBrightness = 0;
      
      Vector2f centerInImage = Vector2f(static_cast<float>(obstacleInImage.left + obstacleInImage.right) * 0.5f,
              std::min(static_cast<float>(obstacleInImage.bottom), static_cast<float>(theCameraInfo.height - (whiteScanOffSet + 1))) * (1.f - whiteScanHeightRatio)
              + std::min(lowerInImage.y(), static_cast<float>(theCameraInfo.height - (whiteScanOffSet + 1))) * whiteScanHeightRatio);
      float center_left = std::max(centerInImage.x() - static_cast<float>(width) * 0.5f, 0.f);
      float center_right = std::min(centerInImage.x() + static_cast<float>(width) * 0.5f, static_cast<float>(theCameraInfo.width) - 0.5f);
      for(int yOffset = -1; yOffset <= 1; ++yOffset)
        for(float x = center_left; x < center_right; x += xStep)
          maxBrightness = std::max(theECImage.grayscaled[static_cast<int>(centerInImage.y()) + whiteScanOffSet * yOffset][static_cast<int>(x)], maxBrightness);
      

      // Get pixel classifier
      std::vector<int> opponentColors = {theOpponentTeamInfo.fieldPlayerColour, theOpponentTeamInfo.goalkeeperColour};
      std::vector<int> ownColors = {theOwnTeamInfo.fieldPlayerColour};
      if (!theOwnTeamInfo.goalkeeperColour == theOpponentTeamInfo.goalkeeperColour) {
        ownColors.push_back(theOwnTeamInfo.goalkeeperColour);
      }

      const std::function<bool(int, int)>& isOwn = getPixelClassifier(ownColors, opponentColors, maxBrightness);
      const std::function<bool(int, int)>& isOpponent = getPixelClassifier(opponentColors, ownColors, maxBrightness);

      float ownPixels = 0;
      float opponentPixels = 0;

      for(float y = upperInImage.y(); y < lowerInImage.y();
          y += yStep, left = std::max(left + xyStep, 0.f), right = std::min(right + xyStep, static_cast<float>(theCameraInfo.width)))
      {
        // calculations for pixel weighting
        float ydiffFromCenter = 2 * (((lowerInImage.y() - upperInImage.y()) / 2) - (y - upperInImage.y())) / (lowerInImage.y() - upperInImage.y());
        float yFactor = std::abs(ydiffFromCenter) < 0.5f ? 1.f : (1.f - std::abs(ydiffFromCenter)) + 0.5f;
        float jerseyEnd = relativeJerseyWidth - 0.1f * std::abs(ydiffFromCenter - 0.4f);
        for(float x = left; x < right; x += xStep)
        {
          float weight = 1.f;
          if(weighPixels)
          {
            const float xdiffFromCenter = 2 * std::abs((right - left) / 2 + left - x) / (right - left);
            const float xFactor = xdiffFromCenter < jerseyEnd ? 1 : 1 - (xdiffFromCenter - jerseyEnd);
            weight = xFactor * yFactor;
          }
          DOT("module:JerseyClassifierProvider:jerseyWeights", static_cast<int>(x), static_cast<int>(y),
              ColorRGBA(static_cast<unsigned  char>(240 - 240 * weight), static_cast<unsigned  char>(240 * weight), static_cast<unsigned  char>(240 * weight), 220),
              ColorRGBA(static_cast<unsigned  char>(240 - 240 * weight), static_cast<unsigned  char>(240 * weight), static_cast<unsigned  char>(240 * weight), 220));
          if(isOwn(static_cast<int>(x), static_cast<int>(y)))
          {
            ownPixels += weight;
            DOT("module:JerseyClassifierProvider:jersey", static_cast<int>(x), static_cast<int>(y), ColorRGBA::yellow, ColorRGBA::yellow);
          }
          else if(isOpponent(static_cast<int>(x), static_cast<int>(y)))
          {
            opponentPixels += weight;
            DOT("module:JerseyClassifierProvider:jersey", static_cast<int>(x), static_cast<int>(y), ColorRGBA::blue, ColorRGBA::blue);
          }
          else
            DOT("module:JerseyClassifierProvider:jersey", static_cast<int>(x), static_cast<int>(y), ColorRGBA::green, ColorRGBA::green);
        }
      }
      // threshold to counter white robot parts or green field background being classified as opponent jersey
      if((theOpponentTeamInfo.fieldPlayerColour == TEAM_WHITE || theOpponentTeamInfo.fieldPlayerColour == TEAM_GREEN)
         && theOwnTeamInfo.fieldPlayerColour != TEAM_WHITE && theOwnTeamInfo.fieldPlayerColour != TEAM_GREEN)
      {
        const float threshold = 1.5f * ownPixels + (examinedPixels) / 5;
        opponentPixels = opponentPixels <= threshold ? 0 : opponentPixels - threshold;
      }
      DRAW_TEXT("module:JerseyClassifierProvider:jerseyClassification", obstacleInImage.left, obstacleInImage.top - 14, 10, ColorRGBA::black, "ownPlayer: " << ownPixels);
      DRAW_TEXT("module:JerseyClassifierProvider:jerseyClassification", obstacleInImage.left, obstacleInImage.top - 4, 10, ColorRGBA::black, "opponent: " << opponentPixels);
      float minJerseyWeight = examinedPixels * minJerseyWeightRatio;
      if((ownPixels > minJerseyWeight || opponentPixels > minJerseyWeight)
         && (ownPixels > (ownPixels + opponentPixels) * minJerseyRatio
             || opponentPixels > (ownPixels + opponentPixels) * minJerseyRatio))
      {
        if(ownPixels > opponentPixels)
        {
          obstacleOnField.type = ObstaclesFieldPercept::ownPlayer;

          // HUETCHEN ADDITION: speichert die eigene Team-Farbe
          obstacleOnField.jerseyColor = theOwnTeamInfo.fieldPlayerColour;

          DRAW_TEXT("module:JerseyClassifierProvider:jerseyClassification", obstacleInImage.left, obstacleInImage.top + 10, 10, ColorRGBA::black, "class: own player");
        }
        else
        {
          obstacleOnField.type = ObstaclesFieldPercept::opponentPlayer;

          // HUETCHEN ADDITION: erkennt die genaue Gegner-Farbe
          int detectedColor = detectJerseyColor(obstacleInImage);
          obstacleOnField.jerseyColor = (detectedColor != -1) ? detectedColor : theOpponentTeamInfo.fieldPlayerColour;
    
          DRAW_TEXT("module:JerseyClassifierProvider:jerseyClassification", obstacleInImage.left, obstacleInImage.top + 10, 10, ColorRGBA::black, "class: oponent");
        }
      }
      else
      {
        obstacleOnField.type = ObstaclesFieldPercept::unknown;
        DRAW_TEXT("module:JerseyClassifierProvider:jerseyClassification", obstacleInImage.left, obstacleInImage.top + 12, 10, ColorRGBA::black, "class: unknown");
      }
      
    }

    
  }

 
  
}

std::function<bool(int, int)> JerseyClassifierProvider::getPixelClassifier(const int positiveColor, const std::vector<int> negativeColors, const int maxBrightness) const
{
  std::vector<std::function<bool(int, int)>> classifiers;
  classifiers.reserve(negativeColors.size());
  for(auto negativeColor : negativeColors)
  {
    classifiers.push_back(getPixelClassifier(positiveColor, negativeColor, maxBrightness));
  }

  return [this, classifiers](const int x, const int y)
    {
      // If any classifier is false, pixel is not positiveColor
      for(auto classifier : classifiers)
      {
        if (!classifier(x,y)) return false;
      }
      return true;
    };
}

std::function<bool(int, int)> JerseyClassifierProvider::getPixelClassifier(const std::vector<int> positiveColors, const std::vector<int> negativeColors, const int maxBrightness) const
{
  std::vector<std::function<bool(int, int)>> classifiers;
  classifiers.reserve(negativeColors.size());
  for(int positiveColor : positiveColors)
  {
    classifiers.push_back(getPixelClassifier(positiveColor, negativeColors, maxBrightness));
  }

  return [this, classifiers](const int x, const int y)
    {
      // If any classifier is true, pixel is one of positiveColors
      for(auto classifier : classifiers)
      {
        if (classifier(x,y)) return true;
      }
      return false;
    };
}

std::function<bool(int, int)> JerseyClassifierProvider::getPixelClassifier(const int fieldPlayerColour, const int otherColor, const int maxBrightness) const
{
  const int teamHue = jerseyHues[fieldPlayerColour];
  const int otherHue = jerseyHues[otherColor];
  const Rangei grayRange = Rangei(static_cast<int>(maxBrightness * this->grayRange.min),
                                  static_cast<int>(maxBrightness * this->grayRange.max));

  // If gray is involved, different brightnesses must be distinguished.
  if(fieldPlayerColour == TEAM_GRAY)
    return [this, grayRange](const int x, const int y)
    {
      const unsigned char saturation = theECImage.saturated[y][x];
      return saturation < colorDelimiter && grayRange.isInside(theECImage.grayscaled[y][x]);
    };
  else if(fieldPlayerColour == TEAM_BLACK && (otherColor == TEAM_GRAY || otherColor == TEAM_WHITE))
    return [this, grayRange](const int x, const int y)
    {
      return theECImage.saturated[y][x] < colorDelimiter && theECImage.grayscaled[y][x] < grayRange.min;
    };
  else if(fieldPlayerColour == TEAM_WHITE && (otherColor == TEAM_BLACK || otherColor == TEAM_GRAY))
    return [this, grayRange](const int x, const int y)
    {
      return theECImage.saturated[y][x] < colorDelimiter && theECImage.grayscaled[y][x] > grayRange.max;
    };

  // Black or white against color
  else if(fieldPlayerColour == TEAM_BLACK)
    return [this, grayRange, otherHue](const int x, const int y)
    {
      return theECImage.saturated[y][x] < satThreshold && theECImage.grayscaled[y][x] <= grayRange.min &&
             std::abs(static_cast<char>(theECImage.hued[y][x] - otherHue)) > hueSimilarityThreshold;
    };
  else if(fieldPlayerColour == TEAM_WHITE)
    return [this, grayRange, otherHue](const int x, const int y)
    {
      return theECImage.saturated[y][x] < satThreshold && theECImage.grayscaled[y][x] >= grayRange.max &&
             std::abs(static_cast<char>(theECImage.hued[y][x] - otherHue)) > hueSimilarityThreshold;
    };

  // This team uses a color, the other team does not.
  else if(otherColor == TEAM_WHITE || otherColor == TEAM_BLACK || otherColor == TEAM_GRAY)
  {
    return [this, teamHue](const int x, const int y)
    {
      return std::abs(static_cast<char>(theECImage.hued[y][x] - teamHue)) <= hueSimilarityThreshold;
    };
  }
  // Both teams use colors, use the closer one.
  else
    return [this, teamHue, otherHue](const int x, const int y)
    {
      const int hue = theECImage.hued[y][x];
      // Casting to char should resolve the wraparound cases.
      return std::abs(static_cast<char>(hue - teamHue)) < std::min(std::abs(static_cast<char>(hue - otherHue)), hueSimilarityThreshold);
    };
}


//HUETCHEN addition start2 
int JerseyClassifierProvider::detectJerseyColor(const ObstaclesImagePercept::Obstacle& obstacleInImage) const
{
  // Bestimme Jersey-Region (gleich wie in detectJersey)
  const int width = obstacleInImage.right - obstacleInImage.left + 1;
  const float yStart = static_cast<float>(obstacleInImage.top);
  const float yEnd = yStart + (obstacleInImage.bottom - obstacleInImage.top) * 0.6f; // Obere 60%
  
  if(yEnd - yStart < 5) return -1; // Kleinere Schwelle

  // Zähle Hue-Werte für jede Farbe
  std::map<int, int> colorCounts; // TEAM_* -> Anzahl
  int totalSamples = 0;

  const float yStep = 5.f; // Kleinere Steps = mehr Samples
  const float xStep = 5.f;

  for(float y = yStart; y < yEnd && y < theCameraInfo.height; y += yStep)
  {
    for(float x = obstacleInImage.left; x < obstacleInImage.right && x < theCameraInfo.width; x += xStep)
    {
      const int ix = static_cast<int>(x);
      const int iy = static_cast<int>(y);

      // Nur saturierte Pixel analysieren (niedrigere Schwelle)
      if(theECImage.saturated[iy][ix] > 40)
      {
        const unsigned char hue = theECImage.hued[iy][ix];
        
        // Finde nächste Farbe aus jerseyHues[]
        int closestColor = -1;
        int minDist = 256;
        
        for(int color = TEAM_BLUE; color <= TEAM_BROWN; ++color)
        {
          // Überspringe Grautöne
          if(color == TEAM_BLACK || color == TEAM_WHITE || color == TEAM_GRAY)
            continue;
          
          const int colorHue = jerseyHues[color];
          // Berücksichtige Hue-Wraparound (0-255)
          const int dist = std::min(std::abs(static_cast<int>(hue) - colorHue), 
                                   256 - std::abs(static_cast<int>(hue) - colorHue));
          
          // Nutze größeren Threshold als hueSimilarityThreshold für mehr Toleranz
          if(dist < minDist && dist < hueSimilarityThreshold + 10)
          {
            minDist = dist;
            closestColor = color;
          }
        }
        
        if(closestColor != -1)
        {
          colorCounts[closestColor]++;
          totalSamples++;
        }
      }
    }
  }

  // Finde dominante Farbe
  if(totalSamples < 5) return -1; // Niedrigere Schwelle

  int dominantColor = -1;
  int maxCount = 0;
  
  for(const auto& [color, count] : colorCounts)
  {
    if(count > maxCount)
    {
      maxCount = count;
      dominantColor = color;
    }
  }

  // Muss mindestens 25% der Samples sein (niedrigere Schwelle)
  if(maxCount > totalSamples * 0.25f)
    return dominantColor; // Gibt TEAM_RED, TEAM_BLUE, TEAM_PURPLE, etc. zurück!
  
  return -1; // Keine klare Farbe
}

//HUETCHEN addition end2
