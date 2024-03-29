/**
 * @file timinglib_pvt.cpp
 * @date 2020-10-09
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
#include "db/timing/timinglib/timinglib_pvt.h"

namespace open_edi {
namespace db {

TPvt::TPvt()
    : TPvt::BaseType(),
      process_(0.0),
      temperature_(0.0),
      voltage_(0.0),
      group_id_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTPvt);
}

TPvt::~TPvt() {}

TPvt::TPvt(Object* owner, TPvt::IndexType id)
    : TPvt::BaseType(owner, id),
      process_(0.0),
      temperature_(0.0),
      voltage_(0.0),
      group_id_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTPvt);
}

TPvt::TPvt(TPvt const& rhs) { copy(rhs); }

TPvt::TPvt(TPvt&& rhs) noexcept { move(std::move(rhs)); }

TPvt& TPvt::operator=(TPvt const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

TPvt& TPvt::operator=(TPvt&& rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

void TPvt::print(std::ostream& stream) {
    stream << " process: " << process_ << " voltage: " << voltage_
           << " temperature: " << temperature_ << std::endl;
}

void TPvt::copy(TPvt const& rhs) {
    this->BaseType::copy(rhs);
    process_ = rhs.process_;
    temperature_ = rhs.temperature_;
    voltage_ = rhs.voltage_;
    group_id_ = rhs.group_id_;
}

void TPvt::move(TPvt&& rhs) {
    this->BaseType::move(std::move(rhs));

    process_ = std::move(rhs.process_);
    temperature_ = std::move(rhs.temperature_);
    voltage_ = std::move(rhs.voltage_);
    group_id_ = std::move(rhs.group_id_);
}

TPvt::IndexType TPvt::memory() const {
    IndexType ret = this->BaseType::memory();

    ret += sizeof(process_);
    ret += sizeof(temperature_);
    ret += sizeof(voltage_);
    ret += sizeof(group_id_);

    return ret;
}

void TPvt::setProcess(float c) { process_ = c; }
void TPvt::setTemperature(float t) { temperature_ = t; }
void TPvt::setVoltage(float v) { voltage_ = v; }
void TPvt::setGroupId(ObjectId id) { group_id_ = id; }

float TPvt::getProcess(void) { return process_; }
float TPvt::getTemperature(void) { return temperature_; }
float TPvt::getVoltage(void) { return voltage_; }
ObjectId TPvt::getGroupId(void) { return group_id_; }

OStreamBase& operator<<(OStreamBase& os, TPvt const& rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");

    TPvt::BaseType const& base = rhs;
    os << base << DataDelimiter();

    os << DataFieldName("process_") << rhs.process_ << DataDelimiter();
    os << DataFieldName("temperature_") << rhs.temperature_ << DataDelimiter();
    os << DataFieldName("voltage_") << rhs.voltage_ << DataDelimiter();
    os << DataFieldName("group_id_") << rhs.group_id_;

    os << DataEnd(")");
    return os;
}
}  // namespace db
}  // namespace open_edi
