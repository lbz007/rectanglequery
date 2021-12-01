/**
* @file base_container_data.cpp
* @date 2020-11-25
* @brief
*
* Copyright (C) 2020 NIIC EDA
*
* All rights reserved.
*
* This software may be modified and distributed under the terms
*
* of the BSD license.  See the LICENSE file for details.
*/

#include "db/timing/sdc/base_container_data.h"
#include "db/timing/sdc/sdc.h"

namespace open_edi {
namespace db {

std::string BaseContainerData::clockIdToName(const ClockId &clock_id) {
    return getSdc()->clockIdToName(clock_id);
}

void ClockBaseContainerData::removeClockFromContainers(const ClockId &clock_id) {
    return getSdc()->removeClockFromContainers(clock_id);
}

}
}