/* @file  wire_length.cpp
 * @date  Nov 2020
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NiiCEDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
/*************************************************************************/                         
#include "wire_length/src/wire_length.h"
#include "utility/src/DetailedPlaceDB.h"
#include <libgen.h>
#include <flute.hpp>

DREAMPLACE_BEGIN_NAMESPACE

double WireLength::calcHPWLInGPU()
{
  double wl = 0.0;
  return wl;
}

PlInt WireLength::calcNetHPWLInCPU(PlNet* net)
{
  PlInt wl = 0;
  PlInt maxX = INT_MIN;
  PlInt minX = INT_MAX;
  PlInt maxY = INT_MIN;
  PlInt minY = INT_MAX;
  forEachNetPins(net) {
    // get pin first box
    BoxV boxes;
    collectPinBoxes(pin, boxes);
    for (auto& box : boxes)
    {
      if (isBoxInvalid(box)) continue;
      PlInt locX = getBoxLLX(box);
      PlInt locY = getBoxLLY(box);
      if (locX > maxX) maxX = locX;
      if (locX < minX) minX = locX;
      if (locY > maxY) maxY = locY;
      if (locY < minY) minY = locY;
      break;
    }
  } endForEachNetPins
  if (maxX >= minX && maxY >= minY) {
    wl = (maxX - minX) + (maxY - minY);
  }
  return wl;
}

double WireLength::calcHPWLInCPU() 
{
  /* Function: calculate HPWL */
  Plong wl = 0;
  int ignoreNetNum = 0;
  bool calcAll = true;
  PlNet* net = nullptr;
  forEachNets(net) {
    if(calcAll || !isNetIgnored(net)) {
      wl += calcNetHPWLInCPU(net); 
    } else {
      ignoreNetNum++;
    }
  } endForEachNets
  dreamplacePrint(kINFO, "Ignored net number: %d, DBU length %lld\n", 
                  ignoreNetNum, wl);

  return dbuToMicrons(wl);
}

double WireLength::calcHPWL() 
{
  if (isGPU()) {
    return calcHPWLInGPU();
  } else {
    return calcHPWLInCPU();
  } 
}

double WireLength::calcFLUTECPU() 
{
  char dest[PATH_MAX];
  memset(dest,0,sizeof(dest)); // readlink does not null terminate!
  if (readlink("/proc/self/exe", dest, PATH_MAX) == -1) {
    perror("readlink");
    return -1;
  } else {
    char* pathCopy = strdup(dest);
    std::string srcdir(dirname(pathCopy));
    std::string powv = srcdir + "/../share/flute/POWV9.dat";
    std::string post = srcdir + "/../share/flute/POST9.dat";
    readLUT(powv.c_str(), post.c_str());
  }
  /* Function: calculate FLUTE */
  long int wl = 0;
  long int hpwl = 0;
  bool calcClk = false;
  PlNet* net = nullptr;
  forEachNets(net) {
    PlInt maxX = INT_MIN;
    PlInt minX = INT_MAX;
    PlInt maxY = INT_MIN;
    PlInt minY = INT_MAX;
    
    if(calcClk || !isNetClock(net)) {
      // temporary store x and y positions 
      std::vector<int> vx;
      std::vector<int> vy; 

      int degree = 0;
      forEachNetPins(net) {
        PlBox pinBox;
        // get pin first box
        BoxV boxes;
        collectPinBoxes(pin, boxes);
        for(auto box: boxes) {
          if(isBoxInvalid(box)) {
            continue;
          } else {
            PlInt locX = getBoxLLX(boxes.at(0));
            vx.push_back(locX);
            PlInt locY = getBoxLLY(boxes.at(0));
            vy.push_back(locY);
            ++degree;

            maxX = (std::max)(maxX, locX);
            maxY = (std::max)(maxY, locY);
            minX = (std::min)(minX, locX);
            minY = (std::min)(minY, locY);
          }
        }
      } endForEachNetPins
    
      if(degree > 1) {
        if(degree >= 100) {
          std::cout<<"ignore net:"<<net->getName()<<" isClk="<<isNetClock(net)<<" d="<<degree<<" xsize="<<vx.size()<<" ysize="<<vy.size()<<std::endl;
          continue;
        }
        int netwl = flute_wl(degree, vx.data(), vy.data(), ACCURACY); 
        int nethpwl = maxX - minX + maxY - minY; 
        //std::cout<<degree<<" "<<netwl<<std::endl;
        wl += netwl;
        hpwl += nethpwl;
      }
    }
  } endForEachNets

  std::cout<<"Flute net hpwl: "<<dbuToMicrons(hpwl)<<std::endl;
  return dbuToMicrons(wl);
}
double WireLength::calcFLUTE() {
  if(isGPU()) {
    //TODO:
    //support calculation on GPU
    dreamplacePrint(kINFO, "Only Support FLUTE wirelength calculation on CPU!");
    return 0;
  } else {
    return calcFLUTECPU();
  }
}

double WireLength::calcMST() 
{
  
  double wl = 0.0; 
  return wl;
}

void WireLength::run() 
{
  /* Function: main to report wire length */
  double total_length = 0.0;
  if (getType() == kHPWL) {
    total_length = calcHPWL();
  } else if (getType() == kMST) {
    total_length = calcMST();
  } else if (getType() == kFLUTE) {
    total_length = calcFLUTE();
  }
  dreamplacePrint(kINFO, "Total wire length(%s) : %g um\n", 
                  kWLTypeName[getType()].c_str(), total_length);
}

DREAMPLACE_END_NAMESPACE
