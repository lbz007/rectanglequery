/**
 * @file sdc_data.cpp
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

#include "db/timing/sdc/sdc_data.h"
#include "db/timing/timinglib/timinglib_lib.h"
#include "db/timing/timinglib/timinglib_term.h"
#include "db/timing/timinglib/timinglib_timingarc.h"

namespace open_edi {
namespace db {

//general purpose commands
const std::string SdcCurrentInstanceContainer::getInstName() const {
    const ObjectId &inst_id = data_->getInstId();
    const Inst* inst = Object::addr<Inst>(inst_id);
    if (!inst) {
        return "";
    }
    return inst->getName();
}

std::ostream &operator<<(std::ostream &os, SdcHierarchySeparatorContainer &rhs) {
    os << "set_hierarchy_separator ";
    os << rhs.get();
    os << "\n";
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcUnitsContainer &rhs) {
    os << "set_units ";
    const FlagValue &v1 = FlagValue("-time", std::to_string(rhs.getSdcTimeUnits()) + "s");
    const FlagValue &v2 = FlagValue("-capacitance", std::to_string(rhs.getSdcCapacitanceUnits()) + "F");
    const FlagValue &v3 = FlagValue("-current", std::to_string(rhs.getSdcCurrentUnits()) + "A");
    const FlagValue &v4 = FlagValue("-voltage", std::to_string(rhs.getSdcVoltageUnits()) + "V");
    const FlagValue &v5 = FlagValue("-resistance", std::to_string(rhs.getSdcResistanceUnits()) + "Ohm");
    const FlagValue &v6 = FlagValue("-power", std::to_string(rhs.getSdcPowerUnits()) + "W");
    os << ContainerDataPrint::getFlagValue(v1, v2, v3, v4, v5, v6);
    os << "\n";
    return os;
}

// timing constraint commands
void SdcClockContainer::getClocks(std::vector<ClockPtr> &clocks) const {
    clocks.clear();
    const auto &id_to_ptr_map = data_->getIdToPtr();
    clocks.reserve(id_to_ptr_map.size());
    for (auto id_to_ptr : id_to_ptr_map) {
        clocks.emplace_back(id_to_ptr.second);
    }
}

void SdcClockContainer::getVirtualClocks(std::vector<ClockPtr> &clocks) const {
    clocks.clear();
    const auto &pin_clock_value = data_->getPinClockValue();
    const auto &id_to_ptr_map = data_->getIdToPtr();
    const auto &pin_view = pin_clock_value.left;
    const auto &range = pin_view.equal_range(UNINIT_OBJECT_ID);
    for (auto it = range.first; it != range.second; ++it) {
        const auto &clock_id = it->second;
        const auto &found = id_to_ptr_map.find(clock_id);
        if (found == id_to_ptr_map.end()) {
            //error messages
            continue;
        }
        clocks.emplace_back(found->second);        
    }
}

void SdcClockContainer::getClockNames(std::vector<std::string> &names) const {
    names.clear();
    const auto &name_to_id_map = data_->getNameToId();
    names.reserve(name_to_id_map.size());
    for (auto name_to_id : name_to_id_map) {
        names.emplace_back(name_to_id.first);
    }
}

void SdcClockContainer::getClockIds(std::vector<ClockId> &ids) const {
    ids.clear();
    const auto &name_to_id_map = data_->getNameToId();
    ids.reserve(name_to_id_map.size());
    for (auto name_to_id : name_to_id_map) {
        ids.emplace_back(name_to_id.second);
    }
}

ClockPtr SdcClockContainer::getClock(const ClockId &id) const {
    if ((id == kInvalidClockId) or (id > data_->getCount())) {
        return nullptr;
    }
    const auto &id_to_ptr_map = data_->getIdToPtr();
    const auto &found = id_to_ptr_map.find(id);
    if (found == id_to_ptr_map.end()) {
        return nullptr;
    }
    return found->second;
}

ClockPtr SdcClockContainer::getClock(const std::string &name) const {
    const auto &name_to_id = data_->getNameToId();
    const auto &found = name_to_id.find(name);
    if (found == name_to_id.end()) {
        return nullptr;
    }
    const auto &id = found->second;
    return getClock(id);
}

const ClockId &SdcClockContainer::getClockId(const std::string &name) const{
    const auto &name_to_id = data_->getNameToId();
    const auto &found = name_to_id.find(name);
    if (found == name_to_id.end()) {
        //TODO error message
        return kInvalidClockId;
    }
    return found->second;
}

const std::string &SdcClockContainer::getClockName(const ClockId &id) const {
    const ClockPtr &clock = getClock(id);
    if (!clock) {
        return ClockContainerData::default_clock_name_;
    }
    return clock->getName();
}

void SdcClockContainer::getClockOnPin(std::vector<ClockPtr> &clocks, const ObjectId &pin_id) const {
    clocks.clear();
    const auto &pin_clock_value = data_->getPinClockValue();
    const auto &pin_view = pin_clock_value.left;
    const auto &range = pin_view.equal_range(pin_id);
    for (auto it = range.first; it != range.second; ++it) {
        const auto &clock_id = it->second;
        const auto &clock = getClock(clock_id);
        if (!clock) {
            //error messages
            continue;
        }
        clocks.emplace_back(clock);
    }
}

void SdcClockContainer::getPinOnClock(std::vector<ObjectId> &pins, const ClockId &id) const {
    pins.clear();
    const auto &pin_clock_value = data_->getPinClockValue();
    const auto &clock_view = pin_clock_value.right;
    const auto &range = clock_view.equal_range(id);
    for (auto it = range.first; it != range.second; ++it) {
        const auto &pin_id = it->second;
        pins.emplace_back(pin_id);
    }
}

bool SdcClockContainer::isClockPin(const ObjectId &pin_id) const {
    const auto &pin_clock_value = data_->getPinClockValue();
    const auto &pin_view = pin_clock_value.left;
    const std::size_t num = pin_view.count(pin_id);
    if (num >= 1) {
        return true;
    }
    return false;
}

std::ostream &operator<<(std::ostream &os, SdcClockContainer &rhs) {
    const auto data = rhs.data_;
    const PinClockMap &pin_value = data->getPinClockValue();
    const auto &create_clocks = data->getCreateClocks();
    const auto &sorted_create_clocks = std::map<ClockId, CreateClockPtr>(create_clocks.begin(), create_clocks.end());
    const auto &clock_view = pin_value.right;
    for (const auto &id_to_create_clock : sorted_create_clocks) {
        const ClockId &clock_id = id_to_create_clock.first;
        const auto &create_clock = id_to_create_clock.second;
        const ClockPtr &clock = rhs.getClock(clock_id);
        os  << "create_clock ";
        const FlagValue &v1 = FlagValue("-period", std::to_string(clock->getPeriod()));
        const FlagValue &v2 = FlagValue("-name", clock->getName());
        const FlagValue &v3 = FlagValue("-comment", ContainerDataPrint::addDoubleQuotes(create_clock->getComment()));
        const FlagValue &v4 = FlagValue("-waveform", ContainerDataPrint::dataListToStr(clock->getWaveform()));
        os << ContainerDataPrint::getFlagValue(v1, v2, v3, v4);

        const Flag &f1 = Flag("-add", create_clock->isAdd());
        os << ContainerDataPrint::getFlag(f1);

        const auto &range = clock_view.equal_range(clock_id);
        std::vector<ObjectId> pins;
        for_each(range.first, range.second, [&pins](const auto &elem) {pins.emplace_back(elem.second);});
        os << ContainerDataPrint::pinIdsToFullNameList(pins);
        os << "\n";
    }
    auto clock_id_to_name = std::bind(&SdcClockContainer::getClockName, rhs, std::placeholders::_1);
    const auto &create_generated_clocks = data->getCreateGeneratedClocks();
    const auto &sorted_create_generated_clocks = std::map<ClockId, CreateGeneratedClockPtr>(create_generated_clocks.begin(), create_generated_clocks.end());
    for (const auto &id_to_generated_clock : sorted_create_generated_clocks) {
        const ClockId &clock_id = id_to_generated_clock.first;
        const auto &generated_clock = id_to_generated_clock.second;
        const ClockPtr &clock = rhs.getClock(clock_id);
        os  << "create_generated_clock ";
        const FlagValue &v1 = FlagValue("-name", clock->getName());
        const FlagValue &v2 = FlagValue("-source", ContainerDataPrint::pinIdsToFullNameList(generated_clock->getSourceMasterPins()));
        const FlagValue &v3 = FlagValue("-edges", ContainerDataPrint::dataListToStr(generated_clock->getEdges()));
        const FlagValue &v4 = FlagValue("-duty_cycle", std::to_string(generated_clock->getDutyCycle()));
        const FlagValue &v5 = FlagValue("-edge_shift", ContainerDataPrint::dataListToStr(generated_clock->getEdgeShifts()));
        const FlagValue &v6 = FlagValue("-master_clock", ContainerDataPrint::clockIdToName(clock_id_to_name, generated_clock->getMasterClock()));
        const FlagValue &v7 = FlagValue("-comment", ContainerDataPrint::addDoubleQuotes(generated_clock->getComment()));
        const FlagValue &v8 = FlagValue("-divided_by", std::to_string(generated_clock->getDividedBy()));
        const FlagValue &v9 = FlagValue("-multiply_by", std::to_string(generated_clock->getMultiplyBy())); //TODO : should only take one of edges/divided_by/multiply_by to output
        os << ContainerDataPrint::getFlagValue(v1, v2, v3, v4, v5, v6, v7, v8, v9);

        const Flag &f1 = Flag("-combinational", generated_clock->isCombinational());
        const Flag &f2 = Flag("-invert", generated_clock->isInvert());
        const Flag &f3 = Flag("-add", generated_clock->isAdd());
        os << ContainerDataPrint::getFlag(f1, f2, f3);

        const auto &range = clock_view.equal_range(clock_id);
        std::vector<ObjectId> pins;
        for_each(range.first, range.second, [&pins](const auto &elem) {pins.emplace_back(elem.second);});
        os << ContainerDataPrint::pinIdsToFullNameList(pins);
        os << "\n";
    }
    return os;
}

template<typename U>
void removeClockForExceptionPathContainer(U &path_container, const ClockId &clock_id) {
    auto it = path_container.begin();
    std::vector<decltype(it)> need_removed_its;
    for (; it != path_container.end(); ++it) {
        auto &from = (*it)->getFrom();
        std::unordered_set<ClockId> &from_clocks = from->getClocks();
        from_clocks.erase(clock_id);
        if (!from->hasObject()) {
            need_removed_its.push_back(it);
            continue;
        }
        auto &to = (*it)->getTo();
        std::unordered_set<ClockId> &to_clocks = to->getClocks();
        to_clocks.erase(clock_id);
        if (!to->hasObject()) {
            need_removed_its.push_back(it);
        }
    }
    for (const auto &it : need_removed_its) {
        path_container.erase(it);
    }
}

void SdcGroupPathContainer::removeClock(const ClockId &clock_id) {
    auto &group_paths = data_->getGroupPaths();
    removeClockForExceptionPathContainer(group_paths, clock_id);
}

void printFromThroughTo(std::ostream &os, const PathNodesPtr &from, const std::vector<PathThroughNodesPtr> &throughs, const PathNodesPtr &to, const std::function<std::string (const ClockId &)> &clock_id_to_name) {
    const std::unordered_set<ObjectId> &from_pins = from->getPins();
    const std::unordered_set<ObjectId> &from_insts = from->getInsts();
    const std::unordered_set<ClockId> &from_clocks = from->getClocks();
    if (from->getRise() and from->getFall()) {
        os << "-from ";
    } else {
        const Flag &f1 = Flag("-rise_from", from->getRise());
        const Flag &f2 = Flag("-fall_from", from->getFall());
        os << ContainerDataPrint::getFlag(f1, f2);
    }
    os << ContainerDataPrint::pinIdsToFullNameList(from_pins);
    os << ContainerDataPrint::instIdsToNameList(from_insts);
    os << ContainerDataPrint::clockIdsToNameList(clock_id_to_name, from_clocks);
    os << " ";

    for (const auto &through : throughs) {
        const std::unordered_set<ObjectId> &through_pins = through->getPins();
        const std::unordered_set<ObjectId> &through_insts = through->getInsts();
        const std::unordered_set<ObjectId> &through_nets = through->getNets();
        if (through->getRise() and through->getFall()) {
            os << "-through ";
        } else {
            const Flag &f1 = Flag("-rise_through", through->getRise());
            const Flag &f2 = Flag("-fall_through", through->getFall());
            os << ContainerDataPrint::getFlag(f1, f2);
        }
        os << ContainerDataPrint::pinIdsToFullNameList(through_pins);
        os << ContainerDataPrint::instIdsToNameList(through_insts);
        os << ContainerDataPrint::netIdsToNameList(through_nets);
        os << " ";
    }

    const std::unordered_set<ObjectId> &to_pins = to->getPins();
    const std::unordered_set<ObjectId> &to_insts = to->getInsts();
    const std::unordered_set<ClockId> &to_clocks = to->getClocks();
    if (to->getRise() and to->getFall()) {
        os << "-to ";
    } else {
        const Flag &f1 = Flag("-rise_to", to->getRise());
        const Flag &f2 = Flag("-fall_to", to->getFall());
        os << ContainerDataPrint::getFlag(f1, f2);
    }
    os << ContainerDataPrint::pinIdsToFullNameList(to_pins);
    os << ContainerDataPrint::instIdsToNameList(to_insts);
    os << ContainerDataPrint::clockIdsToNameList(clock_id_to_name, to_clocks);
}

std::ostream &operator<<(std::ostream &os, SdcGroupPathContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &group_paths = data->getGroupPaths();
    auto clock_id_to_name = std::bind(&GroupPathContainerData::clockIdToName, rhs.data_, std::placeholders::_1);
    for (const auto &group_path : group_paths) {
        os << "group_path ";
        const FlagValue &v1 = FlagValue("-name", group_path->getName());
        const FlagValue &v2 = FlagValue("-weight", std::to_string(group_path->getWeight()));
        const FlagValue &v3 = FlagValue("-comment", ContainerDataPrint::addDoubleQuotes(group_path->getComment()));
        os << ContainerDataPrint::getFlagValue(v1, v2, v3);
        const Flag &f1 = Flag("-default", group_path->getDefaultValue());
        os << ContainerDataPrint::getFlag(f1);
        printFromThroughTo(os, group_path->getFrom(), group_path->getThroughs(), group_path->getTo(), clock_id_to_name);
        os << "\n";
    }
    return os;
}

const SetClockGatingCheckPtr SdcClockGatingCheckContainer::getCurrentDesignCheckTime() const {
    const auto &cell = getTopCell();
    const auto &cell_id = cell->getId();
    const auto &design_to_check = data_->getDesignToCheck();
    const auto &found = design_to_check.find(cell_id);
    if (found == design_to_check.end()) {
        return nullptr;
    }
    return found->second;
}

const std::pair<float, float> SdcClockGatingCheckContainer::getPinCheckTime(bool is_rise, bool is_high, const ObjectId &pin_id) const {
    const auto &design_check = getCurrentDesignCheckTime();
    if (design_check) {
        return std::pair<float, float>(design_check->getSetup(), design_check->getHold());
    }
    //TODO need to consider the instance check?
    const auto &pin_to_check = data_->getPinToCheck();
    const auto &range = pin_to_check.equal_range(pin_id);
    for(auto it = range.first; it != range.second; ++it) {
        const auto &check = it->second;
        if (!check) {
            // error messages
            continue;
        }
        if (check->getRise()!=is_rise or check->getHigh()!=is_high) {
            continue;
        }
        return std::pair<float, float>(check->getSetup(), check->getHold());
    }
    return std::pair<float, float>(0, 0);
}

const std::pair<float, float> SdcClockGatingCheckContainer::getInstCheckTime(bool is_rise, bool is_high, const ObjectId &inst_id) const {
    const auto &design_check = getCurrentDesignCheckTime();
    if (design_check) {
        return std::pair<float, float>(design_check->getSetup(), design_check->getHold());
    }
    const auto &inst_to_check = data_->getInstToCheck();
    const auto &range = inst_to_check.equal_range(inst_id);
    for(auto it = range.first; it != range.second; ++it) {
        const auto &check = it->second;
        if (!check) {
            // error messages
            continue;
        }
        if (check->getRise()!=is_rise or check->getHigh()!=is_high) {
            continue;
        }
        return std::pair<float, float>(check->getSetup(), check->getHold());
    }
    return std::pair<float, float>(0, 0);
}

const std::pair<float, float> SdcClockGatingCheckContainer::getClockCheckTime(bool is_rise, bool is_high, const ClockId &clock_id) const {
    const auto &design_check = getCurrentDesignCheckTime();
    if (design_check) {
        return std::pair<float, float>(design_check->getSetup(), design_check->getHold());
    }
    const auto &clock_to_check = data_->getClockToCheck();
    const auto &range = clock_to_check.equal_range(clock_id);
    for(auto it = range.first; it != range.second; ++it) {
        const auto &check = it->second;
        if (!check) {
            // error messages
            continue;
        }
        if (check->getRise()!=is_rise or check->getHigh()!=is_high) {
            continue;
        }
        return std::pair<float, float>(check->getSetup(), check->getHold());
    }
    return std::pair<float, float>(0, 0);
}

const float SdcClockGatingCheckContainer::getPinCheckTime(bool is_rise, bool is_high, bool is_setup, const ObjectId &pin_id) const {
   const std::pair<float, float>& setup_hold = getPinCheckTime(is_rise, is_high, pin_id);
   return is_setup ? setup_hold.first : setup_hold.second;
}

const float SdcClockGatingCheckContainer::getInstCheckTime(bool is_rise, bool is_high, bool is_setup, const ObjectId &inst_id) const {
   const std::pair<float, float>& setup_hold = getInstCheckTime(is_rise, is_high, inst_id);
   return is_setup ? setup_hold.first : setup_hold.second;
}

const float SdcClockGatingCheckContainer::getClockCheckTime(bool is_rise, bool is_high, bool is_setup, const ClockId &clock_id) const {
   const std::pair<float, float>& setup_hold = getClockCheckTime(is_rise, is_high, clock_id);
   return is_setup ? setup_hold.first : setup_hold.second;
}

void SdcClockGatingCheckContainer::removeClock(const ClockId &clock_id) {
    auto &clock_to_check = data_->getClockToCheck();
    clock_to_check.erase(clock_id);
}

std::ostream &operator<<(std::ostream &os, SdcClockGatingCheckContainer &rhs) {
    const auto &data = rhs.data_;
    auto print_clock_gating = [](std::ostream &os, const SetClockGatingCheckPtr &check) {
        os << "set_clock_gating_check ";
        const FlagValue &v1 = FlagValue("-setup", std::to_string(check->getSetup()));
        const FlagValue &v2 = FlagValue("-hold", std::to_string(check->getHold()));
        os << ContainerDataPrint::getFlagValue(v1, v2);
        const Flag &f1 = Flag("-rise", check->getRise());
        const Flag &f2 = Flag("-fall", check->getFall());
        const Flag &f3 = Flag("-high", check->getHigh());
        const Flag &f4 = Flag("-low", check->getLow());
        os << ContainerDataPrint::getFlag(f1, f2, f3, f4);
    };
    const auto &clock_to_check = data->getClockToCheck();
    for (const auto &clock_check : clock_to_check) {
        const ClockId &clock_id = clock_check.first;
        const SetClockGatingCheckPtr &check = clock_check.second;
        print_clock_gating(os, check);
        os << ContainerDataPrint::getClockPrintName(data->clockIdToName(clock_id));
        os << "\n";
    }
    const auto &design_to_check = data->getDesignToCheck();
    for (const auto &design_check : design_to_check) {
        const ObjectId &design_id = design_check.first;
        const SetClockGatingCheckPtr &check = design_check.second;
        print_clock_gating(os, check);
        os << ContainerDataPrint::cellIdToName(design_id);
        os << "\n";
    }
    const auto &inst_to_check = data->getInstToCheck();
    for (const auto &inst_check : inst_to_check) {
        const ObjectId &inst_id = inst_check.first;
        const SetClockGatingCheckPtr &check = inst_check.second;
        print_clock_gating(os, check);
        os << ContainerDataPrint::instIdToName(inst_id);
        os << "\n";
    }
    const auto &pin_to_check = data->getPinToCheck();
    for (const auto &pin_check : pin_to_check) {
        const ObjectId &pin_id = pin_check.first;
        const SetClockGatingCheckPtr &check = pin_check.second;
        print_clock_gating(os, check);
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    return os;
}

RelationshipType SdcClockGroupsContainer::getClocksRelationshipType(const ClockId &clock_id, const ClockId &other_clock_id) const {
    UnorderedPair<ClockId, ClockId> clock_pair(clock_id, other_clock_id);
    const auto &clock_relationship = data_->getClockRelationship();
    const auto &found = clock_relationship.find(clock_pair);
    if (found == clock_relationship.end()) {
        return RelationshipType::kUnknown;
    }
    return found->second;
}

void SdcClockGroupsContainer::removeClock(const ClockId &clock_id) {
    auto &clocks_relation = data_->getClockRelationship();
    auto it = clocks_relation.begin();
    std::vector<decltype(it)> need_removed_its;
    for (; it != clocks_relation.end(); ++it) {
        const UnorderedClockPair &clock_pair = it->first;
        if ((clock_pair.first == clock_id) or (clock_pair.second == clock_id)) {
            need_removed_its.emplace_back(it);
        }
    }
    for (auto &rm_it : need_removed_its) {
        clocks_relation.erase(rm_it);
    }
    auto &all_groups = data_->getAllGroups();
    auto it2 = all_groups.begin();
    std::vector<decltype(it2)> need_removed_its2;
    for (; it2 != all_groups.end(); ++it2) {
        auto &groups = (*it2)->getGroups();
        for (auto &group_clocks : groups) {
            group_clocks.erase(clock_id);
            if (group_clocks.empty()) {
                need_removed_its2.emplace_back(it2);
            }
        }
    }
    for (auto &rm_it2 : need_removed_its2) {
        all_groups.erase(rm_it2);
    }
}

std::ostream &operator<<(std::ostream &os, SdcClockGroupsContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &all_groups = data->getAllGroups();
    const auto &clock_relationship = data->getClockRelationship();
    auto clock_id_to_name = std::bind(&ClockGroupsContainerData::clockIdToName, rhs.data_, std::placeholders::_1);
    for (const auto &clock_to_groups : all_groups) {
        os << "set_clock_groups ";
        os << "-" << toString(clock_to_groups->getRelationType()) << " ";
        const FlagValue &v1 = FlagValue("-name", clock_to_groups->getName());
        const FlagValue &v2 = FlagValue("-comment", ContainerDataPrint::addDoubleQuotes(clock_to_groups->getComment()));
        os << ContainerDataPrint::getFlagValue(v1, v2);
        for (const auto &clock_groups : clock_to_groups->getGroups()) {
            const FlagValue &v3 = FlagValue("-group", ContainerDataPrint::clockIdsToNameList(clock_id_to_name, clock_groups));
            os << ContainerDataPrint::getFlagValue(v3);
        }
        os << "\n";
    }
    return os;
}

void SdcClockLatencyContainer::getClockLatencyOnPin(std::vector<ClockLatencyOnPinPtr> &latencys, const ObjectId &pin_id) const {
    const auto &pin_to_latency = data_->getPinToLatency();
    const auto &range = pin_to_latency.equal_range(pin_id);
    for (auto it = range.first; it != range.second; ++it) {
        const auto &latency = it->second;
        latencys.emplace_back(latency);
    }
}

void SdcClockLatencyContainer::getClockLatencyOnClock(std::vector<SetClockLatencyPtr> &latencys, const ClockId &clock_id) const {
    const auto &clock_to_latency = data_->getClockToLatency();
    const auto &range = clock_to_latency.equal_range(clock_id);
    for (auto it = range.first; it != range.second; ++it) {
        const auto &latency = it->second;
        latencys.emplace_back(latency);
    }
}

void SdcClockLatencyContainer::removeClock(const ClockId &clock_id) {
    auto &clock_latency = data_->getClockToLatency();
    clock_latency.erase(clock_id);
    auto &pin_latency = data_->getPinToLatency();
    auto it = pin_latency.begin();
    std::vector<decltype(it)> need_removed_its;
    for (; it != pin_latency.end(); ++it) {
        ClockLatencyOnPinPtr &latency = it->second;
        auto &clocks = latency->getPinAssociatedClocks();
        clocks.erase(clock_id);
        if (clocks.empty()) {
            need_removed_its.emplace_back(it);
        }
    }
    for (auto &rm_it : need_removed_its) {
        pin_latency.erase(rm_it);
    }
}

std::ostream &operator<<(std::ostream &os, SdcClockLatencyContainer &rhs) {
    auto print_clock_latency = [](std::ostream &os, const auto &latency) {
        os << "set_clock_latency ";
        os << latency->getDelay() << " ";
        const Flag &f1 = Flag("-rise", latency->getRise());
        const Flag &f2 = Flag("-fall", latency->getFall());
        const Flag &f3 = Flag("-min", latency->getMin());
        const Flag &f4 = Flag("-max", latency->getMax());
        const Flag &f5 = Flag("-dynamic", latency->getDynamic());
        const Flag &f6 = Flag("-source", latency->getSource());
        const Flag &f7 = Flag("-early", latency->getEarly());
        const Flag &f8 = Flag("-late", latency->getLate());
        os << ContainerDataPrint::getFlag(f1, f2, f3, f4, f5, f6, f7, f8);
    };
    const auto &data = rhs.data_;
    auto clock_id_to_name = std::bind(&ClockLatencyContainerData::clockIdToName, rhs.data_, std::placeholders::_1);
    const auto &clock_latency = data->getClockToLatency();
    for (const auto &clock_to_latency : clock_latency) {
        const ClockId &clock_id = clock_to_latency.first;
        const SetClockLatencyPtr &latency = clock_to_latency.second;
        print_clock_latency(os, latency);
        os << ContainerDataPrint::clockIdToName(clock_id_to_name, clock_id);
        os << "\n";
    }
    const auto &pin_latency = data->getPinToLatency();
    for (const auto &pin_to_latency : pin_latency) {
        const ObjectId &pin_id = pin_to_latency.first;
        const ClockLatencyOnPinPtr &latency = pin_to_latency.second;
        print_clock_latency(os, latency);
        const FlagValue &v1 = FlagValue("-clock", ContainerDataPrint::clockIdsToNameList(clock_id_to_name, latency->getPinAssociatedClocks()));
        os << ContainerDataPrint::getFlagValue(v1);
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    return os;
}

void SdcSenseContainer::getPinSense(std::vector<SetSensePtr> &senses, const ObjectId &pin_id) const {
    const auto &pin_sense = data_->getPinSense();
    const auto &range = pin_sense.equal_range(pin_id);
    for (auto it = range.first; it != range.second; ++it) {
        const auto &sense = it->second;
        senses.emplace_back(sense);
    }
}

void SdcSenseContainer::removeClock(const ClockId &clock_id) {
    auto &pin_sense = data_->getPinSense();
    auto it = pin_sense.begin();
    std::vector<decltype(it)> need_removed_its;
    for (; it != pin_sense.end(); ++it) {
        SetSensePtr &sense = it->second;
        auto &clocks = sense->getClocks();
        clocks.erase(clock_id);
        if (clocks.empty()) {
            need_removed_its.emplace_back(it);
        }
    }
    for (auto &rm_it : need_removed_its) {
        pin_sense.erase(rm_it);
    }
}

std::ostream &operator<<(std::ostream &os, SdcSenseContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &pin_sense = data->getPinSense();
    auto clock_id_to_name = std::bind(&SenseContainerData::clockIdToName, rhs.data_, std::placeholders::_1);
    for (const auto &pin_to_sense : pin_sense) {
        const ObjectId &pin_id = pin_to_sense.first;
        const SetSensePtr &sense = pin_to_sense.second;
        os << "set_sense ";
        const FlagValue &v1 = FlagValue("-type", toString(sense->getType()));
        const FlagValue &v2 = FlagValue("-pulse", toString(sense->getPulse()));
        const FlagValue &v3 = FlagValue("-clocks", ContainerDataPrint::clockIdsToNameList(clock_id_to_name, sense->getClocks()));
        os << ContainerDataPrint::getFlagValue(v1, v2, v3);

        const Flag &f1 = Flag("-non_unate", sense->getNonUnate());
        const Flag &f2 = Flag("-positive", sense->getPositive());
        const Flag &f3 = Flag("-negative", sense->getNegative());
        const Flag &f4 = Flag("-clock_leaf", sense->getClockLeaf());
        const Flag &f5 = Flag("-stop_propagation", sense->getStopPropation());
        os << ContainerDataPrint::getFlag(f1, f2, f3, f4, f5);

        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    return os;
}

void SdcClockTransitionContainer::getTransition(std::vector<SetClockTransitionPtr> &transitions, const ClockId &clock_id) const {
    const auto &clock_transitions = data_->getClockTransitions();
    const auto &range = clock_transitions.equal_range(clock_id);
    for (auto it = range.first; it != range.second; ++it) {
        const auto &transition = it->second;
        transitions.emplace_back(transition);
    }
}

void SdcClockTransitionContainer::removeClock(const ClockId &clock_id) {
    auto &clock_transitions = data_->getClockTransitions();
    clock_transitions.erase(clock_id);
}

std::ostream &operator<<(std::ostream &os, SdcClockTransitionContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &clock_transitions = data->getClockTransitions();
    for (const auto &clock_to_transitions : clock_transitions) {
        const ClockId &clock_id = clock_to_transitions.first;
        const SetClockTransitionPtr &transitions = clock_to_transitions.second;
        os << "set_clock_transition ";
        os << transitions->getTransition()<< " ";
        const Flag &f1 = Flag("-rise", transitions->getRise());
        const Flag &f2 = Flag("-fall", transitions->getFall());
        const Flag &f3 = Flag("-min", transitions->getMin());
        const Flag &f4 = Flag("-max", transitions->getMax());
        os << ContainerDataPrint::getFlag(f1, f2, f3, f4);
        os << ContainerDataPrint::getClockPrintName(data->clockIdToName(clock_id));
        os << "\n";
    }
    return os;
}

const float SdcClockUncertaintyContainer:: getUncertaintyOnPin(bool is_setup, const ObjectId &pin_id) const {
    const auto &pin_uncertainty = data_->getPinUncertainty();
    const auto &range = pin_uncertainty.equal_range(pin_id);
    for (auto it = range.first; it != range.second; ++it) {
        const SetClockUncertaintyPtr &uncertainty = it->second;
        if (uncertainty->getSetup() == is_setup) {
            return uncertainty->getUncertainty();
        }
    }
    return 0.0;
}

const float SdcClockUncertaintyContainer::getUncertaintyOnClock(bool is_setup, const ClockId &clock_id) const {
    const auto &clock_uncertainty = data_->getClockUncertainty();
    const auto &range = clock_uncertainty.equal_range(clock_id);
    for (auto it = range.first; it != range.second; ++it) {
        const SetClockUncertaintyPtr &uncertainty = it->second;
        if (uncertainty->getSetup() == is_setup) {
            return uncertainty->getUncertainty();
        }
    }
    return 0.0;
}

void SdcClockUncertaintyContainer::removeClock(const ClockId &clock_id) {
    auto &clock_uncertainty = data_->getClockUncertainty();
    clock_uncertainty.erase(clock_id);
    auto &inter_clock_uncertainty = data_->getInterClockUncertainty();
    auto it = inter_clock_uncertainty.begin();
    std::vector<decltype(it)> need_removed_its;
    for (; it != inter_clock_uncertainty.end(); ++it) {
        const InterClockPair &clock_pair = it->first;
        if ((clock_pair.first == clock_id) or (clock_pair.second == clock_id)) {
            need_removed_its.emplace_back(it);
        }
    }
    for (auto &it : need_removed_its) {
        inter_clock_uncertainty.erase(it);
    }
}

std::ostream &operator<<(std::ostream &os, SdcClockUncertaintyContainer &rhs) {
    const auto &data = rhs.data_;
    auto print_clock_uncertainty = [](std::ostream &os, const auto &uncertainty) {
        os << "set_clock_uncertainty ";
        os << uncertainty->getUncertainty() << " ";
        const Flag &f1 = Flag("-setup", uncertainty->getSetup());
        const Flag &f2 = Flag("-hold", uncertainty->getHold());
        os << ContainerDataPrint::getFlag(f1, f2);
    };
    const auto &clock_uncertainty = data->getClockUncertainty();
    auto clock_id_to_name = std::bind(&ClockUncertaintyContainerData::clockIdToName, rhs.data_, std::placeholders::_1);
    for (const auto &clock_to_uncertainty : clock_uncertainty) {
        const ClockId &clock_id = clock_to_uncertainty.first;
        const SetClockUncertaintyPtr &uncertainty = clock_to_uncertainty.second;
        print_clock_uncertainty(os, uncertainty);
        os << ContainerDataPrint::getClockPrintName(data->clockIdToName(clock_id));
        os << "\n";
    }
    const auto &pin_uncertainty = data->getPinUncertainty();
    for (const auto &pin_to_uncertainty : pin_uncertainty) {
        const ObjectId &pin_id = pin_to_uncertainty.first;
        const SetClockUncertaintyPtr &uncertainty = pin_to_uncertainty.second;
        print_clock_uncertainty(os, uncertainty);
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    const auto &inter_clock_uncertainty = data->getInterClockUncertainty();
    for (const auto &inter_clock_to_uncertainty : inter_clock_uncertainty) {
        const InterClockPair &clock_pair = inter_clock_to_uncertainty.first;
        const InterClockUncertaintyPtr &interclock_uncertainty = inter_clock_to_uncertainty.second;
        print_clock_uncertainty(os, interclock_uncertainty);
        if (interclock_uncertainty->getRiseFrom() and interclock_uncertainty->getFallFrom()) {
            os << "-from ";
        } else {
            const Flag &f1 = Flag("-rise_from", interclock_uncertainty->getRiseFrom());
            const Flag &f2 = Flag("-fall_from", interclock_uncertainty->getFallFrom());
            os << ContainerDataPrint::getFlag(f1, f2);
        }
        const ClockId &from_clock = clock_pair.first;
        os << ContainerDataPrint::getClockPrintName(data->clockIdToName(from_clock)) << " ";
        if (interclock_uncertainty->getRiseTo() and interclock_uncertainty->getFallTo()) {
            os << "-to ";
        } else {
            const Flag &f1 = Flag("-rise_to", interclock_uncertainty->getRiseTo());
            const Flag &f2 = Flag("-fall_to", interclock_uncertainty->getFallTo());
            os << ContainerDataPrint::getFlag(f1, f2);
        }
        const ClockId &to_clock = clock_pair.second;
        os << ContainerDataPrint::clockIdToName(clock_id_to_name, to_clock);
        os << "\n";
    }

    return os;
}

void SdcDataCheckContainer::getDataCheckOnInterPins(std::vector<SetDataCheckPtr>& checks, const ObjectId &from_pin_id, const ObjectId &to_pin_id) const {
    const auto &pin_data_check = data_->getPinDataCheck();
    const auto &range = pin_data_check.equal_range(PinPair(from_pin_id, to_pin_id));
    for (auto it = range.first; it != range.second; ++it) {
        checks.emplace_back(it->second);
    }
}

void SdcDataCheckContainer::removeClock(const ClockId &clock_id) {
    auto &pin_data_check = data_->getPinDataCheck();
    auto it = pin_data_check.begin();
    std::vector<decltype(it)> need_removed_its;
    for (; it != pin_data_check.end(); ++it) {
        SetDataCheckPtr &check = it->second;
        if (check->getClock() == clock_id) {
            need_removed_its.emplace_back(it);
        }
    }
    for (auto &it : need_removed_its) {
        pin_data_check.erase(it);
    }
}

std::ostream &operator<<(std::ostream &os, SdcDataCheckContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &pin_data_check = data->getPinDataCheck();
    for (const auto &pin_to_data_check : pin_data_check) {
        const PinPair &pin_pair = pin_to_data_check.first;
        const SetDataCheckPtr &check = pin_to_data_check.second;
        os << "set_data_check ";
        os << check->getValue() << " ";
        const Flag &f1 = Flag("-setup", check->getSetup());
        const Flag &f2 = Flag("-hold", check->getHold());
        os << ContainerDataPrint::getFlag(f1, f2);
        const FlagValue &v1 = FlagValue("-clock", ContainerDataPrint::getClockPrintName(data->clockIdToName(check->getClock())));
        os << ContainerDataPrint::getFlagValue(v1);
        if (check->getRiseFrom() and check->getFallFrom()) {
            os << "-from ";
        } else {
            const Flag &f1 = Flag("-rise_from", check->getRiseFrom());
            const Flag &f2 = Flag("-fall_from", check->getFallFrom());
            os << ContainerDataPrint::getFlag(f1, f2);
        }
        const ObjectId &from_pin = pin_pair.first;
        os << ContainerDataPrint::pinIdToFullName(from_pin) << " ";
        if (check->getRiseTo() and check->getFallTo()) {
            os << "-to ";
        } else {
            const Flag &f1 = Flag("-rise_to", check->getRiseTo());
            const Flag &f2 = Flag("-fall_to", check->getFallTo());
            os << ContainerDataPrint::getFlag(f1, f2);
        }
        const ObjectId &to_pin = pin_pair.second;
        os << ContainerDataPrint::pinIdToFullName(to_pin);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcDisableTimingContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &inst_to_disable_timing = data->getInstDisableTiming();
    for (const auto &inst_value: inst_to_disable_timing) {
        const ObjectId &inst_id = inst_value.first;
        const SetDisableTimingPtr &timing = inst_value.second;
        const ObjectId &from_id = timing->getFromId();
        const ObjectId &to_id = timing->getToId();
        os << "set_disable_timing ";
        const FlagValue &v1 = FlagValue("-from", ContainerDataPrint::pinIdToFullName(from_id));
        const FlagValue &v2 = FlagValue("-to", ContainerDataPrint::pinIdToFullName(to_id));
        os << ContainerDataPrint::getFlagValue(v1, v2);
        os << ContainerDataPrint::instIdToName(inst_id);
        os << "\n";
    }
    const auto &tcell_to_disable_timing = data->getTCellDisableTiming();
    for (const auto &tcell_value: tcell_to_disable_timing) {
        const ObjectId &tcell_id = tcell_value.first;
        const SetDisableTimingPtr &timing = tcell_value.second;
        const ObjectId &from_id = timing->getFromId();
        const ObjectId &to_id = timing->getToId();
        os << "set_disable_timing ";
        const FlagValue &v1 = FlagValue("-from", ContainerDataPrint::ttermIdToName(from_id));
        const FlagValue &v2 = FlagValue("-to", ContainerDataPrint::ttermIdToName(to_id));
        os << ContainerDataPrint::getFlagValue(v1, v2);
        os << ContainerDataPrint::tcellIdToName(tcell_id);
        os << "\n";
    }
    const auto &pin_to_disable_timing = data->getPinDisableTiming();
    for (const auto &pin_id: pin_to_disable_timing) {
        os << "set_disable_timing ";
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    const auto &tterm_to_disable_timing = data->getTTermDisableTiming();
    for (const auto &tterm_id: tterm_to_disable_timing) {
        os << "set_disable_timing ";
        os << ContainerDataPrint::ttermIdToFullName(tterm_id);
        os << "\n";
    }
    return os;
}

void SdcFalsePathContainer::removeClock(const ClockId &clock_id) {
    auto &false_paths = data_->getFalsePaths();
    removeClockForExceptionPathContainer(false_paths, clock_id);
}

std::ostream &operator<<(std::ostream &os, SdcFalsePathContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &false_paths = data->getFalsePaths();
    auto clock_id_to_name = std::bind(&FalsePathContainerData::clockIdToName, rhs.data_, std::placeholders::_1);
    for (const auto &false_path : false_paths) {
        os << "set_false_path ";
        const FlagValue &v1 = FlagValue("-comment", ContainerDataPrint::addDoubleQuotes(false_path->getComment()));
        os << ContainerDataPrint::getFlagValue(v1);
        const Flag &f1 = Flag("-setup", false_path->getSetup());
        const Flag &f2 = Flag("-hold", false_path->getHold());
        const Flag &f3 = Flag("-rise", false_path->getRise());
        const Flag &f4 = Flag("-fall", false_path->getFall());
        os << ContainerDataPrint::getFlag(f1, f2, f3, f4);
        printFromThroughTo(os, false_path->getFrom(), false_path->getThroughs(), false_path->getTo(), clock_id_to_name);
        os << "\n";
    }
    return os;
}

void SdcIdealLatencyContainer::getPinLatency(std::vector<SetIdealLatencyPtr> &latencys, const ObjectId &pin_id) const {
    latencys.clear();
    const auto &pin_to_latency = data_->getPinToLatency();
    const auto &range = pin_to_latency.equal_range(pin_id);
    for (auto it = range.first; it != range.second; ++it) {
        latencys.emplace_back(it->second);
    }
}

void  SdcIdealTransitionContainer::getPinTransition(std::vector<SetIdealTransitionPtr> &transitions, const ObjectId &pin_id) const {
    const auto &pin_ideal_transitions = data_->getPinIdealTransitions();
    const auto &range = pin_ideal_transitions.equal_range(pin_id);
    for (auto it = range.first; it != range.second; ++it) {
        transitions.emplace_back(it->second);
    }
}

std::ostream &operator<<(std::ostream &os, SdcIdealLatencyContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &pin_latency = data->getPinToLatency();
    for (const auto &pin_to_latency : pin_latency) {
        const ObjectId &pin_id = pin_to_latency.first;
        const SetIdealLatencyPtr &latency = pin_to_latency.second;
        os << "set_ideal_latency ";
        os << latency->getValue() << " ";
        const Flag &f1 = Flag("-rise", latency->getRise());
        const Flag &f2 = Flag("-fall", latency->getFall());
        const Flag &f3 = Flag("-min", latency->getMin());
        const Flag &f4 = Flag("-max", latency->getMax());
        os << ContainerDataPrint::getFlag(f1, f2, f3, f4);
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcIdealNetworkContainer &rhs) {
    const auto &data = rhs.data_;
    const std::unordered_map<ObjectId, bool> &pins = data->getPins();
    for (const auto &pin_to_flag : pins) {
        const ObjectId &pin_id = pin_to_flag.first;
        const bool is_no_propagate = pin_to_flag.second;
        os << "set_ideal_network ";
        const Flag &f1 = Flag("-no_propagate", is_no_propagate);
        os << ContainerDataPrint::getFlag(f1);
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    const std::unordered_set<ObjectId> &nets = data->getNets();
    for (const auto &net_id : nets) {
        os << "set_ideal_network ";
        const Flag &f1 = Flag("-no_propagate", true);
        os << ContainerDataPrint::getFlag(f1);
        os << ContainerDataPrint::netIdToName(net_id);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcIdealTransitionContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &pin_ideal_transitions = data->getPinIdealTransitions();
    for (const auto &pin_to_transition : pin_ideal_transitions) {
        const ObjectId &pin_id = pin_to_transition.first;
        const SetIdealTransitionPtr &transition = pin_to_transition.second;
        os << "set_ideal_transition ";
        os << transition->getValue() << " ";
        const Flag &f1 = Flag("-rise", transition->getRise());
        const Flag &f2 = Flag("-fall", transition->getFall());
        const Flag &f3 = Flag("-min", transition->getMin());
        const Flag &f4 = Flag("-max", transition->getMax());
        os << ContainerDataPrint::getFlag(f1, f2, f3, f4);
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    return os;
}

void SdcInputDelayContainer::getPinInputDelay(std::vector<SetInputDelayPtr> &delays, const ObjectId &pin_id) const {
    const auto &pin_input_delays = data_->getPinInputDelays();
    const auto &range = pin_input_delays.equal_range(pin_id);
    for (auto it = range.first; it != range.second; ++it) {
        delays.emplace_back(it->second);
    }
}

const float SdcMaxTimeBorrowContainer::getMaxTimeBorrowOnPin(const ObjectId &pin_id) {
    const auto &pin_max_time_borrow = data_->getPinMaxTimeBorrow();
    const auto &found = pin_max_time_borrow.find(pin_id);
    if (found == pin_max_time_borrow.end()) {
        return 0.0;
    }
    const auto &time_borrow = found->second;
    return time_borrow.getValue();
}

void SdcInputDelayContainer::removeClock(const ClockId &clock_id) {
    auto &pin_to_input_delay = data_->getPinInputDelays();
    auto it = pin_to_input_delay.begin();
    std::vector<decltype(it)> need_removed_its;
    for (; it != pin_to_input_delay.end(); ++it) {
        const SetInputDelayPtr &input_delay = it->second;
        if (input_delay->getClock() == clock_id) {
            need_removed_its.emplace_back(it);
        }
    }
    for (auto it : need_removed_its) {
        pin_to_input_delay.erase(it);
    }
}

std::ostream &operator<<(std::ostream &os, SdcInputDelayContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &pin_to_input_delay = data->getPinInputDelays();
    for (const auto &pin_value : pin_to_input_delay) {
        const ObjectId &pin_id = pin_value.first;
        const SetInputDelayPtr &input_delay = pin_value.second;
        os << "set_input_delay ";
        const Flag &f1 = Flag("-clock_fall", input_delay->getClockFall());
        const Flag &f2 = Flag("-level_sensitive", input_delay->getLevelSensitive());
        const Flag &f3 = Flag("-rise", input_delay->getRise());
        const Flag &f4 = Flag("-fall", input_delay->getFall());
        const Flag &f5 = Flag("-min", input_delay->getMin());
        const Flag &f6 = Flag("-max", input_delay->getMax());
        const Flag &f7 = Flag("-add_delay", true); //always keep -add_delay when write sdc
        const Flag &f8 = Flag("-network_latency_included", input_delay->getNetworkLatencyIncluded());
        const Flag &f9 = Flag("-source_latency_inlcuded", input_delay->getSourceLatencyIncluded());
        os << ContainerDataPrint::getFlag(f1, f2, f3, f4, f5, f6, f7, f8, f9);
        const FlagValue &v1 = FlagValue("-clock", ContainerDataPrint::getClockPrintName(data->clockIdToName(input_delay->getClock())));
        const FlagValue &v2 = FlagValue("-reference_pin", ContainerDataPrint::pinIdToFullName(input_delay->getReferencePin()));
        os << ContainerDataPrint::getFlagValue(v1, v2);
        os << input_delay->getDelayValue() << " ";
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    return os;
}

void SdcMaxDelayContainer::removeClock(const ClockId &clock_id) {
    auto &delays = data_->getMaxDelays();
    removeClockForExceptionPathContainer(delays, clock_id);
}

std::ostream &operator<<(std::ostream &os, SdcMaxDelayContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &max_delays = data->getMaxDelays();
    auto clock_id_to_name = std::bind(&MaxDelayContainerData::clockIdToName, rhs.data_, std::placeholders::_1);
    for (const auto &max_delay : max_delays) {
        os << "set_max_delay ";
        os << max_delay->getDelayValue()<< " ";
        const FlagValue &v1 = FlagValue("-comment", ContainerDataPrint::addDoubleQuotes(max_delay->getComment()));
        os << ContainerDataPrint::getFlagValue(v1);
        const Flag &f1 = Flag("-rise", max_delay->getRise());
        const Flag &f2 = Flag("-fall", max_delay->getFall());
        const Flag &f3 = Flag("-ignore_clock_latency", max_delay->getIgnoreClockLatency());
        os << ContainerDataPrint::getFlag(f1, f2, f3);
        printFromThroughTo(os, max_delay->getFrom(), max_delay->getThroughs(), max_delay->getTo(), clock_id_to_name);
        os << "\n";
    }
    return os;
}

const float SdcMaxTimeBorrowContainer::getMaxTimeBorrowOnInst(const ObjectId &inst_id) {
    const auto &inst_max_time_borrow = data_->getInstMaxTimeBorrow();
    const auto &found = inst_max_time_borrow.find(inst_id);
    if (found == inst_max_time_borrow.end()) {
        return 0.0;
    }
    const auto &time_borrow = found->second;
    return time_borrow.getValue();
}

const float SdcMaxTimeBorrowContainer::getMaxTimeBorrowOnClock(const ClockId &clock_id) {
    const auto &clock_max_time_borrow = data_->getClockMaxTimeBorrow();
    const auto &found = clock_max_time_borrow.find(clock_id);
    if (found == clock_max_time_borrow.end()) {
        return 0.0;
    }
    const auto &time_borrow = found->second;
    return time_borrow.getValue();
}

void SdcMaxTimeBorrowContainer::removeClock(const ClockId &clock_id) {
    auto &clock_max_time_borrow = data_->getClockMaxTimeBorrow();
    clock_max_time_borrow.erase(clock_id);
}

std::ostream &operator<<(std::ostream &os, SdcMaxTimeBorrowContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &pin_max_time_borrow = data->getPinMaxTimeBorrow();
    for (const auto &pin_value : pin_max_time_borrow) {
        const ObjectId &pin_id = pin_value.first;
        const SetMaxTimeBorrow &max_time_borrow = pin_value.second;
        os << "set_max_time_borrow ";
        os << max_time_borrow.getValue() << " ";
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    const auto &inst_max_time_borrow = data->getInstMaxTimeBorrow();
    for (const auto &inst_value : inst_max_time_borrow) {
        const ObjectId &inst_id = inst_value.first;
        const SetMaxTimeBorrow &max_time_borrow = inst_value.second;
        os << "set_max_time_borrow ";
        os << max_time_borrow.getValue() << " ";
        os << ContainerDataPrint::instIdToName(inst_id);
        os << "\n";
    }
    const auto &clock_max_time_borrow = data->getClockMaxTimeBorrow();
    for (const auto &clock_value : clock_max_time_borrow) {
        const ClockId &clock_id = clock_value.first;
        const SetMaxTimeBorrow &max_time_borrow = clock_value.second;
        os << "set_max_time_borrow ";
        os << max_time_borrow.getValue() << " ";
        os << ContainerDataPrint::getClockPrintName(data->clockIdToName(clock_id));
        os << "\n";
    }
    return os;
}

void SdcMinDelayContainer::removeClock(const ClockId &clock_id) {
    auto &delays = data_->getMinDelays();
    removeClockForExceptionPathContainer(delays, clock_id);
}

std::ostream &operator<<(std::ostream &os, SdcMinDelayContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &min_delays = data->getMinDelays();
    auto clock_id_to_name = std::bind(&MinDelayContainerData::clockIdToName, rhs.data_, std::placeholders::_1);
    for (const auto &min_delay : min_delays) {
        os << "set_min_delay ";
        os << min_delay->getDelayValue() << " ";
        const FlagValue &v1 = FlagValue("-comment", ContainerDataPrint::addDoubleQuotes(min_delay->getComment()));
        os << ContainerDataPrint::getFlagValue(v1);
        const Flag &f1 = Flag("-rise", min_delay->getRise());
        const Flag &f2 = Flag("-fall", min_delay->getFall());
        const Flag &f3 = Flag("-ignore_clock_latency", min_delay->getIgnoreClockLatency());
        os << ContainerDataPrint::getFlag(f1, f2, f3);
        printFromThroughTo(os, min_delay->getFrom(), min_delay->getThroughs(), min_delay->getTo(), clock_id_to_name);
        os << "\n";
    }
    return os;
}

void SdcMinPulseWidthContainer::getMinPulseWidthOnPin(std::vector<SetMinPulseWidthPtr> &pulse_width, const ObjectId &pin_id) const {
    pulse_width.clear();
    const auto &pin_min_pulse_width = data_->getPinMinPulseWidth();
    const auto &range = pin_min_pulse_width.equal_range(pin_id);
    for (auto it = range.first; it != range.second; ++it) {
        pulse_width.emplace_back(it->second);
    }
}

void SdcMinPulseWidthContainer::getMinPulseWidthOnInst(std::vector<SetMinPulseWidthPtr> &pulse_width, const ObjectId &inst_id) const {
    pulse_width.clear();
    const auto &inst_min_pulse_width = data_->getInstMinPulseWidth();
    const auto &range = inst_min_pulse_width.equal_range(inst_id);
    for (auto it = range.first; it != range.second; ++it) {
        pulse_width.emplace_back(it->second);
    }
}

void SdcMinPulseWidthContainer::getMinPulseWidthOnClock(std::vector<SetMinPulseWidthPtr> &pulse_width, const ObjectId &clock_id) const {
    pulse_width.clear();
    const auto &clock_min_pulse_width = data_->getClockMinPulseWidth();
    const auto &range = clock_min_pulse_width.equal_range(clock_id);
    for (auto it = range.first; it != range.second; ++it) {
        pulse_width.emplace_back(it->second);
    }
}

void SdcMinPulseWidthContainer::removeClock(const ClockId &clock_id) {
    auto &clock_to_min_pulse_width = data_->getClockMinPulseWidth();
    clock_to_min_pulse_width.erase(clock_id);
}

std::ostream &operator<<(std::ostream &os, SdcMinPulseWidthContainer &rhs) {
    auto print_min_pulse_width = [](std::ostream &os, const SetMinPulseWidthPtr &min_pulse_width) {
        os << "set_min_pulse_width ";
        os << min_pulse_width->getValue() << " ";
        const Flag &f1 = Flag("-low", min_pulse_width->getLow());
        const Flag &f2 = Flag("-high", min_pulse_width->getHigh());
        os << ContainerDataPrint::getFlag(f1, f2);
    };
    const auto &data = rhs.data_;
    const auto &pin_to_min_pulse_width = data->getPinMinPulseWidth();
    for (const auto &pin_value : pin_to_min_pulse_width) {
        const ObjectId &pin_id = pin_value.first;
        const SetMinPulseWidthPtr &min_pulse_width = pin_value.second;
        print_min_pulse_width(os, min_pulse_width);
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    const auto &inst_to_min_pulse_width = data->getInstMinPulseWidth();
    for (const auto &inst_value : inst_to_min_pulse_width) {
        const ObjectId &inst_id = inst_value.first;
        const SetMinPulseWidthPtr &min_pulse_width = inst_value.second;
        print_min_pulse_width(os, min_pulse_width);
        os << ContainerDataPrint::instIdToName(inst_id);
        os << "\n";
    }
    const auto &design_to_min_pulse_width = data->getDesignMinPulseWidth();
    for (const auto &design_value : design_to_min_pulse_width) {
        const ObjectId &design_id = design_value.first;
        const SetMinPulseWidthPtr &min_pulse_width = design_value.second;
        print_min_pulse_width(os, min_pulse_width);
        os << ContainerDataPrint::cellIdToName(design_id);
        os << "\n";
    }
    const auto &clock_to_min_pulse_width = data->getClockMinPulseWidth();
    for (const auto &clock_value : clock_to_min_pulse_width) {
        const ClockId &clock_id = clock_value.first;
        const SetMinPulseWidthPtr &min_pulse_width = clock_value.second;
        print_min_pulse_width(os, min_pulse_width);
        os << ContainerDataPrint::getClockPrintName(data->clockIdToName(clock_id));
        os << "\n";
    }
    return os;
}

void SdcMulticyclePathContainer::removeClock(const ClockId &clock_id) {
    auto &paths = data_->getMultiCyclePaths();
    removeClockForExceptionPathContainer(paths, clock_id);
}

std::ostream &operator<<(std::ostream &os, SdcMulticyclePathContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &multi_cycle_paths = data->getMultiCyclePaths();
    auto clock_id_to_name = std::bind(&MulticyclePathContainerData::clockIdToName, rhs.data_, std::placeholders::_1);
    for (const auto &multi_cycle_path :multi_cycle_paths) {
        os << "set_multicycle_path ";
        os << multi_cycle_path->getPathMultiplier()<< " ";
        const FlagValue &v1 = FlagValue("-comment", ContainerDataPrint::addDoubleQuotes(multi_cycle_path->getComment()));
        os << ContainerDataPrint::getFlagValue(v1);
        const Flag &f1 = Flag("-setup", multi_cycle_path->getSetup());
        const Flag &f2 = Flag("-hold", multi_cycle_path->getHold());
        const Flag &f3 = Flag("-rise", multi_cycle_path->getRise());
        const Flag &f4 = Flag("-fall", multi_cycle_path->getFall());
        const Flag &f5 = Flag("-start", multi_cycle_path->getStart());
        const Flag &f6 = Flag("-end", multi_cycle_path->getEnd());
        os << ContainerDataPrint::getFlag(f1, f2, f3, f4, f5, f6);
        printFromThroughTo(os, multi_cycle_path->getFrom(), multi_cycle_path->getThroughs(), multi_cycle_path->getTo(), clock_id_to_name);
        os << "\n";
    }
    return os;
}

void SdcOutputDelayContainer::getPinOutputDelay(std::vector<SetOutputDelayPtr> &delays, const ObjectId &pin_id) const {
    const auto &pin_output_delays = data_->getPinOutputDelays();
    const auto &range = pin_output_delays.equal_range(pin_id);
    for (auto it = range.first; it != range.second; ++it) {
        delays.emplace_back(it->second);
    }
}

void SdcOutputDelayContainer::removeClock(const ClockId &clock_id) {
    auto &pin_to_output_delay = data_->getPinOutputDelays();
    auto it = pin_to_output_delay.begin();
    std::vector<decltype(it)> need_removed_its;
    for (; it != pin_to_output_delay.end(); ++it) {
        const SetOutputDelayPtr &output_delay = it->second;
        if (output_delay->getClock() == clock_id) {
            need_removed_its.emplace_back(it);
        }
    }
    for (auto it : need_removed_its) {
        pin_to_output_delay.erase(it);
    }
}

std::ostream &operator<<(std::ostream &os, SdcOutputDelayContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &pin_to_output_delay = data->getPinOutputDelays();
    for (const auto &pin_value : pin_to_output_delay) {
        const ObjectId &pin_id = pin_value.first;
        const SetOutputDelayPtr &output_delay = pin_value.second;
        os << "set_output_delay ";
        const Flag &f1 = Flag("-clock_fall", output_delay->getClockFall());
        const Flag &f2 = Flag("-level_sensitive", output_delay->getLevelSensitive());
        const Flag &f3 = Flag("-rise", output_delay->getRise());
        const Flag &f4 = Flag("-fall", output_delay->getFall());
        const Flag &f5 = Flag("-min", output_delay->getMin());
        const Flag &f6 = Flag("-max", output_delay->getMax());
        const Flag &f7 = Flag("-add_delay", true); //always keep -add_delay when write sdc
        const Flag &f8 = Flag("-network_latency_included", output_delay->getNetworkLatencyIncluded());
        const Flag &f9 = Flag("-source_latency_inlcuded", output_delay->getSourceLatencyIncluded());
        os << ContainerDataPrint::getFlag(f1,f2,f3,f4,f5,f6,f7,f8,f9);
        const FlagValue &v1 = FlagValue("-clock", ContainerDataPrint::getClockPrintName(data->clockIdToName(output_delay->getClock())));
        const FlagValue &v2 = FlagValue("-reference_pin", ContainerDataPrint::pinIdToFullName(output_delay->getReferencePin()));
        os << ContainerDataPrint::getFlagValue(v1, v2);
        os << output_delay->getDelayValue() << " ";
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    return os;
}

bool SdcPropagatedClockContainer::isPropagatedPin(const ObjectId &pin_id) const {
    const auto &pins = data_->getPins();
    const auto &found = pins.find(pin_id);
    if (found != pins.end()) {
        return true;
    }
    return false;
}

void SdcPropagatedClockContainer::removeClock(const ClockId &clock_id) {
    auto &clocks = data_->getClocks();
    clocks.erase(clock_id);
}

std::ostream &operator<<(std::ostream &os, SdcPropagatedClockContainer &rhs) {
    const auto &data = rhs.data_;
    const auto &pins = data->getPins();
    for (const auto &pin_id: pins) {
        os << "set_propagated_clock ";
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    const auto &clocks = data->getClocks();
    for (const auto &clock_id: clocks) {
        os << "set_propagated_clock ";
        os << ContainerDataPrint::getClockPrintName(data->clockIdToName(clock_id));
        os << "\n";
    }
    return os;
}

// environment commands
const CaseValue SdcCaseAnalysisContainer::getPinValue(const ObjectId &pin_id) const {
    const auto &pin_to_case_analysis = data_->getPinToCaseAnalysis();
    const auto &found = pin_to_case_analysis.find(pin_id);
    if (found != pin_to_case_analysis.end()) {
        const auto &case_analysis = found->second;
        return case_analysis.getValue();
    }
    return CaseValue::kUnknown;
}

std::ostream &operator<<(std::ostream &os, SdcCaseAnalysisContainer &rhs) {
    const auto &pin_to_case_analysis = rhs.data_->getPinToCaseAnalysis();
    for ( const auto &pin_value : pin_to_case_analysis ) {
        const auto &pin_id = pin_value.first;
        const auto& case_analysis = pin_value.second;
        os << "set_case_analysis ";
        os << toString(case_analysis.getValue()) << " ";
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcDriveContainer &rhs) {
    const auto &data = rhs.getData();
    const auto &port_value = data->getPortDrives();
    for (const auto &port_to_drives : port_value) {
        const ObjectId &port_id = port_to_drives.first;
        const SetDrivePtr &drive = port_to_drives.second;
        os << "set_drive ";
        os << drive->getResistance() << " ";
        const Flag &f1 = Flag("-rise", drive->getRise());
        const Flag &f2 = Flag("-fall", drive->getFall());
        const Flag &f3 = Flag("-min", drive->getMin());
        const Flag &f4 = Flag("-max", drive->getMax());
        os << ContainerDataPrint::getFlag(f1, f2, f3, f4);
        os << ContainerDataPrint::pinIdToFullName(port_id);
        os << "\n";
    }
    return os;
}

bool SdcDrivingCellContainer::hasDrivingCell(const ObjectId &port_id) const {
    const auto &data = getData();
    const auto &port_value = data->getPortDrivingCell();
    const auto &found = port_value.find(port_id);
    return (found != port_value.end());
}

SetDrivingCellPtr SdcDrivingCellContainer::getDrivingCell(Pin *pin, const SetDrivingCellCondition &condition) const {
    if (!pin) {
        return nullptr;
    }
    const auto &data = getData();
    const auto &port_value = data->getPortDrivingCell();
    const auto &range = port_value.equal_range(pin->getId());
    for (auto it = range.first; it != range.second; ++it) {
        const SetDrivingCellPtr &driving_cell = it->second;
        if (driving_cell->getCondition() == condition) {
            return driving_cell;
        }
    }
    return nullptr;
}

float SdcDrivingCellContainer::getInputRiseSlew(Pin* pin, const SetDrivingCellCondition &condition) const {
    const SetDrivingCellPtr &driving_cell = getDrivingCell(pin, condition);
    return driving_cell->getInputTransitionRise();
}

float SdcDrivingCellContainer::getInputFallSlew(Pin* pin, const SetDrivingCellCondition &condition) const {
    const SetDrivingCellPtr &driving_cell = getDrivingCell(pin, condition);
    return driving_cell->getInputTransitionFall();
}

TCell* SdcDrivingCellContainer::getCell(Pin* pin, const SetDrivingCellCondition &condition) const {
    const SetDrivingCellPtr &driving_cell = getDrivingCell(pin, condition);
    TTerm *to_tterm = Object::addr<TTerm>(driving_cell->getToTTerm());
    assert(to_tterm);
    if (!to_tterm) {
        //TODO should not happen, error messages
        return nullptr;
    }
    return to_tterm->getTCell();
}

TimingArc* SdcDrivingCellContainer::getTimingArc(Pin* pin, const SetDrivingCellCondition &condition) const {
    const SetDrivingCellPtr &driving_cell = getDrivingCell(pin, condition);
    TTerm *to_tterm = Object::addr<TTerm>(driving_cell->getToTTerm());
    assert(to_tterm);
    if (!to_tterm) {
        //TODO should not happen, error messages
        return nullptr;
    }
    const std::vector<TimingArc *> &arcs = to_tterm->getTimingArcs();
    for (const auto &arc : arcs) {
        if (!arc) {
            continue;
        }
        return arc;
    }
    return nullptr;
}

std::ostream &operator<<(std::ostream &os, SdcDrivingCellContainer &rhs) {
    const auto &data = rhs.getData();
    const auto &port_value = data->getPortDrivingCell();
    for (const auto &port_to_driving_cell : port_value) {
        const ObjectId &port_id = port_to_driving_cell.first;
        const SetDrivingCellPtr &driving_cell = port_to_driving_cell.second;
        TTerm *to_tterm = Object::addr<TTerm>(driving_cell->getToTTerm());
        assert(to_tterm);
        TCell *tcell = to_tterm->getTCell();
        assert(tcell);
        const ObjectId &lib_id = tcell->getGroupId();
        TLib *lib = Object::addr<TLib>(lib_id);
        assert(lib);
        os << "set_driving_cell ";
        const FlagValue &v1 = FlagValue("-from_pin", ContainerDataPrint::ttermIdToName(driving_cell->getFromTTerm()));
        const FlagValue &v2 = FlagValue("-pin", ContainerDataPrint::ttermIdToName(driving_cell->getToTTerm()));
        const FlagValue &v3 = FlagValue("-input_transition_rise", std::to_string(driving_cell->getInputTransitionRise()));
        const FlagValue &v4 = FlagValue("-input_transition_fall", std::to_string(driving_cell->getInputTransitionFall()));
        const FlagValue &v5 = FlagValue("-lib_cell", tcell->getName());
        const FlagValue &v6 = FlagValue("-library", lib->getName());
        os << ContainerDataPrint::getFlagValue(v1, v2, v3, v4, v5, v6);
        const Flag &f1 = Flag("-rise", driving_cell->getRise());
        const Flag &f2 = Flag("-fall", driving_cell->getFall());
        const Flag &f3 = Flag("-dont_scale", driving_cell->getDontScale());
        const Flag &f4 = Flag("-no_design_rule", driving_cell->getNoDesignRule());
        const Flag &f5 = Flag("-min", driving_cell->getMin());
        const Flag &f6 = Flag("-max", driving_cell->getMax());
        os << ContainerDataPrint::getFlag(f1, f2, f3, f4, f5, f6);
        os << ContainerDataPrint::pinIdToFullName(port_id);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcFanoutLoadContainer &rhs) {
    const auto &data = rhs.getData();
    const auto &port_value = data->getPortFanoutLoad();
    for (const auto &port_to_fanout_load : port_value) {
        const ObjectId &port_id = port_to_fanout_load.first;
        const auto &fanout_load = port_to_fanout_load.second;
        os << "set_fanout_load ";
        os << fanout_load.getValue() << " ";
        os << ContainerDataPrint::pinIdToFullName(port_id);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcInputTransitionContainer &rhs) {
    const auto &data = rhs.getData();
    const auto &port_value = data->getPortInputTransitions();
    for (const auto &port_to_input_trans : port_value) {
        const ObjectId &port_id = port_to_input_trans.first;
        const SetInputTransitionPtr &input_trans = port_to_input_trans.second;
        os << "set_input_transition ";
        os << input_trans->getTransition() << " ";
        const Flag &f1 = Flag("-rise", input_trans->getRise());
        const Flag &f2 = Flag("-fall", input_trans->getFall());
        const Flag &f3 = Flag("-min", input_trans->getMin());
        const Flag &f4 = Flag("-max", input_trans->getMax());
        os << ContainerDataPrint::getFlag(f1, f2, f3, f4);
        os << ContainerDataPrint::pinIdToFullName(port_id);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcLoadContainer &rhs) {
    const auto &data = rhs.getData();
    auto print_load = [](std::ostream &os, const SetLoadPtr &load_cap) {
        os << "set_load ";
        os << load_cap->getCap() << " ";
        const Flag &f1 = Flag("-min", load_cap->getMin());
        const Flag &f2 = Flag("-max", load_cap->getMax());
        const Flag &f3 = Flag("-rise", load_cap->getRise());
        const Flag &f4 = Flag("-fall", load_cap->getFall());
        const Flag &f5 = Flag("-substract_pin_load", load_cap->getSubstractPinLoad());
        const Flag &f6 = Flag("-pin_load", load_cap->getPinLoad());
        const Flag &f7 = Flag("-wire_load", load_cap->getWireLoad());
        os << ContainerDataPrint::getFlag(f1, f2, f3, f4, f5, f6, f7);
    };
    const auto &port_value = data->getPortLoadCaps();
    for (const auto &port_to_load_cap : port_value) {
        const ObjectId &port_id = port_to_load_cap.first;
        const SetLoadPtr &load_cap = port_to_load_cap.second;
        print_load(os, load_cap);
        os << ContainerDataPrint::pinIdToFullName(port_id);
        os << "\n";
    }
    const auto &net_value = data->getNetLoadCaps();
    for (const auto &net_to_load_cap : net_value) {
        const ObjectId &net_id = net_to_load_cap.first;
        const SetLoadPtr &load_cap = net_to_load_cap.second;
        print_load(os, load_cap);
        os << ContainerDataPrint::netIdToName(net_id);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcLogicContainer &rhs) {
    const auto &data = rhs.getData();
    const auto &port_value = data->getPortLogicValue();
    for (const auto &port_to_logic : port_value) {
        const ObjectId &port_id = port_to_logic.first;
        const LogicValue value = port_to_logic.second.getValue();
        switch (value) {
            case LogicValue::kDontCare :
                os << "set_logic_dc ";
                break;
            case LogicValue::kOne :
                os << "set_logic_one ";
                break;
            case LogicValue::kZero :
                os << "set_logic_zero ";
                break;
            case LogicValue::kUnknown :
            default:
                continue;
        }
        os << ContainerDataPrint::pinIdToFullName(port_id);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcMaxAreaContainer &rhs) {
    const auto &data = rhs.getData();
    const auto &design_value = data->getDesignMaxArea();
    for (const auto &design_to_max_area : design_value) {
        //TODO use current_design to print cell_id
        const ObjectId &cell_id = design_to_max_area.first;
        const auto &max_area = design_to_max_area.second;
        os << "set_max_area ";
        os << max_area.getAreaValue();
        os << "\n";
    }
    return os;
}

const SetMaxCapacitancePtr SdcMaxCapacitanceContainer::getPinCap(const ObjectId &pin_id, const ObjectId &top_cell_id) const {
    const auto &design_max_caps = data_->getDesignMaxCaps();
    const auto &found = design_max_caps.find(top_cell_id);
    if (found != design_max_caps.end()) {
        return found->second;
    }
    const auto &pin_max_caps = data_->getPinMaxCaps();
    const auto &found_pin = pin_max_caps.find(pin_id);
    if (found_pin != pin_max_caps.end()) {
        return found_pin->second;
    }
    return nullptr;
}

std::ostream &operator<<(std::ostream &os, SdcMaxCapacitanceContainer &rhs) {
    const auto &data = rhs.getData();
    const auto &pin_value = data->getPinMaxCaps();
    for (const auto &pin_to_max_caps : pin_value) {
        const ObjectId &pin_id = pin_to_max_caps.first;
        const SetMaxCapacitancePtr &max_caps = pin_to_max_caps.second;
        os << "set_max_capacitance ";
        os << max_caps->getCapValue() << " ";
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    const auto &design_value = data->getDesignMaxCaps();
    for (const auto &design_to_max_caps : design_value) {
        const ObjectId &cell_id = design_to_max_caps.first;
        const SetMaxCapacitancePtr &max_caps = design_to_max_caps.second;
        os << "set_max_capacitance ";
        os << max_caps->getCapValue() << " ";
        os << ContainerDataPrint::cellIdToName(cell_id);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcMaxFanoutContainer &rhs) {
    const auto &data = rhs.getData();
    const auto &pin_value = data->getPortMaxFanout();
    for (const auto &pin_to_max_fanout : pin_value) {
        const auto  &pin_id = pin_to_max_fanout.first;
        const auto  &max_fanout = pin_to_max_fanout.second;
        os << "set_max_fanout ";
        os << max_fanout.getFanoutValue() << " ";
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    const auto &design_value = data->getDesignMaxFanout();
    for (const auto &design_to_max_fanout : design_value) {
        const auto  &cell_id = design_to_max_fanout.first;
        const auto  &max_fanout = design_to_max_fanout.second;
        os << "set_max_fanout ";
        os << max_fanout.getFanoutValue() << " ";
        os << ContainerDataPrint::cellIdToName(cell_id);
        os << "\n";
    }
    return os;
}

void SdcMaxTransitionContainer::removeClock(const ClockId &clock_id) {
    auto &clock_value = data_->getClockMaxTransition();
    clock_value.erase(clock_id);
}

std::ostream &operator<<(std::ostream &os, SdcMaxTransitionContainer &rhs) {
    auto print_transition = [](std::ostream &os, const SetMaxTransitionPtr &transition) {
        os << "set_max_transition ";
        os << transition->getTransitionValue() << " ";
        const Flag &f1 = Flag("-clock_path", transition->getClockPath());
        const Flag &f2 = Flag("-fall", transition->getFall());
        const Flag &f3 = Flag("-rise", transition->getRise());
        os << ContainerDataPrint::getFlag(f1, f2, f3);
    };
    const auto &data = rhs.getData();
    const auto &pin_value = data->getPinMaxTransition();
    for (const auto &pin_to_transition : pin_value) {
        const ObjectId &pin_id = pin_to_transition.first;
        const SetMaxTransitionPtr &transition = pin_to_transition.second;
        print_transition(os, transition);
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    const auto &clock_value = data->getClockMaxTransition();
    for (const auto &clock_to_transition : clock_value) {
        const ClockId &clock_id = clock_to_transition.first;
        const SetMaxTransitionPtr &transition = clock_to_transition.second;
        print_transition(os, transition);
        os << ContainerDataPrint::getClockPrintName(data->clockIdToName(clock_id));
        os << "\n";
    }
    const auto &design_value = data->getDesignMaxTransition();
    for (const auto &design_to_transition : design_value) {
        const ObjectId &design_cell_id = design_to_transition.first;
        const SetMaxTransitionPtr &transition = design_to_transition.second;
        print_transition(os, transition);
        os << ContainerDataPrint::cellIdToName(design_cell_id);
        os << "\n";
    }
    return os;
}

const SetMinCapacitancePtr SdcMinCapacitanceContainer::getPinCap(const ObjectId &pin_id, const ObjectId &top_cell_id) const {
    const auto &design_min_caps = data_->getDesignMinCaps();
    const auto &found = design_min_caps.find(top_cell_id);
    if (found != design_min_caps.end()) {
        return found->second;
    }
    const auto &pin_min_caps = data_->getPinMinCaps();
    const auto &found_pin = pin_min_caps.find(pin_id);
    if (found_pin != pin_min_caps.end()) {
        return found_pin->second;
    }
    return nullptr;
}

std::ostream &operator<<(std::ostream &os, SdcMinCapacitanceContainer &rhs) {
    const auto &data = rhs.getData();
    const auto &pin_value = data->getPinMinCaps();
    for (const auto &pin_to_min_caps : pin_value) {
        const ObjectId &pin_id = pin_to_min_caps.first;
        const SetMinCapacitancePtr &min_caps = pin_to_min_caps.second;
        os << "set_min_capacitance ";
        os << min_caps->getCapValue() << " ";
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    const auto &design_value = data->getDesignMinCaps();
    for (const auto &design_to_min_caps : design_value) {
        const ObjectId &cell_id = design_to_min_caps.first;
        const SetMinCapacitancePtr &min_caps = design_to_min_caps.second;
        os << "set_min_capacitance ";
        os << min_caps->getCapValue() << " ";
        os << ContainerDataPrint::cellIdToName(cell_id);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcOperatingConditionsContainer &rhs) {
    const auto &data = rhs.getData();
    auto print_operating_condition = [](std::ostream &os, const SetOperatingConditionsPtr &operating_condition) {
        auto get_lib_from_condition = [](const ObjectId &condition_id) -> ObjectId {
            const auto &condition = Object::addr<OperatingConditions>(condition_id);
            if (!condition) {
                return UNINIT_OBJECT_ID;
            }
            return condition->getGroupId();
        };
        const ObjectId &condition_id = operating_condition->getCondition();
        const ObjectId &max_condition_id = operating_condition->getMaxCondition();
        const ObjectId &min_condition_id = operating_condition->getMinCondition();
        const FlagValue &v1 = FlagValue( "-analysis_type", toString(operating_condition->getAnalysisType()));
        const FlagValue &v2 = FlagValue("-max", ContainerDataPrint::idToName<OperatingConditions>(max_condition_id));
        const FlagValue &v3 = FlagValue("-min", ContainerDataPrint::idToName<OperatingConditions>(min_condition_id));
        const FlagValue &v4 = FlagValue("-library", ContainerDataPrint::libIdToName(get_lib_from_condition(condition_id)));
        const FlagValue &v5 = FlagValue("-max_library", ContainerDataPrint::libIdToName(get_lib_from_condition(max_condition_id)));
        const FlagValue &v6 = FlagValue("-min_library", ContainerDataPrint::libIdToName(get_lib_from_condition(min_condition_id)));
        os << ContainerDataPrint::getFlagValue(v1, v2, v3, v4, v5, v6);
        os << ContainerDataPrint::idToName<OperatingConditions>(condition_id) << " ";
    };
    const auto &port_value = data->getPortOperatingConditions();
    for (const auto &port_to_condition : port_value) {
        const ObjectId &port_id = port_to_condition.first;
        const SetOperatingConditionsPtr &operating_condition = port_to_condition.second;
        os << "set_operating_conditions ";
        print_operating_condition(os, operating_condition);
        const FlagValue &v7 = FlagValue("-object_list", ContainerDataPrint::pinIdToFullName(port_id));
        os << ContainerDataPrint::getFlagValue(v7);
        os << "\n";
    }
    const auto &inst_value = data->getInstOperatingConditions();
    for (const auto &inst_to_condition : inst_value) {
        const ObjectId &inst_id = inst_to_condition.first;
        const SetOperatingConditionsPtr &operating_condition = inst_to_condition.second;
        os << "set_operating_conditions ";
        print_operating_condition(os, operating_condition);
        const FlagValue &v7 = FlagValue("-object_list", ContainerDataPrint::instIdToName(inst_id));
        os << ContainerDataPrint::getFlagValue(v7);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcPortFanoutNumberContainer &rhs) {
    const auto &data = rhs.getData();
    const auto &port_value = data->getPortFanoutNumber();
    for (const auto &port_to_fanout_number : port_value) {
        const ObjectId &port_id = port_to_fanout_number.first;
        const SetPortFanoutNumber &fanout_number = port_to_fanout_number.second;
        os << "set_port_fanout_number ";
        os << fanout_number.getFanoutNumber() << " ";
        os << ContainerDataPrint::pinIdToFullName(port_id);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcResistanceContainer &rhs) {
    const auto &data = rhs.getData();
    const auto &net_value = data->getNetResistance();
    for (const auto &net_to_resistance : net_value) {
        const ObjectId &net_id = net_to_resistance.first;
        const SetResistancePtr &resistance = net_to_resistance.second;
        os << "set_resistance ";
        os << resistance->getValue() << " ";
        const Flag &f1 = Flag("-min", resistance->getMin());
        const Flag &f2 = Flag("-max", resistance->getMax());
        os << ContainerDataPrint::getFlag(f1, f2);
        os << ContainerDataPrint::netIdToName(net_id);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcTimingDerateContainer &rhs) {
    const auto &data = rhs.getData();
    auto print_timing_derate = [](std::ostream &os, const SetTimingDeratePtr &derate) {
        os << "set_timing_derate ";
        os << derate->getDerateValue() << " ";
        const Flag &f1 = Flag("-min", derate->getMin());
        const Flag &f2 = Flag("-max", derate->getMax());
        const Flag &f3 = Flag("-rise", derate->getRise());
        const Flag &f4 = Flag("-fall", derate->getFall());
        const Flag &f6 = Flag("-late", derate->getLate());
        const Flag &f5 = Flag("-early", derate->getEarly());
        const Flag &f7 = Flag("-static", derate->getStaticType());
        const Flag &f8 = Flag("-dynamic", derate->getDynamicType());
        const Flag &f9 = Flag("-increment", derate->getIncrement()); //TODO If handle the increment in parsing, should ignore the option when write sdc
        const Flag &f11 = Flag("-data", derate->getData());
        const Flag &f10 = Flag("-clock", derate->getClock());
        const Flag &f12 = Flag("-net_delay", derate->getNetDelay());
        const Flag &f13 = Flag("-cell_delay", derate->getCellDelay());
        const Flag &f14 = Flag("-cell_check", derate->getCellCheck());
        os << ContainerDataPrint::getFlag(f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14);
    };
    const auto &net_value = data->getNetTimingDerate();
    for (const auto &net_to_timing_derate : net_value) {
        const ObjectId &net_id = net_to_timing_derate.first;
        const SetTimingDeratePtr &derate = net_to_timing_derate.second;
        print_timing_derate(os, derate);
        os << ContainerDataPrint::netIdToName(net_id);
        os << "\n";
    }
    const auto &inst_value = data->getInstTimingDerate();
    for (const auto &inst_to_timing_derate : inst_value) {
        const ObjectId &inst_id = inst_to_timing_derate.first;
        const SetTimingDeratePtr &derate = inst_to_timing_derate.second;
        print_timing_derate(os, derate);
        os << ContainerDataPrint::instIdToName(inst_id);
        os << "\n";
    }
    const auto &cell_value = data->getTCellTimingDerate();
    for (const auto &cell_to_timing_derate : cell_value) {
        const ObjectId &cell_id = cell_to_timing_derate.first;
        const SetTimingDeratePtr &derate = cell_to_timing_derate.second;
        print_timing_derate(os, derate);
        os << ContainerDataPrint::cellIdToName(cell_id);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcVoltageContainer &rhs) {
    const auto &data = rhs.getData();
    auto print_voltage = [](std::ostream &os, const SetVoltagePtr &voltage) {
        os << "set_voltage ";
        os << voltage->getMaxCaseVoltage() << " ";
        const FlagValue &v1 = FlagValue("-min", std::to_string(voltage->getMinCaseVoltage()));
        os << ContainerDataPrint::getFlagValue(v1);
    };
    const auto &net_value = data->getPowerNetVoltage();
    for (const auto &net_to_voltage : net_value) {
        const ObjectId &net_id = net_to_voltage.first;
        const SetVoltagePtr &voltage = net_to_voltage.second;
        print_voltage(os, voltage);
        const FlagValue &v2 = FlagValue("-object_list", ContainerDataPrint::netIdToName(net_id));
        os << ContainerDataPrint::getFlagValue(v2);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcWireLoadMinBlockSizeContainer &rhs) {
    const auto &data = rhs.getData();
    const auto &cell_min_block_size = data->getDesignMinBlockSize();
    for (const auto &cell_to_size : cell_min_block_size) {
        const auto &min_block_size = cell_to_size.second;
        os << "set_wire_load_min_block_size ";
        os << min_block_size.getBlockSize();
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcWireLoadModeContainer &rhs) {
    const auto &data = rhs.getData();
    const auto &cell_wire_load_mode = data->getDesignWireLoadMode();
    for (const auto &cell_to_mode : cell_wire_load_mode) {
        const auto &wire_load_mode = cell_to_mode.second;
        os << "set_wire_load_mode ";
        os << toString(wire_load_mode.getMode());
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcWireLoadModelContainer &rhs) {
    const auto &data = rhs.getData();
    auto print_wire_load_model = [](std::ostream &os, const SetWireLoadModelPtr &model) {
        auto get_lib_from_wire_load = [](const ObjectId &wire_load_id) -> ObjectId {
            const auto &wire_load = Object::addr<WireLoad>(wire_load_id);
            if (!wire_load) {
                return UNINIT_OBJECT_ID;
            }
            return wire_load->getGroupId();
        };
        const ObjectId &wire_load_id = model->getWireLoad();
        os << "set_wire_load_model ";
        const FlagValue &v1 = FlagValue("-name", ContainerDataPrint::idToName<WireLoad>(wire_load_id));
        const FlagValue &v2 = FlagValue("-library", ContainerDataPrint::libIdToName(get_lib_from_wire_load(wire_load_id)));
        os << ContainerDataPrint::getFlagValue(v1, v2);
        const Flag &f1 = Flag("-min", model->getMin());
        const Flag &f2 = Flag("-max", model->getMax());
        os << ContainerDataPrint::getFlag(f1, f2);
    };
    const auto &pin_value = data->getPinWireLoadModel();
    for (const auto &pin_to_wire_load_model : pin_value) {
        const ObjectId &pin_id = pin_to_wire_load_model.first;
        const SetWireLoadModelPtr &model = pin_to_wire_load_model.second;
        print_wire_load_model(os, model);
        os << ContainerDataPrint::pinIdToFullName(pin_id);
        os << "\n";
    }
    const auto &cell_value = data->getCellWireLoadModel();
    for (const auto &cell_to_wire_load_model : cell_value) {
        const auto  &cell_id = cell_to_wire_load_model.first;
        const auto  &model = cell_to_wire_load_model.second;
        print_wire_load_model(os, model);
        os << ContainerDataPrint::cellIdToName(cell_id);
        os << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcWireLoadSelectionGroupContainer &rhs) {
    const auto &data = rhs.getData();
    auto print_selection = [](std::ostream &os, const SetWireLoadSelectionGroupPtr &selection) {
        auto get_lib_from_wire_load_selection = [](const ObjectId &wire_load_selection_id) -> ObjectId {
            const auto &wire_load_selection = Object::addr<WireLoadSelection>(wire_load_selection_id);
            if (!wire_load_selection) {
                return UNINIT_OBJECT_ID;
            }
            return wire_load_selection->getGroupId();
        };
        const ObjectId &wire_load_selection_id = selection->getSelection();
        os << "set_wire_load_selection_group ";
        os << ContainerDataPrint::idToName<WireLoadSelection>(wire_load_selection_id) << " ";
        const FlagValue &v1 = FlagValue("-library", ContainerDataPrint::libIdToName(get_lib_from_wire_load_selection(wire_load_selection_id)));
        os << ContainerDataPrint::getFlagValue(v1);
        const Flag &f1 = Flag("-min", selection->getMin());
        const Flag &f2 = Flag("-max", selection->getMax());
        os << ContainerDataPrint::getFlag(f1, f2);
    };
    const auto &cell_value = data->getCellWireSelectionGroup();
    for (const auto &cell_to_selection : cell_value) {
        const ObjectId &cell_id = cell_to_selection.first;
        const SetWireLoadSelectionGroupPtr &selection = cell_to_selection.second;
        print_selection(os, selection);
        os << ContainerDataPrint::cellIdToName(cell_id) << " ";
        os << "\n";
    }
    const auto &inst_value = data->getInstWireSelectionGroup();
    for (const auto &inst_to_selection : inst_value) {
        const ObjectId &inst_id = inst_to_selection.first;
        const SetWireLoadSelectionGroupPtr &selection = inst_to_selection.second;
        print_selection(os, selection);
        os << ContainerDataPrint::instIdToName(inst_id) << " ";
        os << "\n";
    }
    return os;
}

//multivoltage power commands
const CreateVoltageAreaPtr SdcVoltageAreaContainer::getInstVoltageArea(const ObjectId &inst_id) const {
   const auto &inst_voltage_area = data_->getInstVoltageArea();
   const auto &found = inst_voltage_area.find(inst_id);
   if (found == inst_voltage_area.end()) {
       // messages;
       return nullptr;
   }
   return found->second;
}

std::ostream &operator<<(std::ostream &os, SdcVoltageAreaContainer &rhs) {
    const auto &inst_value = rhs.data_->getInstVoltageArea();
    for (const auto &inst_to_area : inst_value) {
        const ObjectId &inst_id = inst_to_area.first;
        const CreateVoltageAreaPtr &voltage_area = inst_to_area.second;
        const auto &coordinates = voltage_area->getCoordinates();
        os  << "create_voltage_area ";
        os  << "-corrdinate {";
        for (const auto &box : coordinates) {
            os << box.getLLX() << " " << box.getLLY() << " " << box.getURX() << " " << box.getURY() << " ";
        }
        os << "} ";
        const FlagValue &v1 = FlagValue("-name", voltage_area->getName());
        const FlagValue &v2 = FlagValue("guard_band_x", std::to_string(voltage_area->getGuardBandX()));
        const FlagValue &v3 = FlagValue("guard_band_y", std::to_string(voltage_area->getGuardBandY()));
        os << ContainerDataPrint::getFlagValue(v1, v2, v3);
        os << ContainerDataPrint::instIdToName(inst_id);
        os  << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcLevelShifterStrategyContainer &rhs) {
    os << "set_level_shifter_strategy ";
    const FlagValue &v1 = FlagValue("-rule", toString(rhs.getLevelShifterStrategy()));
    os << ContainerDataPrint::getFlagValue(v1);
    os << "\n";
    return os;
}

std::ostream &operator<<(std::ostream &os, SdcLevelShifterThresholdContainer &rhs) {
    os << "set_level_shifter_threshold ";
    const FlagValue &v1 = FlagValue("-voltage", std::to_string(rhs.getVoltage()));
    const FlagValue &v2 = FlagValue("-percent", std::to_string(rhs.getPercent()));
    os << ContainerDataPrint::getFlagValue(v1, v2);
    os << "\n";
    return os;
}

const float SdcMaxDynamicPowerContainer::getCellPower(const ObjectId &cell_id) const {
    const auto &cell_power = data_->getDynamicPower();
    const auto &found = cell_power.find(cell_id);
    if (found == cell_power.end()) {
        // message
        return 0.0;
    }
    const auto &power = found->second;
    return power.getPowerValue();
}

std::ostream &operator<<(std::ostream &os, SdcMaxDynamicPowerContainer &rhs) {
    const auto &cell_power_map = rhs.data_->getDynamicPower();
    for (const auto &cell_to_power : cell_power_map) {
        //TODO handling the current_design
        const auto &cell_id = cell_to_power.first;
        const auto &power_value = cell_to_power.second;
        os << "set_max_dynamic_power ";
        os << power_value.getPowerValue() << " ";
        const FlagValue &v1 = FlagValue("-unit", "W");
        os << ContainerDataPrint::getFlagValue(v1);
        os << "\n";
    }
    return os;
}

const float SdcMaxLeakagePowerContainer::getCellPower(const ObjectId &cell_id) const {
    if (!data_) {
        // error message
        return 0.0;
    }
    const auto &cell_power = data_->getLeakagePower();
    const auto &found = cell_power.find(cell_id);
    if (found == cell_power.end()) {
        // message
        return 0.0;
    }
    const auto &power = found->second;
    return power.getPowerValue();
}

std::ostream &operator<<(std::ostream &os, SdcMaxLeakagePowerContainer &rhs) {
    const auto &cell_power_map = rhs.data_->getLeakagePower();
    for (const auto &cell_to_power : cell_power_map) {
        //TODO handling the current_design
        const ObjectId &cell_id = cell_to_power.first;
        const SetMaxLeakagePower &power_value = cell_to_power.second;
        os << "set_max_leakage_power ";
        os << power_value.getPowerValue() << " ";
        const FlagValue &v1 = FlagValue("-unit", "W");
        os << ContainerDataPrint::getFlagValue(v1);
        os << "\n";
    }
    return os;
}

//object access commands
const std::string SdcCurrentDesignContainer::getDesignName() const {
    const ObjectId &cell_id = data_->getCellId();
    Cell* cell = Object::addr<Cell>(cell_id);
    if (!cell) {
        return "";
    }
    return cell->getName();
}

}
}
