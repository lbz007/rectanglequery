/**
 * @file timing_constraints.cpp
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

#include "db/timing/sdc/timing_constraints.h"
#include "db/timing/timinglib/timinglib_cell.h"
#include "db/timing/timinglib/timinglib_term.h"

namespace open_edi {
namespace db {

CreateClock::CreateClock() {
    add_ = false;
}

CreateGeneratedClock::CreateGeneratedClock() {
    invert_ = false;
    combinational_ = false;
    add_ = false;
}

const std::string ClockContainerData::default_clock_name_ = "";
open_edi::util::UInt32 ClockContainerData::count_ = 0;

void ClockContainerData::addVirtualClock(ClockPtr &clock, CreateClockPtr &create_clock) {
    if (!clock->isVirtual()) {
        return;
    }
    addClock(clock, create_clock);
    pin_clock_value_.insert(PinClockMap::value_type(UNINIT_OBJECT_ID, clock->getId()));
}

void ClockContainerData::addVirtualClock(ClockPtr &clock, CreateGeneratedClockPtr &create_generated_clock) {
    if (!clock->isVirtual()) {
        return;
    }
    addClock(clock, create_generated_clock);
    pin_clock_value_.insert(PinClockMap::value_type(UNINIT_OBJECT_ID, clock->getId()));
}

void ClockContainerData::addClock(ClockPtr &clock, CreateClockPtr &create_clock) {
    count_++;
    const ClockId clock_id = count_;
    clock->setId(clock_id);
    const std::string &clock_name = clock->getName();
    name_to_id_[clock_name] = clock_id;
    create_clocks_[clock_id] = create_clock;
    id_to_ptr_[clock_id] = clock;
}

void ClockContainerData::updateClock(ClockPtr &clock, CreateClockPtr &create_clock, const ClockId &clock_id) {
    removeClock(clock_id, false);
    clock->setId(clock_id);
    const std::string &clock_name = clock->getName();
    name_to_id_[clock_name] = clock_id;
    create_clocks_[clock_id] = create_clock;
    id_to_ptr_[clock_id] = clock;
}

void ClockContainerData::updateClock(ClockPtr &clock, CreateGeneratedClockPtr &create_generated_clock, const ClockId &clock_id) {
    removeClock(clock_id, false);
    clock->setId(clock_id);
    const std::string &clock_name = clock->getName();
    name_to_id_[clock_name] = clock_id;
    create_generated_clocks_[clock_id] = create_generated_clock;
    id_to_ptr_[clock_id] = clock;
}

void ClockContainerData::addClock(ClockPtr &clock, CreateGeneratedClockPtr &create_generated_clock) {
    count_++;
    const ClockId clock_id = count_;
    clock->setId(clock_id);
    calcGeneratedClockPeriod(clock, create_generated_clock);
    const std::string &clock_name = clock->getName();
    name_to_id_[clock_name] = clock_id;
    create_generated_clocks_[clock_id] = create_generated_clock;
    id_to_ptr_[clock_id] = clock;
}

void ClockContainerData::calcGeneratedClockPeriod(ClockPtr &clock, CreateGeneratedClockPtr &create_generated_clock) {
    //place holder, freq and duty_cycle, divided by and multiply by, edges
}

void ClockContainerData::removeClockPin(const ObjectId &pin_id) {
    std::vector<ClockId> need_remove_clocks;
    const auto &range_left = pin_clock_value_.left.equal_range(pin_id);
    for (auto it = range_left.first; it != range_left.second; ++it) {
        const ClockId &clock_id = it->second;
        const auto num = pin_clock_value_.right.count(clock_id);
        if (num == 1 ) {
            need_remove_clocks.emplace_back(clock_id);
        }
    }
    pin_clock_value_.left.erase(pin_id);
    for (const auto &clock_id : need_remove_clocks) {
        removeClock(clock_id);
    }
}

void ClockContainerData::removeClock(const ClockId &clock_id, bool update_sdc) {
    auto found = id_to_ptr_.find(clock_id);
    if (found == id_to_ptr_.end()) {
        return;
    }
    auto &clock = found->second;
    name_to_id_.erase(clock->getName());
    create_clocks_.erase(clock_id);
    create_generated_clocks_.erase(clock_id);
    clock->setId(kInvalidClockId);
    id_to_ptr_.erase(clock_id);
    pin_clock_value_.right.erase(clock_id);
    if (update_sdc) {
        removeClockFromContainers(clock_id);
    }
}

bool ClockContainerData::addClockPin(const ObjectId &pin_id, ClockPtr &clock, const bool is_add) {
    if (!is_add) {
        removeClockPin(pin_id); // Remove exist clocks if not set "-add" on pin
    }
    pin_clock_value_.insert(PinClockMap::value_type(pin_id, clock->getId()));
    return true;
}

PathNodes::PathNodes() {
    rise_ = false;
    fall_ = false;
}

void PathNodes::checkFlags() {
    if ((rise_ == false) and (fall_ == false)) {
        rise_ = true;
        fall_ = true;
    }
}

PathThroughNodes::PathThroughNodes() {
    rise_ = false;
    fall_ = false;
}

void PathThroughNodes::checkFlags() {
    if ((rise_ == false) and (fall_ == false)) {
        rise_ = true;
        fall_ = true;
    }
}

ExceptionPath::ExceptionPath() {
    from_ = std::make_shared<PathNodes>();
    to_ = std::make_shared<PathNodes>();
}

GroupPath::GroupPath() {
    default_value_ = false;
}

SetClockGatingCheck::SetClockGatingCheck() {
    rise_ = false;
    fall_ = false;
    high_ = false;
    low_ = false;
}

bool SetClockGatingCheck::operator==(const SetClockGatingCheck &rhs) const {
    bool ret = ediEqual(setup_, rhs.getSetup()) and ediEqual(hold_, rhs.getHold()) and
                (rise_ == rhs.getRise()) and (fall_ == rhs.getFall()) and (high_ == rhs.getHigh()) and
                (low_ == rhs.getLow());
    return ret;
}

std::string toString(const RelationshipType &value) {
    switch (value) {
        case RelationshipType::kPhysicallyExclusive :
            return "physically_exclusive";
        case RelationshipType::kAsynchronousAllowPaths :
            return "asynchronous_and_allow_paths";
        case RelationshipType::kAsynchronous :
            return "asynchronous";
        case RelationshipType::kLogicallyExclusive :
            return "logically_exclusive";
        defalut:
            return "unknown";
    }
    return "unknown";
}

template<>
bool ContainerDataAccess::leftHasLowerPriority(const RelationshipType &lhs, const RelationshipType &rhs) {
  return lhs > rhs;
}

void ClockGroupsContainerData::setRelationBetweenClockGroups(const std::unordered_set<ClockId> &lhs, const std::unordered_set<ClockId> &rhs, const RelationshipType &relation) {
    for (const auto &clock_id : lhs) {
        for (const auto &other_clock_id : rhs) {
            if (clock_id == other_clock_id) {
                //error messages
                continue;
            }
            UnorderedClockPair clock_pair(clock_id, other_clock_id);
            addClockRelationship(clock_pair, relation);
        }
    }
}

void ClockGroupsContainerData::buildClockRelationship(SetClockGroupsPtr &set_clock_groups, const std::unordered_set<ClockId> &all_clocks) {
    const auto &groups = set_clock_groups->getGroups();
    const auto &group_num = groups.size();
    assert(group_num != 0);
    const RelationshipType &relation = set_clock_groups->getRelationType();
    if (group_num == 1) { //clocks in the group has relationship with all other clocks in the current design
        setRelationBetweenClockGroups(groups[0], all_clocks, relation);
    }
    if (group_num > 1) {
        size_t i = 0;
        for (; i < group_num-1; ++i) {
            size_t j = i+1;
            for (; j < group_num; ++j) {
                setRelationBetweenClockGroups(groups[i], groups[j], relation);
            }
        }
    }
}

SetClockLatency::SetClockLatency() {
    rise_ = false;
    fall_ = false;
    min_ = false;
    max_ = false;
    dynamic_ = false;
    source_ = false;
    early_ = false;
    late_ = false;
}

bool SetClockLatency::operator==(const SetClockLatency &rhs) const {
    bool ret = ediEqual(delay_, rhs.getDelay()) and (rise_ == rhs.getRise()) and (fall_ == rhs.getFall()) and
                (min_ == rhs.getMin()) and (max_ == rhs.getMax()) and (dynamic_ == rhs.getDynamic()) and
                (source_ == rhs.getSource()) and (early_ == rhs.getEarly()) and (late_ == rhs.getLate());
    return ret;
}

bool ClockLatencyOnPin::operator==(const ClockLatencyOnPin &rhs) const {
    bool ret = ediEqual(this->getDelay(), rhs.getDelay()) and (this->getRise() == rhs.getRise()) and
                (this->getFall() == rhs.getFall()) and (this->getMin() == rhs.getMin()) and
                (this->getMax() == rhs.getMax()) and (this->getDynamic() == rhs.getDynamic()) and
                (this->getSource() == rhs.getSource()) and (this->getEarly() == rhs.getEarly()) and
                (this->getLate() == rhs.getLate()) and (this->getPinAssociatedClocks() == rhs.getPinAssociatedClocks());
    return ret;
}

std::string toString(const DataType &value) {
    switch (value) {
        case DataType::kClock :
            return "clock";
        case DataType::kData:
            return "data";
        defalut:
            return "unknown";
    }
    return "unknown";
}

std::string toString(const PulseType &value) {
    switch (value) {
        case PulseType::kRiseTriggeredHighPulse :
            return "rise_triggered_high_pulse";
        case PulseType::kRiseTriggeredLowPulse :
            return "rise_triggered_low_pulse";
        case PulseType::kFallTriggeredHighPulse :
            return "fall_triggered_high_pulse";
        case PulseType::kFallTriggeredLowPulse :
            return "fall_triggered_low_pulse";
        default:
            return "unknown";
    }
    return "unknown";
}

SetSense::SetSense() {
    non_unate_ = false;
    positive_ = false;
    negative_ = false;
    clock_leaf_ = false;
    stop_propagation_ = false;
}

bool SetSense::setType(const std::string &type_name) {
    type_ = open_edi::util::toEnumByString<DataType>(type_name.c_str());
    if (type_ == DataType::kUnknown) {
        return false;
    }
    return true;
}

bool SetSense::setPulse(const std::string &pulse_name) {
    pulse_ = open_edi::util::toEnumByString<PulseType>(pulse_name.c_str());
    if (pulse_ == PulseType::kUnknown) {
        return false;
    }
    return true;
}

bool SetSense::operator==(const SetSense &rhs) const {
    bool ret = (type_ == rhs.getType()) and (pulse_ == rhs.getPulse()) and (clocks_ == rhs.getClocks()) and
                (non_unate_ == rhs.getNonUnate()) and (positive_ == rhs.getPositive()) and
                (negative_ == rhs.getNegative()) and (clock_leaf_ == rhs.getClockLeaf()) and
                (stop_propagation_ == rhs.getStopPropation());
    return ret;
}

SetClockTransition::SetClockTransition() {
    rise_ = false;
    fall_ = false;
    min_ = false;
    max_ = false;
}

bool SetClockTransition::operator==(const SetClockTransition &rhs) const {
    bool ret = (rise_ == rhs.getRise()) and (fall_ == rhs.getFall()) and
                (min_ == rhs.getMin()) and (max_ == rhs.getMax());
    return ret;
}

SetClockUncertainty::SetClockUncertainty() {
    setup_ = false;
    hold_ = false;
}

bool SetClockUncertainty::operator==(const SetClockUncertainty &rhs) const {
    bool ret = (setup_ == rhs.getSetup()) and (hold_ == rhs.getHold());
    return ret;
}

InterClockUncertainty::InterClockUncertainty() {
    rise_from_ = false;
    fall_from_ = false;
    rise_to_ = false;
    fall_to_ = false;
}

InterClockUncertainty::InterClockUncertainty(Base &base) : SetClockUncertainty(base) {
    rise_from_ = false;
    fall_from_ = false;
    rise_to_ = false;
    fall_to_ = false;
}

bool InterClockUncertainty::operator==(const InterClockUncertainty &rhs) const {
    bool ret =  (this->getSetup() == rhs.getSetup()) and (this->getHold() == rhs.getHold()) and
                (this->getRiseFrom() == rhs.getRiseFrom()) and (this->getFallFrom() == rhs.getFallFrom()) and
                (this->getRiseTo() == rhs.getRiseTo()) and (this->getFallTo() == rhs.getFallTo());
    return ret;
}

SetDataCheck::SetDataCheck() {
    rise_from_ = false;
    fall_from_ = false;
    rise_to_ = false;
    fall_to_ = false;
    setup_ = false;
    hold_ = false;
}

bool SetDataCheck::operator==(const SetDataCheck &rhs) const {
    bool ret =  (rise_from_ == rhs.getRiseFrom()) and (fall_from_ == rhs.getFallFrom()) and
                (rise_to_ == rhs.getRiseTo()) and (fall_to_ == rhs.getFallTo()) and
                (setup_ == rhs.getSetup()) and (hold_ == rhs.getHold()) and
                (clock_ == rhs.getClock());
    return ret;
}

bool DataCheckContainerData::addCheck(const std::string &from_pin_name, const std::string &to_pin_name, const SetDataCheckPtr &check) {
    const auto &from_pin = getPinByFullName(from_pin_name);
    if (!from_pin) {
        return false;
    }
    const auto &to_pin = getPinByFullName(to_pin_name);
    if (!to_pin) {
        return false;
    }
    const auto &from_pin_id = from_pin->getId();
    const auto &to_pin_id = to_pin->getId();
    return ContainerDataAccess::updateMapValue(pin_data_check_, PinPair(from_pin_id, to_pin_id), check);
}

SetDisableTiming::SetDisableTiming() {
    all_arcs_ = false;
}

bool SetDisableTiming::operator==(const SetDisableTiming &rhs) const {
    bool ret = (from_id_ == rhs.getFromId()) and (to_id_ == rhs.getToId()) and (all_arcs_ == rhs.getAllArcs());
    return ret;
}

bool DisableTimingContainerData::addToInst(const std::string &inst_name, const std::string &from, const std::string &to) {
    const auto &top_cell = getTopCell();
    const auto &inst = top_cell->getInstance(inst_name);
    if (!inst) {
        return false;
    }
    SetDisableTimingPtr disable_timing = std::make_shared<SetDisableTiming>();
    //From and to must be specified together, has been checked in the command parser entry

    if (!from.empty() and !to.empty()) {
        const auto &from_pin = inst->getPin(from);
        if (!from_pin) {
            return false;
        }
        const SignalDirection from_dir = from_pin->getDirection();
        if (from_dir == SignalDirection::kOutput) {
            return false;
        }
        const auto &to_pin = inst->getPin(to);
        if (!to_pin) {
            return false;
        }
        const SignalDirection to_dir = to_pin->getDirection();
        if (to_dir == SignalDirection::kInput) {
            return false;
        }
        //TODO check whether arc exist by pin to tterm map
        disable_timing->setFromId(from_pin->getId());
        disable_timing->setToId(to_pin->getId());
    } else {
        disable_timing->setAllArcs();
    }
    bool success = ContainerDataAccess::updateMapValue(inst_disable_timing_, inst->getId(), disable_timing);
    if (!success) {
        return false;
    }
    return true;
}

bool DisableTimingContainerData::addToTCell(AnalysisCorner *corner, const std::string &lib_cell_name, const std::string &from, const std::string &to) {
    TCell* tcell = getLibCellInCorner(corner, lib_cell_name);
    if (!tcell) {
        return false;
    }
    SetDisableTimingPtr disable_timing = std::make_shared<SetDisableTiming>();
    //From and to must be specified together, has been checked in the command parser entry
    if (!from.empty() and !to.empty()) {
        const TTerm *from_tterm = tcell->getTerm(from);
        if (!from_tterm) {
            return false;
        }
        const TTerm *to_tterm = tcell->getTerm(to);
        if (!to_tterm) {
            return false;
        }
        bool has_arc = to_tterm->hasTimingArcWithTTerm(from_tterm->getId());
        if (!has_arc) {
            return false;
        }
        disable_timing->setFromId(from_tterm->getId());
        disable_timing->setToId(to_tterm->getId());
    } else {
        disable_timing->setAllArcs();
    }
    bool success = ContainerDataAccess::updateMapValue(tcell_disable_timing_, tcell->getId(), disable_timing);
    if (!success) {
        return false;
    }
    return true;
}

bool DisableTimingContainerData::addToTTerm(AnalysisCorner *corner, const std::string &pin_name, const std::string &from, const std::string &to) {
    if (!from.empty() and !to.empty()) {
        //error messages
        return false;
    }
    const auto &pos = pin_name.find_last_of('/');
    if (pos == std::string::npos) {
        return false;
    }
    const std::string &lib_cell_name = pin_name.substr(0, pos);
    const std::string &term_name = pin_name.substr(pos+1);
    TCell* tcell = getLibCellInCorner(corner, lib_cell_name);
    if (!tcell) {
        return false;
    }
    const TTerm *tterm = tcell->getTerm(term_name);
    if (!tterm) {
        return false;
    }
    tterm_disable_timing_.emplace(tterm->getId());
    return true;
}

bool DisableTimingContainerData::addToPin(const std::string &pin_name, const std::string &from, const std::string &to) {
    if (!from.empty() and !to.empty()) {
        //error messages
        return false;
    }
    const auto &pin = getPinByFullName(pin_name);
    if (!pin) {
        return false;
    }
    pin_disable_timing_.emplace(pin->getId());
    return true;
}

SetFalsePath::SetFalsePath() : ExceptionPath() {
    setup_ = false;
    hold_ = false;
    rise_ = false;
    fall_ = false;
}

void SetFalsePath::checkFlags() {
    if ((setup_==false) and (hold_==false)) {
        setup_ = true;
        hold_ = true;
    }
    if ((rise_==false) and (fall_==false)) {
        rise_ = true;
        fall_ = true;
    }
}

SetIdealLatency::SetIdealLatency() {
    rise_ = false;
    fall_ = false;
    min_ = false;
    max_ = false;
}

bool SetIdealLatency::operator==(const SetIdealLatency &rhs) const {
    bool ret =  (rise_ == rhs.getRise()) and (fall_ == rhs.getFall()) and
                (min_ == rhs.getMin()) and (max_ == rhs.getMax());
    return ret;
}

bool SetIdealNetwork::addToNet(const std::string &net_name, const bool is_no_propagated) {
    if (!is_no_propagated) {
        //error messages: accept nets only when you specify the -no_propagate option
        return false;
    }
    return ContainerDataAccess::addToNetSet(nets_, net_name);
}

SetIdealTransition::SetIdealTransition() {
    rise_ = false;
    fall_ = false;
    min_ = false;
    max_ = false;
}

bool SetIdealTransition::operator==(const SetIdealTransition &rhs) const {
    bool ret =  (rise_ == rhs.getRise()) and (fall_ == rhs.getFall()) and
                (min_ == rhs.getMin()) and (max_ == rhs.getMax());
    return ret;
}

PortDelay::PortDelay() {
    clock_fall_ = false;
    level_sensitive_ = false;
    rise_ = false;
    fall_ = false;
    min_ = false;
    max_ = false;
    add_delay_ = false;
    network_latency_included_ = false;
    source_latency_included_ = false;
}

bool PortDelay::operator==(const PortDelay &rhs) const {
    bool ret =  (clock_ == rhs.getClock()) and (add_delay_ == rhs.getAddDelay()) and
                (reference_pin_ == rhs.getReferencePin()) and (clock_fall_ == rhs.getClockFall()) and
                (level_sensitive_ == rhs.getLevelSensitive()) and (rise_ == rhs.getRise()) and
                (fall_ == rhs.getFall()) and (min_ == rhs.getFall()) and (max_ == rhs.getMax()) and
                (network_latency_included_ == rhs.getNetworkLatencyIncluded()) and
                (source_latency_included_ == rhs.getSourceLatencyIncluded());
    return ret;
}

template<>
bool ContainerDataAccess::rightHasHigherPriority(const SetInputDelayPtr &lhs, const SetInputDelayPtr &rhs) {
    return !(rhs->getAddDelay());
}

SetMaxDelay::SetMaxDelay() {
    rise_ = false;
    fall_ = false;
    ignore_clock_latency_ = false;
}

void SetMaxDelay::checkFlags() {
    if ((rise_==false) and (fall_==false)) {
        rise_ = true;
        fall_ = true;
    }
}

bool SetMaxTimeBorrow::operator==(const SetMaxTimeBorrow &rhs) const {
    bool ret = ediEqual(value_, rhs.getValue());
    return ret;
}

SetMinDelay::SetMinDelay() {
    rise_ = false;
    fall_ = false;
    ignore_clock_latency_ = false;
}

void SetMinDelay::checkFlags() {
    if ((rise_==false) and (fall_==false)) {
        rise_ = true;
        fall_ = true;
    }
}

SetMinPulseWidth::SetMinPulseWidth() {
    low_ = false;
    high_ = false;
}

bool SetMinPulseWidth::operator==(const SetMinPulseWidth &rhs) const {
    bool ret = (low_ == rhs.getLow()) and (high_ == rhs.getHigh());
    return ret;
}

SetMulticyclePath::SetMulticyclePath() {
    setup_ = false;
    hold_ = false;
    rise_ = false;
    fall_ = false;
    start_ = false;
    end_ = false;
}

void SetMulticyclePath::checkFlags() {
    if ((rise_==false) and (fall_==false)) {
        rise_ = true;
        fall_ = true;
    }
    if ((setup_==false) and (hold_==false)) {
        setup_ = true;
    }
}

template<>
bool ContainerDataAccess::rightHasHigherPriority(const SetOutputDelayPtr &lhs, const SetOutputDelayPtr &rhs) {
    return !(rhs->getAddDelay());
}

bool SetPropagatedClockContainerData::addToClock(ClockPtr &clock) {
    if (!clock) {
        return false;
    }
    if (clock->isPropagated()) {
        return true;
    }
    clock->setPropagated();
    clocks_.emplace(clock->getId());
    return true;
}

}
}
