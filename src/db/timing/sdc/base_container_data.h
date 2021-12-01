/**
 * @file base_container_data.h
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

#ifndef EDI_DB_TIMING_SDC_BASE_CONTAINER_DATA_H_
#define EDI_DB_TIMING_SDC_BASE_CONTAINER_DATA_H_

#include <memory>
#include "db/timing/sdc/clock.h"

namespace open_edi {
namespace db {

class Sdc;
using SdcPtr = std::shared_ptr<Sdc>;

class ContainerData {
  public:
    ContainerData()=default;
    virtual ~ContainerData()=default;

    SdcPtr &getSdc() { return sdc_; }
    const SdcPtr &getSdc() const { return sdc_; }
    void setSdc(const SdcPtr &sdc) { sdc_ = sdc; }

  private:
    SdcPtr sdc_ = nullptr;
};

class BaseContainerData : public ContainerData {
  public:
    BaseContainerData()=default;
    virtual ~BaseContainerData()=default;

    std::string clockIdToName(const ClockId &clock_id);
};

class ClockBaseContainerData : public ContainerData {
  public:
    ClockBaseContainerData()=default;
    virtual ~ClockBaseContainerData()=default;

    void removeClockFromContainers(const ClockId &clock_id);
};

}
}


#endif // EDI_DB_TIMING_SDC_BASE_CONTAINER_DATA_H
