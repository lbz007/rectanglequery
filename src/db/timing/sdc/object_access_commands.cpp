/**
 * @file object_access_commands.cpp
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

#include "db/timing/sdc/object_access_commands.h"
#include "db/core/db.h"

namespace open_edi {
namespace db {

AllInputs::AllInputs() {
    level_sensitive_ = false;
    edge_triggered_ = false;
}

AllOutputs::AllOutputs() {
    level_sensitive_ = false;
    edge_triggered_ = false;
}

AllRegisters::AllRegisters() {
    no_hierarchy_ = false;
    cells_ = false;
    data_pins_ = false;
    clock_pins_ = false;
    slave_clock_pins_ = false;
    async_pins_ = false;
    output_pins_ = false;
    level_sensitive_ = false;
    edge_triggered_ = false;
    master_slave_ = false;
}

CurrentDesign::CurrentDesign() {
    init();
}

void CurrentDesign::init() {
    Cell *top_cell = getTopCell();
    if (!top_cell->getNumOfTerms() and !top_cell->getNumOfCells()) { //No Design Data
        return;
    }
    cell_id_ = top_cell->getId();
}

SwitchCurrentDesignResult CurrentDesign::cd(const std::string &hier_cell_name) {
    if (hier_cell_name == "") {
        return SwitchCurrentDesignResult::kSuccessToOriginCell;
    }
    Cell *top_cell = getTopCell();
    Cell *hier_cell = top_cell->getCell(hier_cell_name);
    if (!hier_cell) {
        return SwitchCurrentDesignResult::kFailedAsNewCellIsInvalid;
    }
    if (!(hier_cell->isHierCell())) {
        return SwitchCurrentDesignResult::kFailedAsNewCellNotHier;
    }
    cell_id_ = hier_cell->getId();
    return SwitchCurrentDesignResult::kSuccessToNewCell;
}

GetCells::GetCells() {
    hierarchical_ = false;
    regexp_ = false;
    nocase_ = false;
}

GetClocks::GetClocks() {
    regexp_ = false;
    nocase_ = false;
}

GetLibCells::GetLibCells() {
    regexp_ = false;
    nocase_ = false;
}

GetLibPins::GetLibPins() {
    regexp_ = false;
    nocase_ = false;
}

GetLibs::GetLibs() {
    regexp_ = false;
    nocase_ = false;
}

GetNets::GetNets() {
    hierarchical_ = false;
    regexp_ = false;
    nocase_ = false;
}

GetPins::GetPins() {
    hierarchical_ = false;
    regexp_ = false;
    nocase_ = false;
}

GetPorts::GetPorts() {
    regexp_ = false;
    nocase_ = false;
}


}
}

