/* @file  wire_lenght.h
 * @date  Oct 2020
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NiiCEDA Inc.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
/*************************************************************************/                         
#ifndef EDI_WIRE_LENGTH_H_
#define EDI_WIRE_LENGTH_H_

#include <vector>
#include <limits>
#include "util/util.h"
#include "utility/src/Msg.h"
#include "flow/src/common_place_DB.h"

DREAMPLACE_BEGIN_NAMESPACE
enum WLType 
{
  kHPWL = 0,
  kMST = 1,
  kFLUTE = 2,
};

static std::string kWLTypeName[3] = {"HPWL", "MST", "FLUTE"};

class WireLength
{
  public:
    WireLength(bool gpu, WLType type) : gpu_(gpu), type_(type)
    {}
    ~WireLength()
    {}
    WireLength(const WireLength&)            = delete; // no copy
    WireLength &operator=(const WireLength&) = delete; // no copy

    WLType      getType()              const { return type_;   }
    bool        isGPU()		       const { return gpu_;    }
    bool        isDBReady()            const { return (db_ && db_->isCommonDBReady()); }
    PlInt       calcNetHPWLInCPU(PlNet* net);
    double      calcHPWLInCPU();
    double      calcHPWLInGPU();
    double      calcHPWL();
    double      calcMST();
    double      calcFLUTE();
    double calcFLUTECPU();

    void run();  // flow run
  private:
    
    CommonPlaceDB* db_ = nullptr;
    WLType type_       = kHPWL;
    bool gpu_          = false;
};


DREAMPLACE_END_NAMESPACE

#endif // EDI_WIRE_LENGTH_H_

