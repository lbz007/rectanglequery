/**
 * @file timinglib_timingtable.cpp
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
#include "db/timing/timinglib/timinglib_timingtable.h"

#include "db/core/db.h"
#include "db/timing/timinglib/timinglib_tabletemplate.h"

namespace open_edi {
namespace db {

TimingTable::TimingTable()
    : TimingTable::BaseType(), group_id_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTimingTable);
}

TimingTable::~TimingTable() {}

TimingTable::TimingTable(Object* owner, TimingTable::IndexType id)
    : TimingTable::BaseType(owner, id), group_id_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTimingTable);
}

TimingTable::TimingTable(TimingTable const& rhs) { copy(rhs); }

TimingTable::TimingTable(TimingTable&& rhs) noexcept { move(std::move(rhs)); }

TimingTable& TimingTable::operator=(TimingTable const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

TimingTable& TimingTable::operator=(TimingTable&& rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

void TimingTable::copy(TimingTable const& rhs) { this->BaseType::copy(rhs); }

void TimingTable::move(TimingTable&& rhs) {
    this->BaseType::move(std::move(rhs));
}

TimingTable::IndexType TimingTable::memory() const {
    IndexType ret = this->BaseType::memory();

    ret += sizeof(group_id_);

    return ret;
}

void TimingTable::setGroupId(ObjectId id) { group_id_ = id; }

TableAxis* TimingTable::getAxis1(void) { return nullptr; }
TableAxis* TimingTable::getAxis2(void) { return nullptr; }
TableAxis* TimingTable::getAxis3(void) { return nullptr; }
ObjectId TimingTable::getGroupId(void) { return group_id_; }
float TimingTable::findValue(float, float, float, float, float, float, float) {
    return 0.0f;
}
void TimingTable::getAxisValueByAxisVariable(float inSlew, float loadCap,
                                             float relatedOutCap,
                                             float& axis1Value,
                                             float& axis2Value,
                                             float& axis3Value) {}
void TimingTable::getAxisValueScaleByAxisVariable(
    float inSlewScale, float loadCapScale, float relatedOutCapScale,
    float& axis1ValueScale, float& axis2ValueScale, float& axis3ValueScale) {}
OStreamBase& operator<<(OStreamBase& os, TimingTable const& rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");

    TimingTable::BaseType const& base = rhs;
    os << base << DataDelimiter();
    os << DataFieldName("group_id_") << rhs.group_id_;

    os << DataEnd(")");
    return os;
}

TimingTable0::TimingTable0() : TimingTable0::BaseType(), value_(0.0) {
    setObjectType(ObjectType::kObjectTypeTimingTable0);
}

TimingTable0::~TimingTable0() {}

TimingTable0::TimingTable0(Object* owner, TimingTable0::IndexType id)
    : TimingTable0::BaseType(owner, id), value_(0.0) {
    setObjectType(ObjectType::kObjectTypeTimingTable0);
}

TimingTable0::TimingTable0(TimingTable0 const& rhs) { copy(rhs); }

TimingTable0::TimingTable0(TimingTable0&& rhs) noexcept {
    move(std::move(rhs));
}

TimingTable0& TimingTable0::operator=(TimingTable0 const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

TimingTable0& TimingTable0::operator=(TimingTable0&& rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

void TimingTable0::copy(TimingTable0 const& rhs) {
    this->BaseType::copy(rhs);
    value_ = rhs.value_;
}

void TimingTable0::move(TimingTable0&& rhs) {
    this->BaseType::move(std::move(rhs));
    value_ = std::move(rhs.value_);
}

TimingTable0::IndexType TimingTable0::memory() const {
    IndexType ret = this->BaseType::memory();

    ret += sizeof(value_);

    return ret;
}
void TimingTable0::setValue(float f) { value_ = f; }
float TimingTable0::getValue(float scale /*=1.0f*/) { return value_ * scale; }
float TimingTable0::findValue(float, float, float,
                              float timeUnitScale /*=1.0f*/, float, float,
                              float) {
    return value_ * timeUnitScale;
}
void TimingTable0::getAxisValueByAxisVariable(float, float, float,
                                              float& axis1Value,
                                              float& axis2Value,
                                              float& axis3Value) {
    axis1Value = axis2Value = axis3Value = 0.0f;
}
void TimingTable0::getAxisValueScaleByAxisVariable(float, float, float,
                                                   float& axis1ValueScale,
                                                   float& axis2ValueScale,
                                                   float& axis3ValueScale) {
    axis1ValueScale = axis2ValueScale = axis3ValueScale = 1.0f;
}
OStreamBase& operator<<(OStreamBase& os, TimingTable0 const& rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");

    TimingTable0::BaseType const& base = rhs;
    os << base << DataDelimiter();

    os << DataFieldName("value_") << rhs.value_;

    os << DataEnd(")");
    return os;
}

