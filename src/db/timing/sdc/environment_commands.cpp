/**
 * @file environment_commands.cpp
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

#include "db/timing/sdc/environment_commands.h"
#include "db/timing/timinglib/timinglib_term.h"
#include "db/timing/timinglib/timinglib_timingarc.h"

namespace open_edi {
namespace db {

std::string toString(const CaseValue &value) {
    switch (value) {
        case CaseValue::kZero:
            return "zero";
        case CaseValue::kOne:
            return "one";
        case CaseValue::kRise:
            return "rise";
        case CaseValue::kFall:
            return "fall";
        defalut:
            return "unknown";
    }
    return "unknown";
}

bool SetCaseAnalysis::setValue(std::string& input) {
    if (input=="0" or input=="zero") {
        value_ = CaseValue::kZero;
        return true;
    }
    if (input=="1" or input=="one") {
        value_ = CaseValue::kOne;
        return true;
    }
    if (input=="rising" or input=="rise") {
        value_ = CaseValue::kRise;
        return true;
    }
    if (input=="falling" or input=="fall") {
        value_ = CaseValue::kFall;
        return true;
    }
    value_ = CaseValue::kUnknown;
    return false;
}

bool SetCaseAnalysis::operator==(const SetCaseAnalysis &rhs) const {
    bool ret = (value_ == rhs.getValue());
    return ret;
}

bool SetCaseAnalysis::is_constant() const {
    return (value_ == CaseValue::kOne) or (value_ == CaseValue::kZero);
}

SetDrive::SetDrive() {
    rise_ = false;
    fall_ = false;
    min_ = false;
    max_ = false;
}

bool SetDrive::operator==(const SetDrive &rhs) const {
    bool ret =  (rise_ == rhs.getRise()) and (fall_ == rhs.getFall()) and
                (min_ == rhs.getMin()) and (max_ == rhs.getMax());
    return ret;
}

SetDrivingCellCondition::SetDrivingCellCondition() {
    rise_ = false;
    fall_ = false;
    dont_scale_ = false;
    no_design_rule_ = false;
    min_ = false;
    max_ = false;
}

bool SetDrivingCellCondition::operator==(const SetDrivingCellCondition &rhs) const {
    bool ret =  (rise_ == rhs.getRise()) and (fall_ == rhs.getFall()) and (min_ == rhs.getMin()) and (max_ == rhs.getMax()) and
                (dont_scale_ == rhs.getDontScale()) and (no_design_rule_ == rhs.getNoDesignRule());
    return ret;
}

bool SetDrivingCell::checkArc(TCell *tcell, const std::string &from_tterm_name, const std::string &to_tterm_name) {
    TTerm* from_tterm = tcell->getTerm(from_tterm_name);
    TTerm* to_tterm = tcell->getTerm(to_tterm_name);
    const std::vector<TTerm *> &all_tterms = tcell->getTerms();
    if (from_tterm and to_tterm) {
        bool has_arc = to_tterm->hasTimingArcWithTTerm(from_tterm->getId());
        if (!has_arc) {
            return false;
        }
        this->from_tterm_ = from_tterm->getId();
        this->to_tterm_ = to_tterm->getId();
    } else if (from_tterm and !to_tterm) {
        for (auto &tterm : all_tterms) {
            if (tterm->getDirection() == PinDirection::kInput) {
                continue;
            }
            bool has_arc = tterm->hasTimingArcWithTTerm(from_tterm->getId());
            if (!has_arc) {
                continue;
            }
            this->from_tterm_ = from_tterm->getId();
            this->to_tterm_ = tterm->getId();
            break;
        }
    } else if (!from_tterm and to_tterm) {
        const auto &arcs = to_tterm->getTimingArcs();
        for (auto &arc : arcs) {
            TTerm *from = arc->getRelatedPin();
            if (!from) {
                continue;
            }
            this->from_tterm_ = from->getId();
            this->to_tterm_ = to_tterm->getId();
            break;
        }
    } else {
        bool found_arc = false;
        for (auto &tterm : all_tterms) {
            if (tterm->getDirection() == PinDirection::kInput) {
                continue;
            }
            const auto &arcs = tterm->getTimingArcs();
            for (auto &arc : arcs) {
                TTerm *from = arc->getRelatedPin();
                if (!from) {
                    continue;
                }
                this->from_tterm_ = from->getId();
                this->to_tterm_ = tterm->getId();
                found_arc = true;
                break;
            }
            if (found_arc) {
                break;
            }
        }
    }
    if ((this->from_tterm_ != UNINIT_OBJECT_ID) and (this->to_tterm_ != UNINIT_OBJECT_ID)) {
        return true;
    }
    return false;
}

bool SetDrivingCell::operator==(const SetDrivingCell &rhs) const {
    return this->condition_ == rhs.getCondition();
}

bool SetFanoutLoad::operator==(const SetFanoutLoad &rhs) const {
    bool ret = ediEqual(value_, rhs.getValue());
    return ret;
}

SetInputTransition::SetInputTransition() {
    rise_ = false;
    fall_ = false;
    min_ = false;
    max_ = false;
}

bool SetInputTransition::operator==(const SetInputTransition &rhs) const {
    bool ret =  (rise_ == rhs.getRise()) and (fall_ == rhs.getFall()) and
                (min_ == rhs.getMin()) and (max_ == rhs.getMax());
    return ret;
}

SetLoad::SetLoad() {
    min_ = false;
    max_ = false;
    rise_ = false;
    fall_ = false;
    substract_pin_load_ = false;
    pin_load_ = false;
    wire_load_ = false;
}

void SetLoad::checkFlags() {
    if (pin_load_ == wire_load_) {
        // If do not specify both -pin_load and -wire_load option or specify both of them, then -pin_load option is the default.
        pin_load_ = true;
        wire_load_ = false;
    }
}

bool SetLoad::operator==(const SetLoad &rhs) const {
    bool ret =  (min_ == rhs.getMin()) and (max_ == rhs.getMax()) and
                (rise_ == rhs.getRise()) and (fall_ == rhs.getFall()) and
                (substract_pin_load_ == rhs.getSubstractPinLoad()) and
                (pin_load_ == rhs.getPinLoad()) and (wire_load_ == rhs.getWireLoad());
    return ret;
}

std::string toString(const LogicValue &value) {
    switch (value) {
        case LogicValue::kZero:
            return "zero";
        case LogicValue::kOne:
            return "one";
        case LogicValue::kDontCare:
            return "dontcare";
        defalut:
            return "unknown";
    }
    return "unknown";
}

bool SetLogic::operator==(const SetLogic &rhs) const {
    bool ret = (value_ == rhs.getValue());
    return ret;
}

bool SetLogic::is_constant() const {
    return (value_ == LogicValue::kOne) or (value_ == LogicValue::kZero);
}

bool SetMaxArea::operator==(const SetMaxArea &rhs) const {
    bool ret = ediEqual(area_value_, rhs.getAreaValue());
    return ret;
}

bool SetMaxCapacitance::operator==(const SetMaxCapacitance &rhs) const {
    bool ret = ediEqual(cap_value_, rhs.getCapValue());
    return ret;
}

bool SetMaxFanout::operator==(const SetMaxFanout &rhs) const {
    bool ret = ediEqual(fanout_value_, rhs.getFanoutValue());
    return ret;
}

SetMaxTransition::SetMaxTransition() {
    clock_path_ = false;
    fall_ = false;
    rise_ = false;
}

bool SetMaxTransition::operator==(const SetMaxTransition &rhs) const {
    bool ret =  (clock_path_ == rhs.getClockPath()) and
                (fall_ == rhs.getFall()) and (rise_ == rhs.getRise());
    return ret;
}

bool SetMinCapacitance::operator==(const SetMinCapacitance &rhs) const {
    bool ret = ediEqual(cap_value_, rhs.getCapValue());
    return ret;
}

std::string toString(const AnalysisType &type) {
    switch (type) {
        case AnalysisType::kSingle :
            return "signal";
        case AnalysisType::kBcWc :
            return "bc_wc";
        case AnalysisType::kOnChipVariation :
            return "on_chip_variation";
        case AnalysisType::kUnknown :
        default:
            return "unknown";
    }
    return "unknown";
}

bool SetOperatingConditions::setValue(std::string& input) {
    if (input=="signal") {
        analysis_type_ = AnalysisType::kSingle;
        return true;
    }
    if (input=="bc_wc") {
        analysis_type_ = AnalysisType::kBcWc;
        return true;
    }
    if (input=="on_chip_variation") {
        analysis_type_ = AnalysisType::kOnChipVariation;
        return true;
    }
    analysis_type_ = AnalysisType::kUnknown;
    return false;
}

bool SetPortFanoutNumber::operator==(const SetPortFanoutNumber &rhs) const {
    bool ret = (fanout_number_ == rhs.getFanoutNumber());
    return ret;
}

SetResistance::SetResistance() {
    min_ = false;
    max_ = false;
}

bool SetResistance::operator==(const SetResistance &rhs) const {
    bool ret = (min_ == rhs.getMin()) and (max_ == rhs.getMax());
    return ret;
}

SetTimingDerate::SetTimingDerate() {
    min_ = false;
    max_ = false;
    rise_ = false;
    fall_ = false;
    early_ = false;
    late_ = false;
    static_type_ = false;
    dynamic_type_ = false;
    increment_ = false;
    clock_ = false;
    data_ = false;
    net_delay_ = false;
    cell_delay_ = false;
    cell_check_ = false;
}

void SetTimingDerate::checkFlags() {
    if ((cell_delay_==false) and (net_delay_==false)) {
        cell_delay_ = true;
        net_delay_ = true;
    }
}

bool SetTimingDerate::operator==(const SetTimingDerate &rhs) const {
    bool ret = (min_ == rhs.getMin()) and (max_ == rhs.getMax()) and (rise_ == rhs.getRise()) and (fall_ == rhs.getFall()) and
               (early_ == rhs.getEarly()) and (late_ == rhs.getLate()) and (static_type_ == rhs.getStaticType()) and
               (dynamic_type_ == rhs.getDynamicType()) and (increment_ == rhs.getIncrement()) and (clock_ == rhs.getClock()) and
               (data_ == rhs.getData()) and (net_delay_ == rhs.getNetDelay()) and (cell_delay_ == rhs.getCellDelay()) and
               (cell_check_ == rhs.getCellCheck());
    return ret;
}

bool SetVoltage::operator==(const SetVoltage &rhs) const {
    bool ret = ediEqual(max_case_voltage_, rhs.getMaxCaseVoltage()) and ediEqual(min_case_voltage_, rhs.getMinCaseVoltage());
    return ret;
}

bool SetWireLoadMinBlockSize::operator==(const SetWireLoadMinBlockSize &rhs) const {
    bool ret = ediEqual(block_size_, rhs.getBlockSize());
    return ret;
}

bool SetWireLoadMode::set(const std::string &mode_name) {
    mode_ = open_edi::util::toEnumByString<WireLoadMode>(mode_name.c_str());
    if (mode_ == WireLoadMode::kUnknown) {
        return false;
    }
    return true;
}

bool SetWireLoadMode::operator==(const SetWireLoadMode &rhs) const {
    bool ret = (mode_ == rhs.getMode());
    return ret;
}

SetWireLoadModel::SetWireLoadModel() {
    min_ = false;
    max_ = false;
}

bool SetWireLoadModel::operator==(const SetWireLoadModel &rhs) const {
    bool ret =  (wire_load_ == rhs.getWireLoad()) and
                (min_ == rhs.getMin()) and (max_ == rhs.getMax());
    return ret;
}

SetWireLoadSelectionGroup::SetWireLoadSelectionGroup() {
    min_ = false;
    max_ = false;
}

bool SetWireLoadSelectionGroup::operator==(const SetWireLoadSelectionGroup &rhs) const {
    bool ret =  (selection_ == rhs.getSelection()) and
                (min_ == rhs.getMin()) and (max_ == rhs.getMax());
    return ret;
}


}
}
