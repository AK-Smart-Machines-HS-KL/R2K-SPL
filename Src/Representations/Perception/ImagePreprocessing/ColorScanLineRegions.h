/**
 * @author Felix
 * @author <a href="mailto:jesse@tzi.de">Jesse Richter-Klug</a>
 */

#pragma once

#include "Tools/ImageProcessing/PixelTypes.h"
#include "Tools/Streams/AutoStreamable.h"

using PixelTypes::Color;

union ScanLineRange
{
  ScanLineRange() = default;
  ScanLineRange(const unsigned short from, const unsigned short to) : from(from), to(to) {}

  struct
  {
    unsigned short from; // upper/left; inclusive
    unsigned short to; // lower/right; exclusive
  };
  struct
  {
    unsigned short upper; // inclusive
    unsigned short lower; // exclusive
  };
  struct
  {
    unsigned short left; // inclusive
    unsigned short right; // exclusive
  };
};

struct ScanLineRegion : public Streamable
{
  ScanLineRegion() = default;
  ScanLineRegion(const unsigned short from, const unsigned short to, const Color c);

  ScanLineRange range;
  Color color;

  static void reg()
  {
    PUBLISH(reg);
    REG_CLASS(ScanLineRegion);
    REG(range.from);
    REG(range.to);
    REG(color);
  }

protected:
  /**
   * Read this object from a stream.
   * @param stream The stream from which the object is read.
   */
  void read(In& stream) override
  {
    STREAM(range.from);
    STREAM(range.to);
    STREAM(color);
  }

  /**
   * Write this object to a stream.
   * @param stream The stream to which the object is written.
   */
  void write(Out& stream) const override
  {
    STREAM(range.from);
    STREAM(range.to);
    STREAM(color);
  }
};

inline ScanLineRegion::ScanLineRegion(const unsigned short from, const unsigned short to, const Color c) :
  range(from, to), color(c)
{}

STREAMABLE(ColorScanLineRegionsVertical,
{
  STREAMABLE(ScanLine,
  {
    ScanLine() = default;
    ScanLine(const unsigned short x);
    ,
    (unsigned short)(0) x,
    (std::vector<ScanLineRegion>) regions,
  });

  void draw() const,

  (std::vector<ScanLine>) scanLines,
  (unsigned)(0) lowResStart, /**< First index of low res scanLines. */
  (unsigned)(1) lowResStep,  /**< Steps between low res scanLines. */
});

inline ColorScanLineRegionsVertical::ScanLine::ScanLine(const unsigned short x) :
  x(x), regions()
{
  regions.reserve(100);
}

/**
 * A version of the ColorScanLineRegionsVertical that has been clipped at the FieldBoundary
 */
STREAMABLE_WITH_BASE(ColorScanLineRegionsVerticalClipped, ColorScanLineRegionsVertical,
{
  void draw() const,
});

/**
 * A version of the ColorScanLineRegionsVertical which is scaled (linear interpolated?)
 */
STREAMABLE_WITH_BASE(CompressedColorScanLineRegionsVertical, ColorScanLineRegionsVertical,
{
  void draw() const,
});

STREAMABLE(ColorScanLineRegionsHorizontal,
{
  STREAMABLE(ScanLine,
  {
    ScanLine() = default;
    ScanLine(const unsigned short y);
    ,
    (unsigned short)(0) y,
    (std::vector<ScanLineRegion>) regions,
  });

  void draw() const,

  (std::vector<ScanLine>) scanLines,
});

inline ColorScanLineRegionsHorizontal::ScanLine::ScanLine(const unsigned short y) :
  y(y), regions()
{
  regions.reserve(100);
}