TimingTable1::TimingTable1()
    : TimingTable1::BaseType(),
      values_(UNINIT_OBJECT_ID),
      axis1_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTimingTable1);
}

TimingTable1::~TimingTable1() {
#if 0
    Timing* timing_lib = getTimingLib();
    if (timing_lib && axis1_ != UNINIT_OBJECT_ID) {
        TableAxis* t = addr<TableAxis>(axis1_);
        if (t) timing_lib->destroyObject<TableAxis>(t);
    }
#endif
}

TimingTable1::TimingTable1(Object* owner, TimingTable1::IndexType id)
    : TimingTable1::BaseType(owner, id),
      values_(UNINIT_OBJECT_ID),
      axis1_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTimingTable1);
}

TimingTable1::TimingTable1(TimingTable1 const& rhs) { copy(rhs); }

TimingTable1::TimingTable1(TimingTable1&& rhs) noexcept {
    move(std::move(rhs));
}

TimingTable1& TimingTable1::operator=(TimingTable1 const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

TimingTable1& TimingTable1::operator=(TimingTable1&& rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

void TimingTable1::copy(TimingTable1 const& rhs) {
    this->BaseType::copy(rhs);
    values_ = rhs.values_;
    axis1_ = rhs.axis1_;
}

void TimingTable1::move(TimingTable1&& rhs) {
    this->BaseType::move(std::move(rhs));
    values_ = std::move(rhs.values_);
    axis1_ = std::move(rhs.axis1_);
}

TimingTable1::IndexType TimingTable1::memory() const {
    IndexType ret = this->BaseType::memory();

    ret += sizeof(values_);
    ret += sizeof(axis1_);

    return ret;
}

void TimingTable1::addValue(float f) {
    ArrayObject<float>* p = nullptr;
    if (values_ == UNINIT_OBJECT_ID) {
        Timing* timing_lib = getTimingLib();
        if (timing_lib != nullptr) {
            p = Object::createObject<ArrayObject<float>>(kObjectTypeArray,
                                                         timing_lib->getId());
            if (p != nullptr) {
                values_ = p->getId();
                p->setPool(timing_lib->getPool());
                p->reserve(32);
            }
        }
    } else {
        p = addr<ArrayObject<float>>(values_);
    }
    if (p != nullptr) p->pushBack(f);
}

void TimingTable1::setAxis1(ObjectId id) { axis1_ = id; }

float TimingTable1::getValue(IndexType index, float scale /*=1.0f*/) {
    if (values_ == UNINIT_OBJECT_ID) return 0.0f;
    ArrayObject<float>* p = addr<ArrayObject<float>>(values_);
    if (p == nullptr) return 0.0f;
    IndexType count = p->getSize();
    if (index >= count) return 0.0f;
    return (*p)[index] * scale;
}

std::vector<float> TimingTable1::getValues(void) {
    std::vector<float> values;
    if (values_ != UNINIT_OBJECT_ID) {
        ArrayObject<float>* p = addr<ArrayObject<float>>(values_);
        if (p != nullptr) {
            for (int64_t i = 0; i < p->getSize(); ++i)
                values.emplace_back((*p)[i]);
        }
    }

    return values;
}

float TimingTable1::findValue(float v1, float, float,
                              float timeUnitScale /*=1.0f*/,
                              float v1Scale /*=1.0f*/, float, float) {
    TableAxis* pAxis1 = getAxis1();
    if (pAxis1 == nullptr) return 0;
    TableAxis::IndexType count = pAxis1->getSize();
    if (count == 1) return getValue(0, timeUnitScale);
    IndexType index1 = pAxis1->findAxisIndex(v1);
    float x1 = pAxis1->getValue(index1, v1Scale);
    float y1 = getValue(index1, timeUnitScale);
    float x2 = pAxis1->getValue(index1 + 1, v1Scale);
    float y2 = getValue(index1 + 1, timeUnitScale);
    float dx = (v1 * v1Scale - x1) / (x2 - x1);
    return (1 - dx) * y1 + dx * y2;
}

void TimingTable1::getAxisValueByAxisVariable(float inSlew, float loadCap,
                                              float relatedOutCap,
                                              float& axis1Value,
                                              float& axis2Value,
                                              float& axis3Value) {
    axis2Value = axis3Value = 0.0f;
    TableAxis* pAxis1 = getAxis1();
    if (pAxis1 == nullptr)
        axis1Value = 0.0f;
    else
        axis1Value =
            pAxis1->getValueByAxisVariable(inSlew, loadCap, relatedOutCap);
}
void TimingTable1::getAxisValueScaleByAxisVariable(
    float inSlewScale, float loadCapScale, float relatedOutCapScale,
    float& axis1ValueScale, float& axis2ValueScale, float& axis3ValueScale) {
    axis2ValueScale = axis3ValueScale = 1.0f;
    TableAxis* pAxis1 = getAxis1();
    if (pAxis1 == nullptr)
        axis1ValueScale = 1.0f;
    else
        axis1ValueScale = pAxis1->getValueScaleByAxisVariable(
            inSlewScale, loadCapScale, relatedOutCapScale);
}
TableAxis* TimingTable1::getAxis1(void) {
    if (axis1_ != UNINIT_OBJECT_ID)
        return addr<TableAxis>(axis1_);
    else
        return nullptr;
}

OStreamBase& operator<<(OStreamBase& os, TimingTable1 const& rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");

    TimingTable1::BaseType const& base = rhs;
    os << base << DataDelimiter();

    os << DataFieldName("values_");
    ArrayObject<float>* p = nullptr;
    if (rhs.values_ != UNINIT_OBJECT_ID)
        p = Object::addr<ArrayObject<float>>(rhs.values_);
    if (p != nullptr)
        os << p->getSize();
    else
        os << 0;
    os << DataBegin("[");
    if (p != nullptr) {
        auto delimiter = DataDelimiter("");
        for (int64_t i = 0; i < p->getSize(); ++i) {
            os << delimiter << (*p)[i];
            delimiter = DataDelimiter();
        }
    }
    os << DataEnd("]") << DataDelimiter();

    os << DataFieldName("axis1_") << rhs.axis1_;
    os << DataBegin("[");
    if (rhs.axis1_ != UNINIT_OBJECT_ID) {
        TableAxis* t = Object::addr<TableAxis>(rhs.axis1_);
        if (t) os << *t;
    }
    os << DataEnd("]");

    os << DataEnd(")");
    return os;
}

TimingTable2::TimingTable2()
    : TimingTable2::BaseType(),
      values_(UNINIT_OBJECT_ID),
      axis1_(UNINIT_OBJECT_ID),
      axis2_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTimingTable2);
}

