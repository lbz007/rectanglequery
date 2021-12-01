/**
 * @file general_purpose_commands.cpp
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

#include <regex>

#include "db/timing/sdc/general_purpose_commands.h"
#include "db/timing/timinglib/timinglib_units.h"
#include "db/timing/timinglib/timinglib_libbuilder.h"
#include "db/timing/sdc/sdc_common.h"

namespace open_edi {
namespace db {

SwitchInstResult CurrentInstance::cd(const ObjectId &cell_id, const std::string &dir) {
    //TODO
    //Now support:
    //1) "." ---> current_instance
    //2) ".." ---> upper level instance
    //3) "name" ---> down level instance
    //4) "" ---> current design
    //Not support:
    //1) "../inst name" or "../../inst name"
    //2) different separator
    if (dir == "") {
        inst_id_ = UNINIT_OBJECT_ID;
        return SwitchInstResult::kSuccessToCurrentDesign;
    }
    const auto &inst = Object::addr<Inst>(inst_id_);
    std::string new_inst_name = "";
    if (!inst) {
        if (dir == ".") {
            return SwitchInstResult::kSuccessToCurrentDesign;
        }
        new_inst_name = dir;
    } else {
        if (dir == ".") {
            return SwitchInstResult::kSuccessToOriginInst;
        }
        const auto &inst_name = inst->getName();
        if (dir == "..") {
            const auto &pos = inst_name.find_last_of("/");
            if (pos == std::string::npos) {
                return SwitchInstResult::kFailedAsOriginInstNotHier;
            }
            new_inst_name = inst_name.substr(0, pos-1);
        } else {
            new_inst_name = inst_name + "/" + dir;
        }
    }
    auto new_inst = getTopCell()->getInstance(new_inst_name);
    if (!new_inst) {
        return SwitchInstResult::kFailedAsNewInstIsInvalid;
    }
    Cell *master_cell = new_inst->getMaster();
    assert(master_cell);
    if (!(master_cell->isHierCell())) {
        return SwitchInstResult::kFailedAsNewInstNotHier;
    }
    bool is_inst_in_current_design = isCellInOtherCell(master_cell->getId(), cell_id);
    if (!is_inst_in_current_design) {
        return SwitchInstResult::kFailedAsNewInstNotInCurrentDesign;
    }
    inst_id_ = new_inst->getId();
    return SwitchInstResult::kSuccessToNewInst;
}

const std::string SetHierarchySeparator::legal_chars_ = "/@^#.|";
bool SetHierarchySeparator::setAndCheck(const std::string &input) {
    if (input.size() != 1) {
        return false;
    }
    auto found = legal_chars_.find(input);
    if (found == std::string::npos) {
        return false;
    }
    separator_ = input.front();
    return true;
}

bool SetUnits::splitUnit(float &value, std::string &suffix, const std::string &target) {
    static const std::regex re("([0-9]*\\.?[0-9]*)?([a-zA-Z]*)");
    std::smatch sm;
    std::regex_match(target, sm, re);
    if (!sm.ready() or (sm.size()!=3)) {
        //TODO messages ("Regex Not match", target, sm.ready(), sm.size())
        return false;
    }
    try {
        if (!(sm[1].str().empty())) {
            value = std::stof(sm[1].str());
        }
        if (!(sm[2].str().empty())) {
            suffix = sm[2].str();
        }
    } catch (std::exception &ex) {
        //TODO error messages ("Exception caugth as ", ex.what())
        return false;
    }
    return true;
}

bool SetUnits::setAndCheckCapacitance(const std::string &cap) {
    float value = 1.0;
    std::string suffix = "f";
    splitUnit(value, suffix, cap);
    capacitance_unit_value_ = value * UnitMultiply::getCapacitiveUnitMultiply(suffix);
    assert(liberty_units_);
    if (!liberty_units_) {
        //TODO error messages
        return false;
    }
    const auto& lib_cap_unit = liberty_units_->getCapacitanceUnit();
    if (!ediEqual(capacitance_unit_value_ , lib_cap_unit.scale)) {
        //TODO error messages ("Cap unit is mismatch with liberty")
        return false;
    }
    return true;
}

bool SetUnits::setAndCheckResistance(const std::string &res) {
    float value = 1.0;
    std::string suffix = "ohm";
    splitUnit(value, suffix, res);
    resistance_unit_value_ = value * UnitMultiply::getResistanceUnitMultiply(suffix);
    assert(liberty_units_);
    if (!liberty_units_) {
        return false;
    }
    const auto& lib_res_unit = liberty_units_->getResistanceUnit();
    if (!ediEqual(resistance_unit_value_ , lib_res_unit.scale)) {
        //TODO error messages  ("Res unit is mismatch with liberty")
        return false;
    }
    return true;
}

bool SetUnits::setAndCheckTime(const std::string &time) {
    float value = 1.0;
    std::string suffix = "s";
    splitUnit(value, suffix, time);
    time_unit_value_ = value * UnitMultiply::getTimeUnitMultiply(suffix);
    assert(liberty_units_);
    if (!liberty_units_) {
        return false;
    }
    const auto& lib_time_unit = liberty_units_->getTimeUnit();
    if (!ediEqual(time_unit_value_ , lib_time_unit.scale)) {
        //TODO error messages ("Time unit is mismatch with liberty")
        return false;
    }
    return true;
}

bool SetUnits::setAndCheckVoltage(const std::string &voltage) {
    float value = 1.0;
    std::string suffix = "v";
    splitUnit(value, suffix, voltage);
    voltage_unit_value_ = value * UnitMultiply::getVoltageUnitMultiply(suffix);
    assert(liberty_units_);
    if (!liberty_units_) {
        return false;
    }
    const auto& lib_vol_unit = liberty_units_->getVoltageUnit();
    if (!ediEqual(voltage_unit_value_ , lib_vol_unit.scale)) {
        //TODO error messages ("Voltage unit is mismatch with liberty")
        return false;
    }
    return true;
}

bool SetUnits::setAndCheckCurrent(const std::string &current) {
    float value = 1.0;
    std::string suffix = "A";
    splitUnit(value, suffix, current);
    current_unit_value_ = value * UnitMultiply::getCurrentUnitMultiply(suffix);
    assert(liberty_units_);
    if (!liberty_units_) {
        return false;
    }
    const auto& lib_current_unit = liberty_units_->getCurrentUnit();
    if (!ediEqual(current_unit_value_ , lib_current_unit.scale)) {
        //TODO error messages ("Current unit is mismatch with liberty")
        return false;
    }
    return true;
}

bool SetUnits::setAndCheckPower(const std::string &power) {
    float value = 1.0;
    std::string suffix = "w";
    splitUnit(value, suffix, power);
    power_unit_value_ = value * UnitMultiply::getPowerUnitMultiply(suffix);
    assert(liberty_units_);
    if (!liberty_units_) {
        return false;
    }
    const auto& lib_power_unit = liberty_units_->getPowerUnit();
    if (!ediEqual(power_unit_value_ , lib_power_unit.scale)) {
        //TODO error messages ("Power unit is mismatch with liberty")
        return false;
    }
    return true;
}



}
}
