/**
 * @file timinglib_timingarc.cpp
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
#include "db/timing/timinglib/timinglib_timingarc.h"

#include "db/core/db.h"
#include "db/timing/timinglib/timinglib_function.h"
#include "db/timing/timinglib/timinglib_term.h"
#include "db/timing/timinglib/timinglib_timingtable.h"

namespace open_edi {
namespace db {

TimingArcData::TimingArcData()
    : TimingArcData::BaseType(),
      timing_sense_(TimingSense::kUnknown),
      timing_type_(TimingType::kUnknown),
      when_(UNINIT_OBJECT_ID),
      cell_rise_(UNINIT_OBJECT_ID),
      cell_fall_(UNINIT_OBJECT_ID),
      rise_transition_(UNINIT_OBJECT_ID),
      fall_transition_(UNINIT_OBJECT_ID),
      rise_constraint_(UNINIT_OBJECT_ID),
      fall_constraint_(UNINIT_OBJECT_ID),
      group_id_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTimingArc);
}

TimingArcData::~TimingArcData() {}

TimingArcData::TimingArcData(Object* owner, TimingArcData::IndexType id)
    : TimingArc::BaseType(owner, id),
      timing_sense_(TimingSense::kUnknown),
      timing_type_(TimingType::kUnknown),
      when_(UNINIT_OBJECT_ID),
      cell_rise_(UNINIT_OBJECT_ID),
      cell_fall_(UNINIT_OBJECT_ID),
      rise_transition_(UNINIT_OBJECT_ID),
      fall_transition_(UNINIT_OBJECT_ID),
      rise_constraint_(UNINIT_OBJECT_ID),
      fall_constraint_(UNINIT_OBJECT_ID),
      group_id_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTimingArc);
}

TimingArcData::TimingArcData(TimingArcData const& rhs) { copy(rhs); }

TimingArcData::TimingArcData(TimingArcData&& rhs) noexcept { move(std::move(rhs)); }

TimingArcData& TimingArcData::operator=(TimingArcData const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

TimingArcData& TimingArcData::operator=(TimingArcData&& rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

void TimingArcData::copy(TimingArcData const& rhs) {
    this->BaseType::copy(rhs);

    timing_sense_ = rhs.timing_sense_;
    timing_type_ = rhs.timing_type_;
    when_ = rhs.when_;
    cell_rise_ = rhs.cell_rise_;
    cell_fall_ = rhs.cell_fall_;
    rise_transition_ = rhs.rise_transition_;
    fall_transition_ = rhs.fall_transition_;
    rise_constraint_ = rhs.rise_constraint_;
    fall_constraint_ = rhs.fall_constraint_;
    group_id_ = rhs.group_id_;
}

void TimingArcData::move(TimingArcData&& rhs) {
    this->BaseType::move(std::move(rhs));

    timing_sense_ = std::move(rhs.timing_sense_);
    timing_type_ = std::move(rhs.timing_type_);
    when_ = std::move(rhs.when_);
    cell_rise_ = std::move(rhs.cell_rise_);
    cell_fall_ = std::move(rhs.cell_fall_);
    rise_transition_ = std::move(rhs.rise_transition_);
    fall_transition_ = std::move(rhs.fall_transition_);
    rise_constraint_ = std::move(rhs.rise_constraint_);
    fall_constraint_ = std::move(rhs.fall_constraint_);
    group_id_ = std::move(rhs.group_id_);
}

TimingArcData::IndexType TimingArcData::memory() const {
    IndexType ret = this->BaseType::memory();

    ret += sizeof(timing_sense_);
    ret += sizeof(timing_type_);
    ret += sizeof(when_);
    ret += sizeof(cell_rise_);
    ret += sizeof(cell_fall_);
    ret += sizeof(rise_transition_);
    ret += sizeof(fall_transition_);
    ret += sizeof(rise_constraint_);
    ret += sizeof(fall_constraint_);
    ret += sizeof(group_id_);

    return ret;
}

/// set
void TimingArcData::setTimingSense(TimingSense ts) { timing_sense_ = ts; }
void TimingArcData::setTimingType(TimingType tt) { timing_type_ = tt; }
TFunction* TimingArcData::setWhen(const std::string& str) {
    Timing* timing_lib = getTimingLib();
    if (timing_lib) {
        auto p = Object::createObject<TFunction>(kObjectTypeTFunction,
                                                 timing_lib->getId());
        if (p) {
            when_ = p->getId();
            p->setFuncStr(str);
            p->setGroupId(this->getId());
        }
        return p;
    }
    return nullptr;
}
void TimingArcData::setCellRise(ObjectId id) { cell_rise_ = id; }
void TimingArcData::setCellFall(ObjectId id) { cell_fall_ = id; }
void TimingArcData::setRiseTransition(ObjectId id) { rise_transition_ = id; }
void TimingArcData::setFallTransition(ObjectId id) { fall_transition_ = id; }
void TimingArcData::setRiseConstraint(ObjectId id) { rise_constraint_ = id; }
void TimingArcData::setFallConstraint(ObjectId id) { fall_constraint_ = id; }
void TimingArcData::setGroupId(ObjectId id) { group_id_ = id; }

/// get
TimingSense TimingArcData::getTimingSense(void) const { return timing_sense_; }
TimingType TimingArcData::getTimingType(void) const { return timing_type_; }
TFunction* TimingArcData::getWhen(void) const { return Object::addr<TFunction>(when_); }
TimingTable* TimingArcData::getCellRise(void) const { return Object::addr<TimingTable>(cell_rise_); }
TimingTable* TimingArcData::getCellFall(void) const { return Object::addr<TimingTable>(cell_fall_); }
TimingTable* TimingArcData::getRiseTransition(void) const { return Object::addr<TimingTable>(rise_transition_); }
TimingTable* TimingArcData::getFallTransition(void) const { return Object::addr<TimingTable>(fall_transition_); }
TimingTable* TimingArcData::getRiseConstraint(void) const { return Object::addr<TimingTable>(rise_constraint_); }
TimingTable* TimingArcData::getFallConstraint(void) const { return Object::addr<TimingTable>(fall_constraint_); }
ObjectType getTimingtableObjectType(const std::string& str) {
    if (str == "kObjectTypeTimingTable")
        return kObjectTypeTimingTable;
    else if (str == "kObjectTypeTimingTable0")
        return kObjectTypeTimingTable0;
    else if (str == "kObjectTypeTimingTable1")
        return kObjectTypeTimingTable1;
    else if (str == "kObjectTypeTimingTable2")
        return kObjectTypeTimingTable2;
    else if (str == "kObjectTypeTimingTable3")
        return kObjectTypeTimingTable3;
    return kObjectTypeMax;
}

std::string getTimingtableStr(ObjectType t) {
    switch (t) {
        case kObjectTypeTimingTable:
            return "kObjectTypeTimingTable";
        case kObjectTypeTimingTable0:
            return "kObjectTypeTimingTable0";
        case kObjectTypeTimingTable1:
            return "kObjectTypeTimingTable1";
        case kObjectTypeTimingTable2:
            return "kObjectTypeTimingTable2";
        case kObjectTypeTimingTable3:
            return "kObjectTypeTimingTable3";
        default:
            return "";
    }
}
ObjectId TimingArcData::getGroupId(void) const { return group_id_; }

void outputTimingTable(OStreamBase* os, ObjectId id) {
    *os << id;
    *os << DataBegin("[");
    if (id != UNINIT_OBJECT_ID) {
        auto p = Object::addr<TimingTable>(id);
        if (p) {
            if (p->getObjectType() == kObjectTypeTimingTable0) {
                auto q = dynamic_cast<TimingTable0*>(p);
                if (q) {
                    *os << getTimingtableStr(p->getObjectType())
                        << DataDelimiter();
                    *os << *q;
                }
            } else if (p->getObjectType() == kObjectTypeTimingTable1) {
                auto q = dynamic_cast<TimingTable1*>(p);
                if (q) {
                    *os << getTimingtableStr(p->getObjectType())
                        << DataDelimiter();
                    *os << *q;
                }
            } else if (p->getObjectType() == kObjectTypeTimingTable2) {
                auto q = dynamic_cast<TimingTable2*>(p);
                if (q) {
                    *os << getTimingtableStr(p->getObjectType())
                        << DataDelimiter();
                    *os << *q;
                }
            } else if (p->getObjectType() == kObjectTypeTimingTable3) {
                auto q = dynamic_cast<TimingTable3*>(p);
                if (q) {
                    *os << getTimingtableStr(p->getObjectType())
                        << DataDelimiter();
                    *os << *q;
                }
            } else {
                *os << getTimingtableStr(p->getObjectType()) << DataDelimiter();
                *os << *p;
            }
        }
    }
    *os << DataEnd("]");
}

OStreamBase& operator<<(OStreamBase& os, TimingArcData const& rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");

    TimingArc::BaseType const& base = rhs;
    os << base << DataDelimiter();

    os << DataFieldName("timing_sense_") << rhs.timing_sense_
       << DataDelimiter();
    os << DataFieldName("timing_type_") << rhs.timing_type_ << DataDelimiter();
    os << DataFieldName("when_") << rhs.when_;
    {
        os << DataBegin("[");
        if (rhs.when_ != UNINIT_OBJECT_ID) {
            auto p = Object::addr<TFunction>(rhs.when_);
            if (p) os << *p;
        }
        os << DataEnd("]") << DataDelimiter();
    }
    os << DataFieldName("cell_rise_");
    outputTimingTable(&os, rhs.cell_rise_);
    os << DataDelimiter();

    os << DataFieldName("cell_fall_");
    outputTimingTable(&os, rhs.cell_fall_);
    os << DataDelimiter();

    os << DataFieldName("rise_transition_");
    outputTimingTable(&os, rhs.rise_transition_);
    os << DataDelimiter();

    os << DataFieldName("fall_transition_");
    outputTimingTable(&os, rhs.fall_transition_);
    os << DataDelimiter();

    os << DataFieldName("rise_constraint_");
    outputTimingTable(&os, rhs.rise_constraint_);
    os << DataDelimiter();

    os << DataFieldName("fall_constraint_");
    outputTimingTable(&os, rhs.fall_constraint_);
    os << DataDelimiter();

    os << DataFieldName("group_id_") << rhs.group_id_;

    os << DataEnd(")");
    return os;
}

TimingArc::TimingArc()
    : TimingArc::BaseType(),
      is_disabled_(false),
      related_pin_(UNINIT_OBJECT_ID),
      group_id_(UNINIT_OBJECT_ID),
      data_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTimingArc);
}

TimingArc::~TimingArc() {}

TimingArc::TimingArc(Object* owner, TimingArc::IndexType id)
    : TimingArc::BaseType(owner, id),
      is_disabled_(false),
      related_pin_(UNINIT_OBJECT_ID),
      group_id_(UNINIT_OBJECT_ID),
      data_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTimingArc);
}

TimingArc::TimingArc(TimingArc const& rhs) { copy(rhs); }

TimingArc::TimingArc(TimingArc&& rhs) noexcept { move(std::move(rhs)); }

TimingArc& TimingArc::operator=(TimingArc const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

TimingArc& TimingArc::operator=(TimingArc&& rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

void TimingArc::copy(TimingArc const& rhs) {
    this->BaseType::copy(rhs);

    related_pin_ = rhs.related_pin_;
    group_id_ = rhs.group_id_;
    data_ = rhs.data_;
    is_disabled_ = rhs.is_disabled_;
}

void TimingArc::move(TimingArc&& rhs) {
    this->BaseType::move(std::move(rhs));

    related_pin_ = std::move(rhs.related_pin_);
    group_id_ = std::move(rhs.group_id_);
    data_ = std::move(rhs.data_);
    is_disabled_ = std::move(rhs.is_disabled_);
}

TimingArc::IndexType TimingArc::memory() const {
    IndexType ret = this->BaseType::memory();

    ret += sizeof(related_pin_);
    ret += sizeof(group_id_);
    ret += sizeof(data_);
    ret += sizeof(is_disabled_);

    return ret;
}

/// set
void TimingArc::setRelatedPin(ObjectId id) { related_pin_ = id; }
void TimingArc::setGroupId(ObjectId id) { group_id_ = id; }
TimingArcData* TimingArc::createData() {
    Timing* timing_lib = getTimingLib();
    if (timing_lib) {
        auto p = Object::createObject<TimingArcData>(kObjectTypeTFunction,
                                                 timing_lib->getId());
        if (p) {
            data_ = p->getId();
            p->setGroupId(this->getId());
        }
        return p;
    }
    return nullptr;
}
void TimingArc::setDisabled(void) { is_disabled_ = true; }

/// set
void TimingArc::setTimingSense(TimingSense ts) {
    TimingArcData *data = getData();
    if (data != nullptr) {
        data->setTimingSense(ts);
    }
}
void TimingArc::setTimingType(TimingType tt) {
    TimingArcData *data = getData();
    if (data != nullptr) {
        data->setTimingType(tt);
    }
}
TFunction* TimingArc::setWhen(const std::string& str) {
    TimingArcData *data = getData();
    if (data != nullptr) {
        return data->setWhen(str);
    }
    return nullptr;
}
TimingTable* TimingArc::createTimingTable(ObjectType type) {
    Timing* timing_lib = getTimingLib();
    if (timing_lib) {
        TimingTable* p = nullptr;
        if (type == kObjectTypeTimingTable)
            p = Object::createObject<TimingTable>(kObjectTypeTimingTable,
                                                  timing_lib->getId());
        else if (type == kObjectTypeTimingTable0)
            p = Object::createObject<TimingTable0>(kObjectTypeTimingTable0,
                                                   timing_lib->getId());
        else if (type == kObjectTypeTimingTable1)
            p = Object::createObject<TimingTable1>(kObjectTypeTimingTable1,
                                                   timing_lib->getId());
        else if (type == kObjectTypeTimingTable2)
            p = Object::createObject<TimingTable2>(kObjectTypeTimingTable2,
                                                   timing_lib->getId());
        else if (type == kObjectTypeTimingTable3)
            p = Object::createObject<TimingTable3>(kObjectTypeTimingTable3,
                                                   timing_lib->getId());
        return p;
    }
    return nullptr;
}
void TimingArc::setCellRise(ObjectId id) {
    TimingArcData *data = getData();
    if (data != nullptr) {
        return data->setCellRise(id);
    }
}
void TimingArc::setCellFall(ObjectId id) {
    TimingArcData *data = getData();
    if (data != nullptr) {
        return data->setCellFall(id);
    }
}
void TimingArc::setRiseTransition(ObjectId id) {
    TimingArcData *data = getData();
    if (data != nullptr) {
        return data->setRiseTransition(id);
    }
}
void TimingArc::setFallTransition(ObjectId id) {
    TimingArcData *data = getData();
    if (data != nullptr) {
        return data->setFallTransition(id);
    }
}
void TimingArc::setRiseConstraint(ObjectId id) {
    TimingArcData *data = getData();
    if (data != nullptr) {
        return data->setRiseConstraint(id);
    }
}
void TimingArc::setFallConstraint(ObjectId id) {
    TimingArcData *data = getData();
    if (data != nullptr) {
        return data->setFallConstraint(id);
    }
}

/// get
TTerm* TimingArc::getRelatedPin() const { return Object::addr<TTerm>(related_pin_); }
ObjectId TimingArc::getGroupId(void) const { return group_id_; }
TimingArcData* TimingArc::getData(void) const { return Object::addr<TimingArcData>(data_); }
bool TimingArc::isDisabled(void) const { return is_disabled_; }

// get
TimingSense TimingArc::getTimingSense(void) const{
    const TimingArcData* data = getData();
    if (data == nullptr) {
        return TimingSense::kUnknown;
    }
    return data->getTimingSense(); 
}
TimingType TimingArc::getTimingType(void) const {
    const TimingArcData* data = getData();
    if (data == nullptr) {
        return TimingType::kUnknown;
    }
    return data->getTimingType(); 
}
TFunction* TimingArc::getWhen(void) const {
    const TimingArcData* data = getData();
    return data ? data->getWhen() : nullptr;
}
TimingTable* TimingArc::getCellRise(void) const {
    const TimingArcData* data = getData();
    return data ? data->getCellRise() : nullptr;
}
TimingTable* TimingArc::getCellFall(void) const {
    const TimingArcData* data = getData();
    return data ? data->getCellFall() : nullptr;
}
TimingTable* TimingArc::getRiseTransition(void) const {
    const TimingArcData* data = getData();
    return data ? data->getRiseTransition() : nullptr;
}
TimingTable* TimingArc::getFallTransition(void) const {
    const TimingArcData* data = getData();
    return data ? data->getFallTransition() : nullptr;
}
TimingTable* TimingArc::getRiseConstraint(void) const {
    const TimingArcData* data = getData();
    return data ? data->getRiseConstraint() : nullptr;
}
TimingTable* TimingArc::getFallConstraint(void) const {
    const TimingArcData* data = getData();
    return data ? data->getFallConstraint() : nullptr;
}

OStreamBase& operator<<(OStreamBase& os, TimingArc const& rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");
    TimingArc::BaseType const& base = rhs;
    os << base << DataDelimiter();
    os << DataFieldName("related_pin_");
    const TTerm *related_pin = rhs.getRelatedPin();
    if (related_pin) {
        os << related_pin->getName();
    }
    os << DataDelimiter();
    os << DataFieldName("group_id_") << rhs.getGroupId() << DataDelimiter();
    os << DataFieldName("data_");
    const auto &data = rhs.getData();
    if (data) {
        os << *data;
    }
    os << DataDelimiter();
    os << DataFieldName("is_disabled_") << rhs.isDisabled() << DataDelimiter();
    os << DataEnd(")");
    return os;
}

}  // namespace db
}  // namespace open_edi