TimingTable2::~TimingTable2() {
#if 0
    Timing* timing_lib = getTimingLib();
    if (timing_lib && axis1_ != UNINIT_OBJECT_ID) {
        TableAxis* t = addr<TableAxis>(axis1_);
        if (t) timing_lib->destroyObject<TableAxis>(t);
    }
    if (timing_lib && axis2_ != UNINIT_OBJECT_ID) {
        TableAxis* t = addr<TableAxis>(axis2_);
        if (t) timing_lib->destroyObject<TableAxis>(t);
    }
#endif
}

TimingTable2::TimingTable2(Object* owner, TimingTable2::IndexType id)
    : TimingTable2::BaseType(owner, id),
      values_(UNINIT_OBJECT_ID),
      axis1_(UNINIT_OBJECT_ID),
      axis2_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTimingTable2);
}

TimingTable2::TimingTable2(TimingTable2 const& rhs) { copy(rhs); }

TimingTable2::TimingTable2(TimingTable2&& rhs) noexcept {
    move(std::move(rhs));
}

TimingTable2& TimingTable2::operator=(TimingTable2 const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

TimingTable2& TimingTable2::operator=(TimingTable2&& rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

void TimingTable2::copy(TimingTable2 const& rhs) {
    this->BaseType::copy(rhs);
    values_ = rhs.values_;
    axis1_ = rhs.axis1_;
    axis2_ = rhs.axis2_;
}

void TimingTable2::move(TimingTable2&& rhs) {
    this->BaseType::move(std::move(rhs));
    values_ = std::move(rhs.values_);
    axis1_ = std::move(rhs.axis1_);
    axis2_ = std::move(rhs.axis2_);
}

TimingTable2::IndexType TimingTable2::memory() const {
    IndexType ret = this->BaseType::memory();

    ret += sizeof(values_);
    ret += sizeof(axis1_);
    ret += sizeof(axis2_);

    return ret;
}

void TimingTable2::addValue(float f) {
    ArrayObject<float>* p = nullptr;
    if (values_ == UNINIT_OBJECT_ID) {
        Timing* timing_lib = getTimingLib();
        if (timing_lib != nullptr) {
            p = Object::createObject<ArrayObject<float>>(kObjectTypeArray,
                                                         timing_lib->getId());
            if (p != nullptr) {
                values_ = p->getId();
                p->setPool(timing_lib->getPool());
                p->reserve(32);
            }
        }
    } else {
        p = addr<ArrayObject<float>>(values_);
    }
    if (p != nullptr) p->pushBack(f);
}
void TimingTable2::setAxis1(ObjectId id) { axis1_ = id; }
void TimingTable2::setAxis2(ObjectId id) { axis2_ = id; }

float TimingTable2::getValue(IndexType index1, IndexType index2,
                             float scale /*=1.0f*/) {
    TableAxis* t = getAxis1();
    if (t) {
        ArrayObject<float>* p = nullptr;
        if (values_ != UNINIT_OBJECT_ID) p = addr<ArrayObject<float>>(values_);
        if (p != nullptr) {
            IndexType index = index1 * t->getSize() + index2;
            if (index >= p->getSize()) return 0.0f;
            return (*p)[index] * scale;
        }
    }
    return 0.0;
}

float TimingTable2::findValue(float v1, float v2, float,
                              float timeUnitScale /*=1.0f*/,
                              float v1Scale /*=1.0f*/, float v2Scale /*=1.0f*/,
                              float) {
    TableAxis* pAxis1 = getAxis1();
    TableAxis* pAxis2 = getAxis2();
    if (pAxis1 == nullptr || pAxis2 == nullptr) return 0.0f;
    IndexType axis1_count = pAxis1->getSize();
    IndexType axis2_count = pAxis2->getSize();
    if (axis1_count == 1 && axis2_count == 1)
        return getValue(0, 0, timeUnitScale);
    if (axis1_count == 1) {
        IndexType index2 = pAxis2->findAxisIndex(v2);
        float y00 = getValue(0, index2, timeUnitScale);
        float x2_lower = pAxis2->getValue(index2, v2Scale);
        float x2_upper = pAxis2->getValue(index2 + 1, v2Scale);
        float dx = (v2 * v2Scale - x2_lower) / (x2_upper - x2_lower);
        float y01 = getValue(0, index2 + 1, timeUnitScale);
        return ((1 - dx) * y00 + dx * y01) * v2Scale;
    } else if (axis2_count == 1) {
        IndexType index1 = pAxis1->findAxisIndex(v1);
        float y00 = getValue(index1, 0, timeUnitScale);
        float x1_lower = pAxis1->getValue(index1, v1Scale);
        float x1_upper = pAxis1->getValue(index1 + 1, v1Scale);
        float dx1 = (v1 * v1Scale - x1_lower) / (x1_upper - x1_lower);
        float y10 = getValue(index1 + 1, 0, timeUnitScale);
        return ((1 - dx1) * y00 + dx1 * y10) * v1Scale;
    } else {
        IndexType index1 = pAxis1->findAxisIndex(v1);
        IndexType index2 = pAxis2->findAxisIndex(v2);
        float y00 = getValue(index1, index2, timeUnitScale);
        float x1_lower = pAxis1->getValue(index1, v1Scale);
        float x1_upper = pAxis1->getValue(index1 + 1, v1Scale);
        float dx1 = (v1 * v1Scale - x1_lower) / (x1_upper - x1_lower);
        float y10 = getValue(index1 + 1, index2, timeUnitScale);
        float y11 = getValue(index1 + 1, index2 + 1, timeUnitScale);
        float x2_lower = pAxis2->getValue(index2, v2Scale);
        float x2_upper = pAxis2->getValue(index2 + 1, v2Scale);
        float dx2 = (v2 * v2Scale - x2_lower) / (x2_upper - x2_lower);
        float y01 = getValue(index1, index2 + 1, timeUnitScale);
        float result = (1 - dx1) * (1 - dx2) * y00 + dx1 * (1 - dx2) * y10 +
                       dx1 * dx2 * y11 + (1 - dx1) * dx2 * y01;
        return result;
    }
}

void TimingTable2::getAxisValueByAxisVariable(float inSlew, float loadCap,
                                              float relatedOutCap,
                                              float& axis1Value,
                                              float& axis2Value,
                                              float& axis3Value) {
    axis3Value = 0.0f;
    TableAxis* pAxis1 = getAxis1();
    if (pAxis1 == nullptr)
        axis1Value = 0.0f;
    else
        axis1Value =
            pAxis1->getValueByAxisVariable(inSlew, loadCap, relatedOutCap);
    TableAxis* pAxis2 = getAxis2();
    if (pAxis2 == nullptr)
        axis2Value = 0.0f;
    else
        axis2Value =
            pAxis2->getValueByAxisVariable(inSlew, loadCap, relatedOutCap);
}
void TimingTable2::getAxisValueScaleByAxisVariable(
    float inSlewScale, float loadCapScale, float relatedOutCapScale,
    float& axis1ValueScale, float& axis2ValueScale, float& axis3ValueScale) {
    axis3ValueScale = 1.0f;
    TableAxis* pAxis1 = getAxis1();
    if (pAxis1 == nullptr)
        axis1ValueScale = 1.0f;
    else
        axis1ValueScale = pAxis1->getValueScaleByAxisVariable(
            inSlewScale, loadCapScale, relatedOutCapScale);
    TableAxis* pAxis2 = getAxis2();
    if (pAxis2 == nullptr)
        axis2ValueScale = 1.0f;
    else
        axis2ValueScale = pAxis2->getValueScaleByAxisVariable(
            inSlewScale, loadCapScale, relatedOutCapScale);
}

TableAxis* TimingTable2::getAxis1(void) {
    if (axis1_ != UNINIT_OBJECT_ID)
        return addr<TableAxis>(axis1_);
    else
        return nullptr;
}

TableAxis* TimingTable2::getAxis2(void) {
    if (axis2_ != UNINIT_OBJECT_ID)
        return addr<TableAxis>(axis2_);
    else
        return nullptr;
}

OStreamBase& operator<<(OStreamBase& os, TimingTable2 const& rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");

    TimingTable2::BaseType const& base = rhs;
    os << base << DataDelimiter();

    os << DataFieldName("values_");
    {
        ArrayObject<float>* p = nullptr;
        if (rhs.values_ != UNINIT_OBJECT_ID)
            p = Object::addr<ArrayObject<float>>(rhs.values_);
        if (p != nullptr)
            os << p->getSize();
        else
            os << 0;
        os << DataBegin("[");
        if (p != nullptr) {
            auto delimiter = DataDelimiter("");
            for (int64_t i = 0; i < p->getSize(); ++i) {
                os << delimiter << (*p)[i];
                delimiter = DataDelimiter();
            }
        }
        os << DataEnd("]") << DataDelimiter();
    }

    os << DataFieldName("axis1_") << rhs.axis1_;
    os << DataBegin("[");
    if (rhs.axis1_ != UNINIT_OBJECT_ID) {
        TableAxis* t = Object::addr<TableAxis>(rhs.axis1_);
        if (t) os << *t;
    }
    os << DataEnd("]") << DataDelimiter();

    os << DataFieldName("axis2_") << rhs.axis2_;
    os << DataBegin("[");
    if (rhs.axis2_ != UNINIT_OBJECT_ID) {
        TableAxis* t = Object::addr<TableAxis>(rhs.axis2_);
        if (t) os << *t;
    }
    os << DataEnd("]");

    os << DataEnd(")");
    return os;
}

TimingTable3::TimingTable3()
    : TimingTable3::BaseType(), axis3_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTimingTable3);
}

