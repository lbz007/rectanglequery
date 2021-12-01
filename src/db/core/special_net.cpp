/* @file  net.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#include "db/core/special_net.h"

#include "db/core/cell.h"
#include "db/core/db.h"
#include "db/core/pin.h"
#include "db/util/array.h"

namespace open_edi {
namespace db {
using IdArray = ArrayObject<ObjectId>;

/**
 * @brief Construct a new Special SpecialNet object
 *
 */
SpecialNet::SpecialNet() {
    name_index_ = 0;
    fix_bump_ = 0;
    net_type_ = 0;
    status_ = 0;
    pattern_ = 0;
    rule_ = 0;
    voltage_ = 0;
    frequency_ = 0;
    cap_ = 0;
    weight_ = 0;
    origin_net_ = 0;
}

Cell* SpecialNet::getCell() {
    ObjectId cell_id = getOwnerId();
    if (cell_id) {
        return addr<Cell>(cell_id);
    }
    return nullptr;
}

/**
 * @brief Get the Name Index object
 *
 * @return SymbolIndex
 */
SymbolIndex SpecialNet::getNameIndex() const { return name_index_; }

/**
 * @brief Get the Name object
 *
 * @return std::string const&
 */
std::string const& SpecialNet::getName() {
    StorageUtil *storage_util = getStorageUtil();
    ediAssert(storage_util != nullptr);
    SymbolTable *symtbl = storage_util->getSymbolTable();
    ediAssert(symtbl != nullptr);

    return symtbl->getSymbolByIndex(name_index_);
}

/**
 * @brief Get the Fix Bump object
 *
 * @return Bits
 */
Bits SpecialNet::getFixBump() const { return fix_bump_; }

/**
 * @brief Get the Voltage object
 *
 * @return int
 */
int SpecialNet::getVoltage() const { return voltage_; }

/**
 * @brief Get the Pattern object
 *
 * @return Bits
 */
Bits SpecialNet::getPattern() const { return pattern_; }
/**
 * @brief Get the Source object
 *
 * @return Bits
 */
SourceType SpecialNet::getSource() const { return source_; }

/**
 * @brief Get the Frequency object
 *
 * @return int
 */
int SpecialNet::getFrequency() const { return frequency_; }

/**
 * @brief Get the Capacitance object
 *
 * @return double
 */
double SpecialNet::getCapacitance() const { return cap_; }

/**
 * @brief Get the Weight object
 *
 * @return double
 */
double SpecialNet::getWeight() const { return weight_; }

/**
 * @brief is analog net
 *
 * @return true
 * @return false
 */
bool SpecialNet::isAnalog() { return getType() == kSpecialNetTypeAnalog; }

/**
 * @brief is clock net
 *
 * @return true
 * @return false
 */
bool SpecialNet::isClock() { return getType() == kSpecialNetTypeClock; }

/**
 * @brief is ground net
 *
 * @return true
 * @return false
 */
bool SpecialNet::isGround() { return getType() == kSpecialNetTypeGround; }

/**
 * @brief is power net
 *
 * @return true
 * @return false
 */
bool SpecialNet::isPower() { return getType() == kSpecialNetTypePower; }

/**
 * @brief is reset net
 *
 * @return true
 * @return false
 */
bool SpecialNet::isReset() { return getType() == kSpecialNetTypeReset; }

/**
 * @brief is scan net
 *
 * @return true
 * @return false
 */
bool SpecialNet::isScan() { return getType() == kSpecialNetTypeScan; }

/**
 * @brief is signal net
 *
 * @return true
 * @return false
 */
bool SpecialNet::isSignal() { return getType() == kSpecialNetTypeSignal; }

/**
 * @brief is tie off net
 *
 * @return true
 * @return false
 */
bool SpecialNet::isTieOff() { return getType() == kSpecialNetTypeTieOff; }

/**
 * @brief Set the Fix Bump object
 *
 * @param fix_bump
 */
void SpecialNet::setFixBump(Bits fix_bump) { fix_bump_ = fix_bump; }

/**
 * @brief Set the Pattern object
 *
 * @param pattern
 */
void SpecialNet::setPattern(Bits pattern) { pattern_ = pattern; }

/**
 * @brief Set the Source object
 *
 * @param source
 */
void SpecialNet::setSource(SourceType source) { source_ = source; }

/**
 * @brief Set the Voltage object
 *
 * @param voltage
 */
void SpecialNet::setVoltage(int voltage) { voltage_ = voltage; }

