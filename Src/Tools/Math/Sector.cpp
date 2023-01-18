#include "Sector.h"

    Angle min;
    Angle max;

    Sector::Sector(const Angle bound1, const Angle bound2, const Angle inside) {
      if(isInside(inside, bound1, bound2)) {
        min = bound1;
        max = bound2;
      } else {
        min = bound2;
        max = bound1;
      }
    }
    Sector Sector::centeredSector(Angle mid, Angle size) {
      return Sector(mid - size / 2, mid + size / 2);
    }

    bool Sector::isInside(Angle angle) const {
      return isInside(angle, min, max);
    }

    void Sector::invert() {
        float tmp = min;
        min = max;
        max = tmp;
    }

    Angle Sector::getCenter() const {
      if(min < max) {
        return (min + max) / 2;
      } else {
        return Angle::normalize((min + max) / 2 + 180_deg);
      }
    }

    float Sector::getSize() const {
      if(min < max) {
        return max - min;
      } else {
        return min.diffAbs(max);
      }
    }

    bool Sector::isInside(Angle angle, Angle min, Angle max)
    {
      Angle normAngle = Angle::normalize(angle);
      if(min < max) {
        return normAngle > min && normAngle < max;
      } else {
        return normAngle > min || normAngle < max;
      }
    }

  void SectorList::addSector(const Sector& newSect) 
  {
    for(auto iter = begin(); iter != end(); iter++)
    {
      Sector& localSect = *iter;
      bool minInside = localSect.isInside(newSect.min);
      bool maxInside = localSect.isInside(newSect.max);
      if(minInside) {
        if (maxInside) {
          if (newSect.isInside(localSect.min)) { // List becomes single full sector
            clear();
            push_back(Sector(-pi,pi - 0.000001f));
          } 
          return;
        } else { // newSect clips localSect on the MaxSide
          erase(iter);
          addSector(Sector(localSect.min, newSect.max));
          return;
        }
      } else {
        if (maxInside) { // newSect clips localSect on the minSide
            erase(iter);
            addSector(Sector(newSect.min, localSect.max));
            return;  
          
        } else if (newSect.isInside(localSect.min)) // localSect contained in newSect 
        {
            iter = erase(iter); // Remove LocalSect and continue adding
        } 
      }
    } // no intersection with other sectors -> add
    push_back(newSect);
  }

  void SectorList::subtractSector(const Sector& sect) 
  {
    for(auto iter = begin(); iter != end(); iter++)
    {
      Sector& localSect = *iter;
      bool minInside = localSect.isInside(sect.min);
      bool maxInside = localSect.isInside(sect.max);
      if(minInside) {
        if (maxInside) {
          if (sect.isInside(localSect.min)) { // List becomes single full sector
            clear();
            push_back(Sector(sect.max, sect.min));
            return;
          } else {
            push_back(Sector(localSect.min, sect.min));
            push_back(Sector(sect.max, localSect.max));
            erase(iter);
            iter = begin();
          }
        } else {
          push_back(Sector(localSect.min, sect.min));
          erase(iter);
          iter = begin();
        }
      } else {
        if (maxInside) {
          push_back(Sector(sect.max, localSect.max));
          erase(iter);
          iter = begin();
        } else {
          if (sect.isInside(localSect.min)) { 
            erase(iter);
            iter = begin();
          } else if (localSect.isInside(sect.max)) { // New sector is already contained in localSect
            return;
          }
        }
      }
    }
  }

  void SectorList::join(const SectorList& other) {
    for(auto& sector : other)
    {
      addSector(sector);
    }
  }