TimingTable3::~TimingTable3() {
#if 0
    Timing* timing_lib = getTimingLib();
    if (timing_lib && axis3_ != UNINIT_OBJECT_ID) {
        TableAxis* t = addr<TableAxis>(axis3_);
        if (t) timing_lib->destroyObject<TableAxis>(t);
    }
#endif
}

TimingTable3::TimingTable3(Object* owner, TimingTable3::IndexType id)
    : TimingTable3::BaseType(owner, id), axis3_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTimingTable3);
}

TimingTable3::TimingTable3(TimingTable3 const& rhs) { copy(rhs); }

TimingTable3::TimingTable3(TimingTable3&& rhs) noexcept {
    move(std::move(rhs));
}

TimingTable3& TimingTable3::operator=(TimingTable3 const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

TimingTable3& TimingTable3::operator=(TimingTable3&& rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

void TimingTable3::copy(TimingTable3 const& rhs) {
    this->BaseType::copy(rhs);
    axis3_ = rhs.axis3_;
}

void TimingTable3::move(TimingTable3&& rhs) {
    this->BaseType::move(std::move(rhs));
    axis3_ = std::move(rhs.axis3_);
}

TimingTable3::IndexType TimingTable3::memory() const {
    IndexType ret = this->BaseType::memory();

    ret += sizeof(axis3_);

    return ret;
}

void TimingTable3::setAxis3(ObjectId id) { axis3_ = id; }

float TimingTable3::getValue(IndexType index1, IndexType index2,
                             IndexType index3, float scale /*=1.0f*/) {
    TableAxis* t1 = getAxis1();
    TableAxis* t2 = getAxis2();
    if (t1 && t2) {
        ArrayObject<float>* p = nullptr;
        if (values_ != UNINIT_OBJECT_ID) p = addr<ArrayObject<float>>(values_);
        if (p != nullptr) {
            IndexType index =
                (index1 * t1->getSize() + index2) * t2->getSize() + index3;
            if (index >= p->getSize()) return 0.0f;
            return (*p)[index] * scale;
        }
    }

    return 0.0f;
}

float TimingTable3::findValue(float v1, float v2, float v3,
                              float timeUnitScale /*=1.0f*/,
                              float v1Scale /*=1.0f*/, float v2Scale /*=1.0f*/,
                              float v3Scale /*=1.0f*/) {
    TableAxis* pAxis1 = getAxis1();
    TableAxis* pAxis2 = getAxis2();
    TableAxis* pAxis3 = getAxis3();
    if (pAxis1 == nullptr || pAxis2 == nullptr || pAxis3 == nullptr)
        return 0.0f;
    IndexType axis1_count = pAxis1->getSize();
    IndexType axis2_count = pAxis2->getSize();
    IndexType axis3_count = pAxis3->getSize();
    IndexType index1 = pAxis1->findAxisIndex(v1);
    IndexType index2 = pAxis2->findAxisIndex(v2);
    IndexType index3 = pAxis3->findAxisIndex(v3);
    float dx1, dx2, dx3;
    dx1 = dx2 = dx3 = 0.0f;
    float y000 = getValue(index1, index2, index3, timeUnitScale);
    float y001, y010, y011, y100, y101, y110, y111;
    y001 = y010 = y011 = y100 = y101 = y110 = y111 = 0.0;
    if (axis1_count > 1) {
        float x1_lower = pAxis1->getValue(index1, v1Scale);
        float x1_upper = pAxis1->getValue(index1 + 1, v1Scale);
        dx1 = (v1 * v1Scale - x1_lower) / (x1_upper - x1_lower);
        y100 = getValue(index1 + 1, index2, index3, timeUnitScale);
        if (axis2_count > 1) {
            y110 = getValue(index1 + 1, index2 + 1, index3, timeUnitScale);
            if (axis3_count > 1)
                y111 =
                    getValue(index1 + 1, index2 + 1, index3 + 1, timeUnitScale);
        }
        if (axis3_count > 1)
            y101 = getValue(index1 + 1, index2, index3 + 1, timeUnitScale);
    }
    if (axis2_count > 1) {
        float x2_lower = pAxis2->getValue(index2, v2Scale);
        float x2_upper = pAxis2->getValue(index2 + 1, v2Scale);
        dx2 = (v2 * v2Scale - x2_lower) / (x2_upper - x2_lower);
        y010 = getValue(index1, index2 + 1, index3, timeUnitScale);
        if (axis3_count > 1)
            y011 = getValue(index1, index2 + 1, index3 + 1, timeUnitScale);
    }
    if (axis3_count > 1) {
        float x3_lower = pAxis3->getValue(index3, v3Scale);
        float x3_upper = pAxis3->getValue(index3 + 1, v3Scale);
        dx3 = (v3 * v3Scale - x3_lower) / (x3_upper - x3_lower);
        y001 = getValue(index1, index2, index3 + 1, timeUnitScale);
    }
    float result =
        (1 - dx1) * (1 - dx2) * (1 - dx3) * y000 +
        (1 - dx1) * (1 - dx2) * dx3 * y001 +
        (1 - dx1) * dx2 * (1 - dx3) * y010 + (1 - dx1) * dx2 * dx3 * y011 +
        dx1 * (1 - dx2) * (1 - dx3) * y100 + dx1 * (1 - dx2) * dx3 * y101 +
        dx1 * dx2 * (1 - dx3) * y110 + dx1 * dx2 * dx3 * y111;
    return result;
}

void TimingTable3::getAxisValueByAxisVariable(float inSlew, float loadCap,
                                              float relatedOutCap,
                                              float& axis1Value,
                                              float& axis2Value,
                                              float& axis3Value) {
    TableAxis* pAxis1 = getAxis1();
    if (pAxis1 == nullptr)
        axis1Value = 0.0f;
    else
        axis1Value =
            pAxis1->getValueByAxisVariable(inSlew, loadCap, relatedOutCap);
    TableAxis* pAxis2 = getAxis2();
    if (pAxis2 == nullptr)
        axis2Value = 0.0f;
    else
        axis2Value =
            pAxis2->getValueByAxisVariable(inSlew, loadCap, relatedOutCap);
    TableAxis* pAxis3 = getAxis3();
    if (pAxis3 == nullptr)
        axis3Value = 0.0f;
    else
        axis3Value =
            pAxis3->getValueByAxisVariable(inSlew, loadCap, relatedOutCap);
}
void TimingTable3::getAxisValueScaleByAxisVariable(
    float inSlewScale, float loadCapScale, float relatedOutCapScale,
    float& axis1ValueScale, float& axis2ValueScale, float& axis3ValueScale) {
    TableAxis* pAxis1 = getAxis1();
    if (pAxis1 == nullptr)
        axis1ValueScale = 1.0f;
    else
        axis1ValueScale = pAxis1->getValueScaleByAxisVariable(
            inSlewScale, loadCapScale, relatedOutCapScale);
    TableAxis* pAxis2 = getAxis2();
    if (pAxis2 == nullptr)
        axis2ValueScale = 1.0f;
    else
        axis2ValueScale = pAxis2->getValueScaleByAxisVariable(
            inSlewScale, loadCapScale, relatedOutCapScale);
    TableAxis* pAxis3 = getAxis3();
    if (pAxis3 == nullptr)
        axis3ValueScale = 1.0f;
    else
        axis3ValueScale = pAxis3->getValueScaleByAxisVariable(
            inSlewScale, loadCapScale, relatedOutCapScale);
}

TableAxis* TimingTable3::getAxis3(void) {
    if (axis3_ != UNINIT_OBJECT_ID)
        return addr<TableAxis>(axis3_);
    else
        return nullptr;
}

OStreamBase& operator<<(OStreamBase& os, TimingTable3 const& rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");

    TimingTable3::BaseType const& base = rhs;
    os << base << DataDelimiter();

    os << DataFieldName("axis3_") << rhs.axis3_;
    os << DataBegin("[");
    if (rhs.axis3_ != UNINIT_OBJECT_ID) {
        TableAxis* t = Object::addr<TableAxis>(rhs.axis3_);
        if (t) os << *t;
    }
    os << DataEnd("]");

    os << DataEnd(")");
    return os;
}
}  // namespace db
}  // namespace open_edi
