/**
 * @file PatchUtilities.h
 * This file implements helper functions for processing image patches
 * @author Bernd Poppinga
 */

#pragma once

#include "Representations/Infrastructure/CameraImage.h"
#include "Tools/ImageProcessing/Image.h"
#include "Tools/Math/Eigen.h"
#include "Tools/Streams/Enum.h"

class PatchUtilities
{
public:
  ENUM(ExtractionMode,
  {,
    fast,
    fastInterpolated,
    interpolated,
  });

  template<typename OutType>
  static void normalizeContrast(OutType* output, const Vector2i& size, const float percent = 0.02f);
  static void normalizeContrast(GrayscaledImage& output, const float percent = 0.02f);

  template<typename OutType>
  static void extractPatch(const Vector2i& center, const Vector2i& inSize, const Vector2i& outSize, const GrayscaledImage& src, OutType* dest, const ExtractionMode mode = fast);
  static void extractPatch(const Vector2i& center, const Vector2i& inSize, const Vector2i& outSize, const GrayscaledImage& src, GrayscaledImage& dest, const ExtractionMode mode = fast);

  // This methods only work correctly if the image dimensions are multiples of the patch size.
  template<typename OutType, bool grayscale>
  static void extractInput(const CameraImage& cameraImage, const Vector2i& patchSize, OutType* input);
  static void extractInput(const CameraImage& cameraImage, const Vector2i& patchSize, std::uint8_t* input);

private:
  template<typename OutType, bool interpolate = false>
  static void getImageSection(const Vector2i& center, const Vector2i& inSize, const Vector2i& outSize, const GrayscaledImage& src, OutType* output);

  template<typename OutType>
  static void getInterpolatedImageSection(const Vector2i& center, const Vector2i& inSize, const Vector2i& outSize, const GrayscaledImage& src, OutType* output);

  static Matrix3f calcInverseTransformation(const Vector2i& center, const Vector2i& inSize, const Vector2i& outSize);
};