/**
 * @brief Set the Frequency object
 *
 * @param frequency
 */
void SpecialNet::setFrequency(int frequency) { frequency_ = frequency; }

/**
 * @brief Set the Capacitance object
 *
 * @param cap
 */
void SpecialNet::setCapacitance(double cap) { cap_ = cap; }

/**
 * @brief Set the Weight object
 *
 * @param weight
 */
void SpecialNet::setWeight(double weight) { weight_ = weight; }

/**
 * @brief Set the Name object
 *
 * @param name
 */
bool SpecialNet::setName(std::string const& name) {
    StorageUtil *storage_util = getStorageUtil();
    ediAssert(storage_util != nullptr);
    SymbolTable *symtbl = storage_util->getSymbolTable();
    ediAssert(symtbl != nullptr);

    int64_t index = symtbl->getOrCreateSymbol(name.c_str());
    if (index == kInvalidSymbolIndex) return false;

    name_index_ = index;
    symtbl->addReference(name_index_, this->getId());
    return true;
}

/**
 * @brief Get the Type object
 *
 * @return Bits
 */
SpecialNetType SpecialNet::getType() const {
    return static_cast<SpecialNetType>(net_type_);
}

/**
 * @brief Set the Type object
 *
 * @param type
 */
void SpecialNet::setType(SpecialNetType net_type) { net_type_ = net_type; }

/**
 * @brief Get the Rule object
 *
 * @return ObjectIndex
 */
const char* SpecialNet::getNonDefaultRule() const {
    NonDefaultRule* rule = addr<NonDefaultRule>(rule_);
    return rule->getName();
}

/**
 * @brief Set the Rule object
 *
 * @param rule
 */
void SpecialNet::setNonDefaultRule(ObjectId rule) { rule_ = rule; }

/**
 * @brief add wire to the net's wirelist
 *
 * @param wire
 */
void SpecialNet::addWire(Wire* wire) {
    ArrayObject<ObjectId>* wire_vector = nullptr;
    if (wires_ == 0) {
        wires_ = __createObjectIdArray(32);
    }
    if (wires_) wire_vector = addr<ArrayObject<ObjectId>>(wires_);
    if (wire_vector) wire_vector->pushBack(wire->getId());
}

void SpecialNet::setOriginNet(SpecialNet* net) {
    if (net) origin_net_ = net->getId();
}

const char* SpecialNet::getOriginNet() {
    SpecialNet* net = nullptr;
    if (origin_net_) {
        net = addr<SpecialNet>(origin_net_);
    }
    if (net) {
        return net->getName().c_str();
    } else {
        return nullptr;
    }
}

/**
 * @brief create a wire in net
 *
 * @return Wire*
 */
Wire* SpecialNet::createWire(int x1, int y1, int x2, int y2, int width) {
    Wire* wire = Object::createObject<Wire>(kObjectTypeWire, getOwnerId());
    int x = 0, y = 0, length = 0, height = 0;

    // the wire is horizontal
    if (y1 == y2) {
        if (x1 > x2) std::swap(x1, x2);  // keep the x1 in left

        x = x1;
        y = y1 - width / 2;
        length = x2 - x1;
        height = width;
    }
    // the wire is vertical
    if (x1 == x2) {
        if (y1 > y2) std::swap(y1, y2);  // keep the y1 in down
        x = x1 - width / 2;
        y = y1;
        length = width;
        height = y2 - y1;
    }

    wire->setX(x);
    wire->setY(y);
    wire->setHeight(height);
    wire->setLength(length);
    wire->setNet(this);

    return wire;
}

Wire* SpecialNet::createWire(int x1, int y1, int x2, int y2) {
    Wire* wire = Object::createObject<Wire>(kObjectTypeWire, getOwnerId());
    wire->setX(x1);
    wire->setY(y1);
    wire->setHeight(y2 - y1);
    wire->setLength(x2 - x1);
    return wire;
}

/**
 * @brief Get the Pin Array object
 *
 * @return ArrayObject<ObjectId>*
 */
ArrayObject<ObjectId>* SpecialNet::getPinArray() const {
    if (pins_ != 0) {
        ArrayObject<ObjectId>* pin_array = addr<ArrayObject<ObjectId>>(pins_);
        return pin_array;
    } else {
        return nullptr;
    }
}

/**
 * @brief Get the Wire Array object
 *
 * @return ArrayObject<ObjectId>*
 */
