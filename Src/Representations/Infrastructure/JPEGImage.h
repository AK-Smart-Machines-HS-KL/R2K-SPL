/**
 * @file JPEGImage.h
 *
 * Declaration of struct JPEGImage
 */

#pragma once

#include "Representations/Infrastructure/CameraImage.h"
#include "Tools/Streams/Streamable.h"

/**
 * Definition of a struct for JPEG-compressed images.
 */
struct JPEGImage : public Streamable
{
private:
  unsigned size; /**< The size of the compressed image. */
  int width; /**< The width of the image in pixel */
  int height; /**< The height of the image in pixel */
  std::vector<unsigned char> allocator; /**< The data storage */

public:
  JPEGImage() = default;

  /**
   * Constructs a JPEG image from an image.
   * @param src The image used as template.
   */
  JPEGImage(const CameraImage& src);

  /**
   * Assignment operator.
   * @param src The image used as template.
   * @return The resulting JPEG image.
   */
  JPEGImage& operator=(const CameraImage& src);

  /**
   * Uncompress image.
   * @param dest Will receive the uncompressed image.
   */
  void toCameraImage(CameraImage& dest) const;

  unsigned timestamp = 0; /**< The timestamp of this image. */

protected:
  /**
   * Read this object from a stream.
   * @param stream The stream from which the object is read.
   */
  void read(In& stream) override;

  /**
   * Write this object to a stream.
   * @param stream The stream to which the object is written.
   */
  void write(Out& stream) const override;

private:
  static void reg();
};
