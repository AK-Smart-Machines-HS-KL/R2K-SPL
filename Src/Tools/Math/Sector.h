/**
 * @file Sector.h
 * @author Andy Hobelsberger (anho1001@stud.hs-kl.de)
 * @brief Defines Sector and SectorList
 * 
 * A sector is defined as a region on a circle bounded by two angles
 * A sector list constains multiple sectors and can add or subtract sectors 
 * 
 * @version 1.0
 * @date 2022-12-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include "Angle.h"
#include <list>

// Draws a sector on WorldView
#define DRAW_SECTOR(drawID, sector, poseOffset) \
        LINE(drawID, \
        poseOffset.translation.x(), poseOffset.translation.y(), \
        (poseOffset.translation + Vector2f(1000,0).rotated(sector.min)).x(), (poseOffset.translation + Vector2f(1000,0).rotated(sector.min)).y(), \
        3, Drawings::dottedPen, ColorRGBA::green); \
        LINE(drawID, \
        poseOffset.translation.x(), poseOffset.translation.y(), \
        (poseOffset.translation + Vector2f(1000,0).rotated(sector.max)).x(), (poseOffset.translation + Vector2f(1000,0).rotated(sector.max)).y(), \
        3, Drawings::dottedPen, ColorRGBA::red);

class Sector
{
public:
    Angle min;
    Angle max;

    Sector(const Angle min, const Angle max) : min(Angle::normalize(min)), max(Angle::normalize(max)) {}
    Sector(const Angle bound1, const Angle bound2, const Angle inside);
    static Sector centeredSector(Angle mid, Angle size);

    bool isInside(Angle angle) const;

    Angle getCenter() const;

    float getSize() const;
    void invert();

    static bool isInside(Angle angle, Angle min, Angle max);

};

class SectorList : public std::list<Sector>
{
  public:
  void addSector(const Sector& sect); 

  void subtractSector(const Sector& sect);

  void join(const SectorList& other);
};
