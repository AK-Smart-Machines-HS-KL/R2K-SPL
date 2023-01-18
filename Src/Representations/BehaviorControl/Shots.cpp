#include "Shots.h"
#include "Tools/Debugging/DebugDrawings.h"

void Shots::draw() const
{
  DECLARE_DEBUG_DRAWING("representation:Shots", "drawingOnField");
  CIRCLE("representation:Shots", goalShot.target.x(), goalShot.target.y(), 75, 0, Drawings::solidPen, ColorRGBA::black, Drawings::solidBrush, ColorRGBA::green);
}