ArrayObject<ObjectId>* SpecialNet::getWireArray() const {
    if (wires_ != 0) {
        ArrayObject<ObjectId>* wire_array = addr<ArrayObject<ObjectId>>(wires_);
        return wire_array;
    } else {
        return nullptr;
    }
}

/**
 * @brief Get the Via Array object
 *
 * @return ArrayObject<ObjectId>*
 */
ArrayObject<ObjectId>* SpecialNet::getViaArray() const {
    if (vias_ != 0) {
        ArrayObject<ObjectId>* via_array = addr<ArrayObject<ObjectId>>(vias_);
        return via_array;
    } else {
        return nullptr;
    }
}

/**
 * @brief add pin to not
 *
 * @param pin
 * @return int
 */
int SpecialNet::addPin(Pin* pin) {
    IdArray* pin_vector = nullptr;
    if (pins_ == 0) {
        pins_ = __createObjectIdArray(64);
    }
    if (pins_) pin_vector = addr<IdArray>(pins_);
    if (pin) pin_vector->pushBack(pin->getId());

    return 0;
}

Via* SpecialNet::createVia(int x, int y, ViaMaster* via_master) {
    Via* via = Object::createObject<Via>(kObjectTypeVia, getOwnerId());
    if (via) via->setLoc(x, y);
    return via;
}

/**
 * @brief  add via to the net's vialist
 *
 * @param via
 */
void SpecialNet::addVia(Via* via) {
    ArrayObject<ObjectId>* via_vector = nullptr;
    if (vias_ == 0) {
        vias_ = __createObjectIdArray(16);
    }
    if (vias_) via_vector = addr<ArrayObject<ObjectId>>(vias_);
    if (via_vector) via_vector->pushBack(via->getId());
}

/**
 * @brief print function for SpecialNet
 *
 */
void SpecialNet::printDEF(IOManager &io_manager) {
    io_manager.write(" - %s \n", getName().c_str());
    // pin
    if (pins_) {
        IdArray* pin_vector = addr<IdArray>(pins_);
        for (IdArray::iterator iter = pin_vector->begin();
             iter != pin_vector->end(); ++iter) {
            Pin* pin = nullptr;
            ObjectId id = (*iter);
            if (id) pin = addr<Pin>(id);
            if (pin) {
                Inst* inst = pin->getInst();
                if (inst) {
                    io_manager.write("\n  ( %s ", inst->getName().c_str());
                } else {
                    io_manager.write("\n  ( PIN ");
                }
                io_manager.write("%s ) ", pin->getName().c_str());
            }
        }
    }

    if (net_type_) {
        if (isAnalog()) {
            io_manager.write(" + USE ANALOG ;\n");
        }
        if (isClock()) {
            io_manager.write(" + USE CLOCK ;\n");
        }
        if (isGround()) {
            io_manager.write(" + USE GROUND ;\n");
        }
        if (isPower()) {
            io_manager.write(" + USE POWER ;\n");
        }
        if (isReset()) {
            io_manager.write(" + USE RESET ;\n");
        }
        if (isScan()) {
            io_manager.write(" + USE SCAN ;\n");
        }
        if (isSignal()) {
            io_manager.write(" + USE SIGNAL ;\n");
        }
        if (isTieOff()) {
            io_manager.write(" + USE TIEOFF ;\n");
        }
    }

    // wire
    if (wires_) {
        ArrayObject<ObjectId>* wire_vector =
            addr<ArrayObject<ObjectId>>(wires_);
        for (ArrayObject<ObjectId>::iterator iter = wire_vector->begin();
             iter != wire_vector->end(); ++iter) {
            Wire* wire = nullptr;
            ObjectId id = (*iter);
            if (id) wire = addr<Wire>(id);
            if (wire) wire->printDEF(io_manager);
        }
    }
    // via
    if (vias_) {
        ArrayObject<ObjectId>* via_vector = addr<ArrayObject<ObjectId>>(vias_);
        for (ArrayObject<ObjectId>::iterator iter = via_vector->begin();
             iter != via_vector->end(); ++iter) {
            Via* via = nullptr;
            ObjectId id = (*iter);
            if (id) via = addr<Via>(id);
            if (via) via->printDEF(io_manager);
        }
    }

    if (SourceType::kNetlist != source_ &&
        SourceType::kUnknown != source_) {
        io_manager.write("  + SOURCE %s\n", toString(source_).c_str());
    }

}

}  // namespace db
}  // namespace open_edi
