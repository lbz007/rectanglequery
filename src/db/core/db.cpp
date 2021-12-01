/* @file  db.cpp
 * @date  Aug 2020
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NiiCEDA Inc.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "db/core/db.h"

#include "db/core/object.h"
#include "db/core/root.h"
#include "db/util/array.h"
#include "db/util/symbol_table.h"
#include "util/polygon_table.h"

namespace open_edi {
namespace db {

using Version = open_edi::util::Version;

// top cell contains all objects in library and design
static Cell *kTopCell = nullptr;
static bool kIsTopCellInitialized = false;
static Version kCurrentVersion;
static Root &kRoot = Root::getInstance();

/// @brief resetTopCell:
//    Note currently the root is also reset. might be changed someday.
void resetTopCell() {
    // MemPool::initMemPool();
    kRoot.reset();
    kTopCell = nullptr;
    kCurrentVersion.reset();
}

/// @brief initTopCell
///
/// @return
bool initTopCell() {
    if (kIsTopCellInitialized) return true;

    kRoot.initTechLib();
    kRoot.initTimingLib();
    kRoot.initTopCell();

    kTopCell = kRoot.getTopCell();

    Floorplan *floorplan = kTopCell->createFloorplan();
    if (!floorplan) {
        message->issueMsg("DB", 12, kError);
        return false;
    }

    kIsTopCellInitialized = true;
    kCurrentVersion.init();

    return true;
}

/// @brief getRoot
///
/// @return
Root *getRoot() { return &kRoot; }

/// @brief getTopCell
///
/// @return
Cell *getTopCell() { return kTopCell; }

/// @brief getTechLib
///
/// @return
Tech *getTechLib() { return kRoot.getTechLib(); }

/// @brief getTimingLib
///
/// @return
Timing *getTimingLib() { return kRoot.getTimingLib(); }

/// @brief  setTopCell
///
/// @return
void setTopCell(ObjectId top_cell_id) {
    if (top_cell_id == 0) return;
    kTopCell = (Cell *)Object::addr<Cell>(top_cell_id);
    kRoot.setTopCell(kTopCell);
}

/// @brief  setTechLib
///
/// @return
void setTechLib(ObjectId tech_lib_id) {
    if (tech_lib_id == 0) return;
    Tech *tech = (Tech *)Object::addr<Tech>(tech_lib_id);
    kRoot.setTechLib(tech);
}

/// @brief  setTimingLib
///
/// @return
void setTimingLib(ObjectId timing_lib_id) {
    if (timing_lib_id == 0) return;
    Timing *timing = (Timing *)Object::addr<Timing>(timing_lib_id);
    kRoot.setTimingLib(timing);
}

/// @brief getCurrentVersion
///
/// @return
Version &getCurrentVersion() { return kCurrentVersion; }

/// @brief  setCurrentVersion
///
/// @return
void setCurrentVersion(Version &v) { kCurrentVersion.set(v); }

/// @brief  getCell
///
/// @return
Cell *getCell() {
    Cell *cell = getTopCell();
    return cell;
}

/**
 * @brief reportDesign()
 *
 */
void reportDesign() {
    Cell *cell = nullptr;
    Inst *inst = nullptr;
    Pin *pin = nullptr;
    Net *net = nullptr;
    Wire *wire = nullptr;
    SpecialNet *snet = nullptr;

    ArrayObject<ObjectId> *arr_ptr = nullptr, *arr_ptr1 = nullptr,
                          *arr_ptr2 = nullptr;
    ArrayObject<ObjectId>::iterator iter, iter1;

    cell = getTopCell();
    if (cell == nullptr) return;

    int64_t inst_nr = 0, pin_nr = 0;
    int64_t wire_nr = 0, net_nr = 0, via_nr = 0, wire_node_nr = 0;
    int64_t snet_nr = 0, swire_nr = 0, svia_nr = 0, swire_node_nr = 0;

    arr_ptr = Object::addr<ArrayObject<ObjectId> >(cell->getInstances());
    for (iter = arr_ptr->begin(); iter != arr_ptr->end(); iter++) {
        inst = Object::addr<Inst>(*iter);
        if (nullptr == inst || !inst->getIsValid()) continue;
        inst_nr++;
        arr_ptr1 = inst->getPinArray();
        if (arr_ptr1 == nullptr) continue;
        for (iter1 = arr_ptr1->begin(); iter1 != arr_ptr1->end(); iter1++) {
            pin = Object::addr<Pin>(*iter);
            if (nullptr == pin || !pin->getIsValid()) continue;
            pin_nr++;
        }
    }

    arr_ptr = cell->getNetArray();
    for (iter = arr_ptr->begin(); iter != arr_ptr->end(); iter++) {
        net = Object::addr<Net>(*iter);
        if (!net) continue;
        net_nr++;
        // counting wire
        ArrayObject<ObjectId> *wire_array = net->getWireArray();
        if (wire_array) {
            wire_nr += wire_array->getSize();
        }
        // counting via
        ArrayObject<ObjectId> *via_array = net->getViaArray();
        if (via_array) {
            via_nr += via_array->getSize();
        }
    }

    arr_ptr = cell->getSpecialNetArray();
    for (iter = arr_ptr->begin(); iter != arr_ptr->end(); iter++) {
        snet = Object::addr<SpecialNet>(*iter);
        if (!snet) continue;
        snet_nr++;
        arr_ptr1 = snet->getWireArray();
        // counting swire
        ArrayObject<ObjectId> *swire_array = snet->getWireArray();
        if (swire_array) {
            swire_nr += swire_array->getSize();
        }
        // counting svia
        ArrayObject<ObjectId> *svia_array = snet->getViaArray();
        if (svia_array) {
            svia_nr += svia_array->getSize();
        }
    }

    message->info("\ninst: %-10d\t inst_pin: %-10d\n", inst_nr, pin_nr);
    message->info("net: %-11d\t wire: %-11d \tvia:%-11d\n", net_nr, wire_nr, via_nr);
    message->info("snet: %-10d\t swire: %-10d \tsvia :%-10d \n", snet_nr, swire_nr,
                  svia_nr);
}  // namespace db

}  // namespace db
}  // namespace open_edi
