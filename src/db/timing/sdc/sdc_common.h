/**
 * @file sdc_common.h
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

#ifndef EDI_DB_TIMING_SDC_SDC_COMMON_H_
#define EDI_DB_TIMING_SDC_SDC_COMMON_H_

#include <boost/functional/hash.hpp>
#include <unordered_set>
#include <unordered_map>

#include "db/core/object.h"
#include "db/core/db.h"
#include "util/data_traits.h"
#include "db/timing/sdc/clock.h"
#include "db/timing/timinglib/timinglib_cell.h"

namespace open_edi {
namespace db {

class TUnits;
class TCell;
class TLib;
class TimingArc;

template<typename T, typename U>
class UnorderedPair {
  public:
    UnorderedPair() {}
    UnorderedPair(T first_value, U second_value) : first(first_value), second(second_value) {}
    bool operator==(const UnorderedPair<T, U> &rhs) const;
    template<T, U>
    friend std::size_t hash_value(const UnorderedPair<T, U> &p);

    //keep the same usage as std::pair
    T first;
    U second;
};

template<typename T, typename U>
bool UnorderedPair<T,U>::operator==(const UnorderedPair<T, U> &rhs) const {
    if ((this->first == rhs.first) and (this->second == rhs.second)) {
        return true;
    }
    if ((this->first == rhs.second) and (this->second == rhs.first )) {
        return true;
    }
    return false;
}

template<typename T, typename U>
std::size_t hash_value(const UnorderedPair<T, U> &p) {
    std::size_t seed1 = 0;
    boost::hash_combine(seed1, p.first);
    std::size_t seed2 = 0;
    boost::hash_combine(seed2, p.second);
    std::size_t seed = (seed1 xor seed2) + p.first + p.second;
    return seed;
}

template <typename T>
bool ediEqual(const T &value1, const T &value2, T eps = std::numeric_limits<T>::epsilon()) {
    return fabs(value1 - value2) < eps;
}

TLib* getMainLiberty();
TUnits* getMainLibertyUnit();

Pin* getPinByFullName(const std::string &full_name);

bool isCellInOtherCell(const ObjectId &cell_id, const ObjectId &other_cell_id);
bool isInstInCell(const ObjectId &inst_id, const ObjectId &cell_id);

TCell* getLibCellInCorner(AnalysisCorner *analysis_corner, const std::string &cell_name, std::string liberty_name = "");

void getSdcFileContents(std::string &contents, const std::vector<std::string> &files,  const std::string &view_name);

class ContainerDataAccess {
  public:
    template<typename V>
    static bool isEqual(const V &v1, const V &v2) { return v1 == v2; }

    template<typename V>
    static bool isEqual(const std::shared_ptr<V> &v1, const std::shared_ptr<V> &v2) { return (*v1 == *v2); }

    template<typename V>
    static bool leftHasLowerPriority(const V &lhs, const V &rhs) { return true; }

    template<typename V>
    static bool rightHasHigherPriority(const V &lhs, const V &rhs) { return false; }

    template<typename Key, typename Value, typename Hash = std::hash<Key>, 
            typename KeyEqual = std::equal_to<Key>, 
            typename Allocator = std::allocator< std::pair<const Key, Value> > >
    static bool updateMapValue(std::unordered_map<Key, Value, Hash, KeyEqual, Allocator> &map, const Key &key, const Value &value);

    template<typename Key, typename Value, typename Hash = std::hash<Key>, 
            typename KeyEqual = std::equal_to<Key>, 
            typename Allocator = std::allocator< std::pair<const Key, Value> > >
    static bool updateMapValue(std::unordered_multimap<Key, Value, Hash, KeyEqual, Allocator> &multi_map, const Key &key, const Value &value);

    static bool setToPin(ObjectId &pin_id, const std::string &pin_name);

    template<typename U, typename V>
    static bool addToPinValueMap(U &pin_map, const std::string &pin_name, const V &value);

    template<typename U, typename V>
    static bool addToPortValueMap(U &port_map, const std::string &port_name, const V &value);

    template<typename U, typename V>
    static bool addToInstValueMap(U &inst_map, const std::string &inst_name, const V &value);

    template<typename U, typename V>
    static bool addToCellValueMap(U &cell_map, const std::string &cell_name, const V &value);

    template<typename U, typename V>
    static bool addToTCellValueMap(U &cell_map, AnalysisCorner *corner, const std::string &cell_name, const V &value);

    template<typename U, typename V>
    static bool addToNetValueMap(U &net_map, const std::string &net_name, const V &value);

    template<typename U, typename V>
    static bool addToPowerNetValueMap(U &net_map, const std::string &net_name, const V &value);

    template<typename U, typename V>
    static bool addToClockValueMap(U &clock_map, const ClockId &clock_id, const V &value);

    template<typename U, typename V>
    static bool addToCurrentDesignValueMap(U &design_map, const ObjectId &cell_id, const V &value);

    template<typename U>
    static bool addToPinSet(U &pin_set, const std::string &pin_name);

    template<typename U>
    static bool addToPortSet(U &port_set, const std::string &port_name);

    template<typename U>
    static bool addToInstSet(U &inst_set, const std::string &inst_name);

    template<typename U>
    static bool addToCellSet(U &cell_set, const std::string &cell_name);

    template<typename U>
    static bool addToNetSet(U &net_set, const std::string &net_name);

    template<typename U>
    static bool addToClockSet(U &clock_set, const ClockId &clock_id);

    template<typename U>
    static bool addToCurrentDesignSet(U &design_set, const ObjectId &cell_id);

    template<typename U, typename V>
    static void forEachObjectMap(U &map_like, const std::function<void(const ObjectId &, const V &)> &func);

    template<typename U>
    static void forEachObjectSet(U &set_like, const std::function<void(const ObjectId &)> &func);

};

template<typename Key, typename Value, typename Hash, typename KeyEqual, typename Allocator>
bool ContainerDataAccess::updateMapValue(std::unordered_map<Key, Value, Hash, KeyEqual, Allocator> &map, const Key &key, const Value &value) {
    auto found = map.find(key);
    if (found == map.end()) {
        map.emplace(key, value);
        return true;
    }
    if (leftHasLowerPriority(found->second, value)) {
        found->second = value;
        return true;
    }
    return false;
}

template<typename Key, typename Value, typename Hash, typename KeyEqual, typename Allocator>
bool ContainerDataAccess::updateMapValue(std::unordered_multimap<Key, Value, Hash, KeyEqual, Allocator> &multi_map, const Key &key, const Value &value) {
    auto range = multi_map.equal_range(key);
    if (range.first == multi_map.end()) {
        multi_map.emplace(key, value);
        return true;
    }
    std::vector<decltype(range.first)> need_removed;
    for (auto it = range.first; it != range.second; ++it) {
        auto &exist_value = it->second;
        if (isEqual(exist_value, value)) {
            exist_value = value;
            return true;
        }
        if (rightHasHigherPriority(exist_value, value)) {
            need_removed.emplace_back(it);
        }
    }
    for (const auto &it : need_removed) {
        multi_map.erase(it);
    }
    multi_map.emplace(key, value);
    return true; //Always return true
}

template<typename U, typename V>
bool ContainerDataAccess::addToPinValueMap(U &pin_map, const std::string &pin_name, const V &value) {
    const auto &pin = getPinByFullName(pin_name);
    if (!pin) {
        return false;
    }
    const auto &pin_id = pin->getId();
    return updateMapValue(pin_map, pin_id, value);
}

template<typename U, typename V>
bool ContainerDataAccess::addToPortValueMap(U &port_map, const std::string &port_name, const V &value) {
    const auto &top_cell = getTopCell();
    const auto &port = top_cell->getIOPin(port_name);
    if (!port) {
        return false;
    }
    const auto &port_id = port->getId();
    return updateMapValue(port_map, port_id, value);
}

template<typename U, typename V>
bool ContainerDataAccess::addToInstValueMap(U &inst_map, const std::string &inst_name, const V &value) {
    const auto &top_cell = getTopCell();
    const auto &inst = top_cell->getInstance(inst_name);
    if (!inst) {
        return false;
    }
    const auto &inst_id = inst->getId();
    return updateMapValue(inst_map, inst_id, value);
}

template<typename U, typename V>
bool ContainerDataAccess::addToCellValueMap(U &cell_map, const std::string &cell_name, const V &value) {
    const auto &top_cell = getTopCell();
    const auto &cell = top_cell->getCell(cell_name);
    if (!cell) {
        return false;
    }
    const auto &cell_id = cell->getId();
    return updateMapValue(cell_map, cell_id, value);
}

template<typename U, typename V>
bool ContainerDataAccess::addToTCellValueMap(U &cell_map, AnalysisCorner *corner, const std::string &cell_name, const V &value) {
    TCell* tcell = getLibCellInCorner(corner, cell_name);
    if (!tcell) {
        return false;
    }
    const auto &cell_id = tcell->getId();
    return updateMapValue(cell_map, cell_id, value);
}

template<typename U, typename V>
bool ContainerDataAccess::addToNetValueMap(U &net_map, const std::string &net_name, const V &value) {
    const auto &top_cell = getTopCell();
    const auto &net = top_cell->getNet(net_name);
    if (!net) {
        return false;
    }
    const auto &net_id = net->getId();
    return updateMapValue(net_map, net_id, value);
}

template<typename U, typename V>
bool ContainerDataAccess::addToPowerNetValueMap(U &net_map, const std::string &net_name, const V &value) {
    const auto &top_cell = getTopCell();
    const auto &net = top_cell->getSpecialNet(net_name);
    if (!net) {
        return false;
    }
    const auto &net_id = net->getId();
    return updateMapValue(net_map, net_id, value);
}

template<typename U, typename V>
bool ContainerDataAccess::addToClockValueMap(U &clock_map, const ClockId &clock_id, const V &value) {
    return updateMapValue(clock_map, clock_id, value);
}

template<typename U, typename V>
bool ContainerDataAccess::addToCurrentDesignValueMap(U &design_map, const ObjectId &cell_id, const V &value) {
    return updateMapValue(design_map, cell_id, value);
}

template<typename U>
bool ContainerDataAccess::addToPinSet(U &pin_set, const std::string &pin_name) {
    const auto &pin = getPinByFullName(pin_name);
    if (!pin) {
        return false;
    }
    pin_set.emplace(pin->getId());
    return true;
}

template<typename U>
bool ContainerDataAccess::addToPortSet(U &port_set, const std::string &port_name) {
    const auto &top_cell = getTopCell();
    const auto &port = top_cell->getIOPin(port_name);
    if (!port) {
        return false;
    }
    port_set.emplace(port->getId());
    return true;
}

template<typename U>
bool ContainerDataAccess::addToInstSet(U &inst_set, const std::string &inst_name) {
    const auto &top_cell = getTopCell();
    const auto &inst = top_cell->getInstance(inst_name);
    if (!inst) {
        return false;
    }
    inst_set.emplace(inst->getId());
    return true;
}

template<typename U>
bool ContainerDataAccess::addToCellSet(U &cell_set, const std::string &cell_name) {
    const auto &top_cell = getTopCell();
    const auto &cell = top_cell->getCell(cell_name);
    if (!cell) {
        return false;
    }
    cell_set.emplace(cell->getId());
    return true;
}

template<typename U>
bool ContainerDataAccess::addToNetSet(U &net_set, const std::string &net_name) {
    const auto &top_cell = getTopCell();
    const auto &net = top_cell->getNet(net_name);
    if (!net) {
        return false;
    }
    net_set.emplace(net->getId());
    return true;
}

template<typename U>
bool ContainerDataAccess::addToClockSet(U &clock_set, const ClockId &clock_id) {
    clock_set.emplace(clock_id);
    return true;
}

template<typename U>
bool ContainerDataAccess::addToCurrentDesignSet(U &design_set, const ObjectId &cell_id) {
    design_set.emplace(cell_id);
    return true;
}

template<typename U, typename V>
void ContainerDataAccess::forEachObjectMap(U &map_like, const std::function<void(const ObjectId &, const V &)> &func) {
    for (const auto &object_value : map_like) {
        const ObjectId &object_id = object_value.first;
        const V &value = object_value.second;
        func(object_id, value);
    }
}

template<typename U>
void ContainerDataAccess::forEachObjectSet(U &set_like, const std::function<void(const ObjectId &)> &func) {
    for (const auto &object_id : set_like) {
        func(object_id);
    }
}

using Flag = std::pair<std::string, bool>;
using FlagValue = std::pair<std::string, std::string>;

class ContainerDataPrint {
  public:
    static std::string addDoubleQuotes(const std::string &str);

    template<typename U>
    static std::string dataListToStr(U &data_list);

    template<typename U>
    static std::string idToName(const ObjectId &id);

    template<typename F>
    static std::string clockIdToName(F func, const ClockId &clock_id);

    static std::string pinIdToName(const ObjectId &pin_id);
    static std::string termIdToName(const ObjectId &term_id);
    static std::string cellIdToName(const ObjectId &cell_id);
    static std::string ttermIdToName(const ObjectId &tterm_id);
    static std::string tcellIdToName(const ObjectId &tcell_id);
    static std::string instIdToName(const ObjectId &inst_id);
    static std::string getPinOrPortName(bool &is_port, const ObjectId &pin_id);
    static std::string pinIdToFullName(const ObjectId &pin_id);
    static std::string ttermIdToFullName(const ObjectId &tterm_id);
    static std::string getClockPrintName(const std::string &clock_name);
    static std::string netIdToName(const ObjectId &net_id);
    static std::string libIdToName(const ObjectId &lib_id);

    template<typename U, typename V>
    static std::string idsToNameList(const U &ids);

    template<typename U>
    static std::string pinIdsToNameList(const U &pin_ids);

    template<typename U>
    static std::string termIdsToNameList(const U &term_ids);

    template<typename U>
    static std::string cellIdsToNameList(const U &cell_ids);

    template<typename U>
    static std::string ttermIdsToNameList(const U &tterm_ids);

    template<typename U>
    static std::string tcellIdsToNameList(const U &tcell_ids);

    template<typename U>
    static std::string instIdsToNameList(const U &inst_ids);

    template<typename U>
    static std::string pinIdsToFullNameList(const U &pin_ids);

    template<typename U>
    static std::string ttermIdsToFullNameList(const U &tterm_ids);

    template<typename F, typename U>
    static std::string clockIdsToNameList(F func, const U &clock_ids);

    template<typename U>
    static std::string netIdsToNameList(const U &net_ids);

    template<typename U>
    static std::string libIdsToNameList(const U &lib_ids);

    static std::string getFlag(const Flag &p);

    template<typename ... Types>
    static std::string getFlag(const Flag &first, Types ... rest);

    static std::string getFlagValue(const FlagValue &p);

    template<typename ... Types>
    static std::string getFlagValue(const FlagValue &first, Types ... rest);
};

template<typename U>
std::string ContainerDataPrint::dataListToStr(U &data_list) {
    if (data_list.empty()) {
        return "";
    }
    std::string str;
    for (const auto & data : data_list) {
        str.append(" ").append(std::to_string(data));
    }
    return "{" + str + " }";
}

template<typename U>
std::string ContainerDataPrint::idToName(const ObjectId &id) {
    const auto &object = Object::addr<U>(id);
    if (!object) {
        return "";
    }
    return object->getName();
}

template<typename F>
std::string ContainerDataPrint::clockIdToName(F func, const ClockId &clock_id) {
    const std::string &clock_name = func(clock_id);
    return getClockPrintName(clock_name);
}

template<typename U, typename V>
std::string ContainerDataPrint::idsToNameList(const U &ids) {
    std::string ret = "";
    for (const ObjectId &id: ids) {
        ret += idToName<V>(id) + " ";
    }
    return ret;
}

template<typename U>
std::string ContainerDataPrint::pinIdsToNameList(const U &pin_ids) {
    return idsToNameList<U, Pin>(pin_ids);
}

template<typename U>
std::string ContainerDataPrint::termIdsToNameList(const U &term_ids) {
    return idsToNameList<U, Term>(term_ids);
}

template<typename U>
std::string ContainerDataPrint::cellIdsToNameList(const U &cell_ids) {
    const std::string &cell_names = idsToNameList<U, Cell>(cell_ids);
    if (cell_names.empty()) {
        return "";
    }
    return "[get_designs { " + cell_names + " }]";
}

template<typename U>
std::string ContainerDataPrint::ttermIdsToNameList(const U &tterm_ids) {
    return idsToNameList<U, TTerm>(tterm_ids);
}

template<typename U>
std::string ContainerDataPrint::tcellIdsToNameList(const U &tcell_ids) {
    const std::string &tcell_names = idsToNameList<U, TCell>(tcell_ids);
    if (tcell_names.empty()) {
        return "";
    }
    return "[get_lib_cells { " + tcell_names + " }]";
}

template<typename U>
std::string ContainerDataPrint::instIdsToNameList(const U &inst_ids) {
    const std::string &inst_names = idsToNameList<U, Inst>(inst_ids);
    if (inst_names.empty()) {
        return "";
    }
    return "[get_cells { " + inst_names + " }]";
}

template<typename U>
std::string ContainerDataPrint::pinIdsToFullNameList(const U &pin_ids) {
    if (pin_ids.empty()) {
        return "";
    }
    std::string ret = "";
    bool is_port = false;
    for (const ObjectId &pin_id : pin_ids) {
        const std::string &pin_name = getPinOrPortName(is_port, pin_id);
        if (pin_name.empty()) {
            continue;
        }
        ret += pin_name + " ";
    }
    if (ret.empty()) {
        return "";
    }
    if (is_port) {
        ret = "[get_ports { " + ret + " }]";
    } else {
        ret = "[get_pins { " + ret + " }]";
    }

    return ret;
}

template<typename U>
std::string ContainerDataPrint::ttermIdsToFullNameList(const U &tterm_ids) {
    if (tterm_ids.empty()) {
        return "";
    }
    std::string ret = "";
    for (const ObjectId &tterm_id : tterm_ids) {
        ret += ttermIdToFullName(tterm_id) + " ";
    }
    return "[get_lib_pins { " + ret + " }]";
}

template<typename F, typename U>
std::string ContainerDataPrint::clockIdsToNameList(F func, const U &clock_ids) {
    if (clock_ids.empty()) {
        return "";
    }
    std::string ret = "";
    for (const ClockId &clock_id : clock_ids) {
        ret += func(clock_id) + " ";
    }
    ret = "[get_clocks { " + ret + " }]";
    return ret;
}

template<typename U>
std::string ContainerDataPrint::netIdsToNameList(const U &net_ids) {
    const std::string &net_names = idsToNameList<U, Net>(net_ids);
    if (net_names.empty()) {
        return "";
    }
    return "[get_nets { " + net_names + " }]";
}

template<typename U>
std::string ContainerDataPrint::libIdsToNameList(const U &lib_ids) {
    const std::string &lib_names = idsToNameList<U, TLib>(lib_ids);
    if (lib_names.empty()) {
        return "";
    }
    return "[get_nets { " + lib_names + " }]";
}

template<typename ... Types>
std::string ContainerDataPrint::getFlag(const Flag &first, Types ... rest) {
    return getFlag(first) + getFlag(rest...);
}

template<typename ... Types>
std::string ContainerDataPrint::getFlagValue(const FlagValue &first, Types ... rest) {
    return getFlagValue(first) + getFlagValue(rest...);
}

}
}

#endif //EDI_DB_TIMING_SDC_SDC_COMMON_H_
