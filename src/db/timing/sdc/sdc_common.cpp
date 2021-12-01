/**
 * @file sdc_common.cpp
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


#include "db/timing/sdc/sdc_common.h"
#include "db/timing/timinglib/analysis_view.h"
#include "db/timing/timinglib/libset.h"
#include "db/timing/timinglib/timinglib_term.h"
#include "db/timing/timinglib/timinglib_units.h"
#include "db/timing/timinglib/timinglib_lib.h"

namespace open_edi {
namespace db {

TLib* getMainLiberty() {
    const Timing *timing_db = getTimingLib();
    if (!timing_db) {
        message->issueMsg("SDC", 1, open_edi::util::kError);
        return nullptr;
    }
    const uint64_t view_num = timing_db->getNumOfAnalysisViews();
    if (view_num == 0) {
        message->issueMsg("SDC", 32, open_edi::util::kError);
        return nullptr;
    }
    constexpr size_t first_id = 0;
    AnalysisView *first_view = timing_db->getAnalysisView(first_id);
    if (!first_view) {
        message->issueMsg("SDC", 32, open_edi::util::kError);
        return nullptr;
    }
    AnalysisCorner *corner = first_view->getAnalysisCorner();
    if (!corner) {
        message->issueMsg("SDC", 30, open_edi::util::kError, (first_view->getName()).c_str());
        return nullptr;
    }
    LibSet* lib_set = corner->getLibset();
    if (!lib_set) {
        message->issueMsg("SDC", 31, open_edi::util::kWarn, (corner->getName()).c_str());
        return nullptr;
    }
    const std::vector<TLib*>& lib_vec = lib_set->getTimingLibs();
    if (lib_vec.empty()) {
        message->issueMsg("SDC", 31, open_edi::util::kWarn, (corner->getName()).c_str());
        return nullptr;
    }
    TLib* lib =  lib_vec.front();
    if (!lib) {
        message->issueMsg("SDC", 31, open_edi::util::kWarn, (corner->getName()).c_str());
        return nullptr;
    }
    return lib;
}

TUnits* getMainLibertyUnit() {
    TLib* lib = getMainLiberty();
    if (!lib) {
        return nullptr;
    }
    return lib->getUnits();
}

Pin* getPinByFullName(const std::string &full_name) {
    //TODO
    //Maybe separator is not "/"
    const auto &pos = full_name.find_last_of('/');
    const auto &top_cell = getTopCell();
    if (pos == std::string::npos) { // port
        return top_cell->getIOPin(full_name);
    }
    const auto &inst_name = full_name.substr(0, pos);
    const auto &pin_name = full_name.substr(pos+1);
    const auto &inst = top_cell->getInstance(inst_name);
    if (!inst) {
        return nullptr;
    }
    return inst->getPin(pin_name);
}

bool isCellInOtherCell(const ObjectId &cell_id, const ObjectId &other_cell_id) {
    if ((cell_id == UNINIT_OBJECT_ID) or (other_cell_id == UNINIT_OBJECT_ID)) {
        return false;
    }
    if (cell_id == other_cell_id) {
        return true;
    }
    const Cell *top_cell = getTopCell();
    const ObjectId &top_cell_id = top_cell->getId();
    if (other_cell_id == top_cell_id) {
        return true;
    }
    const Cell *cell = Object::addr<Cell>(cell_id);
    assert(cell);
    if (!cell) {
        // error messages (Could not find Cell cell_id in db)
        return false;
    }
    ObjectId up_cell_id = cell->getOwnerId();
    while (up_cell_id != top_cell_id) {
        if (up_cell_id == other_cell_id) {
            return true;
        }
        up_cell_id = cell->getOwnerId();
    }
    return false;
}

bool isInstInCell(const ObjectId &inst_id, const ObjectId &cell_id) {
    Inst* inst = Object::addr<Inst>(inst_id);
    if (!inst) {
        // error messages (Could not find Inst inst_id in db)
        return false;
    }
    Cell *master_cell = inst->getMaster();
    assert(master_cell);
    bool ret = isCellInOtherCell(master_cell->getId(), cell_id);
    return ret;
}

TCell* getLibCellInCorner(AnalysisCorner *analysis_corner, const std::string &cell_name, std::string liberty_name) {
    if (!analysis_corner) {
        //TODO messages (analysis corner is not avaliable)
        return nullptr;
    }
    LibSet* lib_set = analysis_corner->getLibset();
    assert(lib_set);
    if (!lib_set) {
        //TODO messages (Could not find any liberty in analysis corner analysis_corner->getName())
        return nullptr;
    }
    const std::vector<TLib*>& lib_vec = lib_set->getTimingLibs();
    for (const auto &lib : lib_vec) {
        if (!lib) {
            //TODO messages (Null liberty was found in analysis corner analysis_corner->getName())
            continue;
        }
        if (liberty_name != "") {
            if (lib->getName() == liberty_name) {
                return lib->getTimingCell(cell_name);
            }
        } else {
            TCell *cell = lib->getTimingCell(cell_name);
            if (cell) {
                return cell;
            }
        }
    }
    return nullptr;
}

void getSdcFileContents(std::string &contents, const std::vector<std::string> &files,  const std::string &view_name) {
    contents = "";
    contents.reserve(50000);
    std::string line = "";
    for (const std::string &file : files) {
        std::ifstream input(file);
        if (!input) {
            //TODO messages
            continue;
        }
        while (std::getline(input, line)) {
            boost::trim(line);
            if (line.empty() or line[0] == '#') {
                continue;
            }
            contents += line + " -view " + view_name + " \n";
        }
    }
}

bool ContainerDataAccess::setToPin(ObjectId &pin_id, const std::string &pin_name) {
    const auto &pin = getPinByFullName(pin_name);
    if (!pin) {
        return false;
    }
    pin_id = pin->getId();
    return true;
}

std::string ContainerDataPrint::addDoubleQuotes(const std::string &str) {
    if (str.empty()) {
        return "";
    }
    return "\"" + str + "\"";
}

std::string ContainerDataPrint::pinIdToName(const ObjectId &pin_id) {
    return idToName<Pin>(pin_id);
}

std::string ContainerDataPrint::termIdToName(const ObjectId &term_id) {
    return idToName<Term>(term_id);
}

std::string ContainerDataPrint::cellIdToName(const ObjectId &cell_id) {
    return "[get_designs " + idToName<Cell>(cell_id) + " ]";
}

std::string ContainerDataPrint::ttermIdToName(const ObjectId &tterm_id) {
    return idToName<TTerm>(tterm_id);
}

std::string ContainerDataPrint::tcellIdToName(const ObjectId &tcell_id) {
    return "[get_lib_cells " + idToName<TCell>(tcell_id) + " ]";
}

std::string ContainerDataPrint::instIdToName(const ObjectId &inst_id) {
    return "[get_cells " + idToName<Inst>(inst_id) + " ]";
}

std::string ContainerDataPrint::getPinOrPortName(bool &is_port, const ObjectId &pin_id) {
    //TODO Maybe separator is not "/"
    Pin* pin = Object::addr<Pin>(pin_id);
    if (!pin) {
        // error messages (Could not find pin_id in db)
        return "";
    }
    const std::string &pin_name = pin->getName();
    is_port = pin->getIsIOPin();
    if (is_port) {
        return pin_name;
    }
    const Inst *inst = pin->getInst();
    if (!inst) {
        // error messages (Pin pin_name did not have inst in db)
        return "";
    }
    const std::string &inst_name = inst->getName();
    const std::string &pin_full_name = inst_name + "/" + pin_name;
    return pin_full_name;
}

std::string ContainerDataPrint::pinIdToFullName(const ObjectId &pin_id) {
    bool is_port = false;
    const std::string &pin_name = getPinOrPortName(is_port, pin_id);
    if (pin_name.empty()) {
        return "";
    }
    std::string ret = "";
    if (is_port) {
        ret = "[get_ports " + pin_name + " ]";
    } else {
        ret = "[get_pins " + pin_name + " ]";
    }
    return ret;
}

std::string ContainerDataPrint::ttermIdToFullName(const ObjectId &tterm_id) {
    //TODO Maybe separator is not "/"
    TTerm* tterm = Object::addr<TTerm>(tterm_id);
    if (!tterm) {
        // error messages (Could not find tterm_id in db)
        return "";
    }
    const std::string &tterm_name = tterm->getName();
    const TCell* tcell = tterm->getTCell();
    if (!tcell) {
        // error messages (TTerm did not have tcell in db)
        return "";
    }
    const std::string &tcell_name = tcell->getName();
    const std::string &tterm_full_name = tcell_name + "/" + tterm_name;
    return "[get_lib_pins " + tterm_full_name + " ]";
}

std::string ContainerDataPrint::getClockPrintName(const std::string &clock_name) {
    if (clock_name.empty()) {
        return "";
    }
    return "[get_clocks " + clock_name + " ]";
}

std::string ContainerDataPrint::netIdToName(const ObjectId &net_id) {
    const std::string &net_name = idToName<Net>(net_id);
    if (net_name.empty()) {
        return "";
    }
    return "[get_nets " + net_name + " ]";
}

std::string ContainerDataPrint::libIdToName(const ObjectId &lib_id) {
    const std::string &lib_name = idToName<TLib>(lib_id);
    if (lib_name.empty()) {
        return "";
    }
    return "[get_libs " + lib_name + " ]";
}

std::string ContainerDataPrint::getFlag(const Flag &p) {
    const std::string &flag_str = p.first;
    const bool &flag = p.second;
    return flag ? (flag_str + " ") : "";
}

std::string ContainerDataPrint::getFlagValue(const FlagValue &p) {
    const std::string &flag_str = p.first;
    const std::string &object_str = p.second;
    return object_str.empty() ? "" : (flag_str + " " + object_str + " ");
}


}
}
