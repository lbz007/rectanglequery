/**
 * @file timinglib_sequential.cpp
 * @date 2021-06-26
 * @brief TSequential Class
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#include "db/timing/timinglib/timinglib_sequential.h"

#include <unordered_set>

#include "db/core/db.h"
#include "db/timing/timinglib/timinglib_function.h"
#include "db/timing/timinglib/timinglib_term.h"

namespace open_edi {
namespace db {

TSequential::TSequential()
    : TSequential::BaseType(),
      is_register_(false),
      clr_preset_out_(TimingClearPreset::kUnknown),
      clr_preset_out_inv_(TimingClearPreset::kUnknown),
      group_id_(UNINIT_OBJECT_ID),
      clock_(UNINIT_OBJECT_ID),
      data_(UNINIT_OBJECT_ID),
      clear_(UNINIT_OBJECT_ID),
      preset_(UNINIT_OBJECT_ID),
      output_(UNINIT_OBJECT_ID),
      output_inv_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTimingSequential);
}

TSequential::~TSequential() {}

TSequential::TSequential(Object *owner, TSequential::IndexType id)
    : TSequential::BaseType(owner, id),
      is_register_(false),
      clr_preset_out_(TimingClearPreset::kUnknown),
      clr_preset_out_inv_(TimingClearPreset::kUnknown),
      group_id_(UNINIT_OBJECT_ID),
      clock_(UNINIT_OBJECT_ID),
      data_(UNINIT_OBJECT_ID),
      clear_(UNINIT_OBJECT_ID),
      preset_(UNINIT_OBJECT_ID),
      output_(UNINIT_OBJECT_ID),
      output_inv_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTimingSequential);
}

TSequential::TSequential(TSequential const &rhs) { copy(rhs); }

TSequential::TSequential(TSequential &&rhs) noexcept { move(std::move(rhs)); }

TSequential &TSequential::operator=(TSequential const &rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

TSequential &TSequential::operator=(TSequential &&rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

TSequential::IndexType TSequential::memory() const {
    IndexType ret = this->BaseType::memory();

    ret += sizeof(is_register_);
    ret += sizeof(clr_preset_out_);
    ret += sizeof(clr_preset_out_inv_);
    ret += sizeof(group_id_);
    ret += sizeof(clock_);
    ret += sizeof(data_);
    ret += sizeof(clear_);
    ret += sizeof(preset_);
    ret += sizeof(output_);
    ret += sizeof(output_inv_);

    return ret;
}

void TSequential::setLatch(bool b) { is_register_ = !b; }
void TSequential::setRegister(bool b) { is_register_ = b; }
void TSequential::setClearPresetOut(TimingClearPreset c) {
    clr_preset_out_ = c;
}
void TSequential::setClearPresetOutInv(TimingClearPreset c) {
    clr_preset_out_inv_ = c;
}
TFunction* TSequential::setClock(const std::string &str) {
    Timing *timing_lib = getTimingLib();
    if (timing_lib) {
        auto p = Object::createObject<TFunction>(kObjectTypeTFunction,
                                                 timing_lib->getId());
        if (p) {
            clock_ = p->getId();
            p->setFuncStr(str);
            p->setGroupId(this->getId());
        }
        return p;
    }
    return nullptr;
}
TFunction* TSequential::setData(const std::string &str) {
    Timing *timing_lib = getTimingLib();
    if (timing_lib) {
        auto p = Object::createObject<TFunction>(kObjectTypeTFunction,
                                                 timing_lib->getId());
        if (p) {
            data_ = p->getId();
            p->setFuncStr(str);
            p->setGroupId(this->getId());
        }
        return p;
    }
    return nullptr;
}
TFunction* TSequential::setClear(const std::string &str) {
    Timing *timing_lib = getTimingLib();
    if (timing_lib) {
        auto p = Object::createObject<TFunction>(kObjectTypeTFunction,
                                                 timing_lib->getId());
        if (p) {
            clear_ = p->getId();
            p->setFuncStr(str);
            p->setGroupId(this->getId());
        }
        return p;
    }
    return nullptr;
}
TFunction* TSequential::setPreset(const std::string &str) {
    Timing *timing_lib = getTimingLib();
    if (timing_lib) {
        auto p = Object::createObject<TFunction>(kObjectTypeTFunction,
                                                 timing_lib->getId());
        if (p) {
            preset_ = p->getId();
            p->setFuncStr(str);
            p->setGroupId(this->getId());
        }
        return p;
    }
    return nullptr;
}
void TSequential::setOutput(ObjectId id) { output_ = id; }
void TSequential::setOutputInv(ObjectId id) { output_inv_ = id; }
void TSequential::setGroupId(ObjectId id) { group_id_ = id; }

bool TSequential::isLatch() { return !is_register_; }
bool TSequential::isRegister() { return is_register_; }
TimingClearPreset TSequential::getClearPresetOut() { return clr_preset_out_; }
TimingClearPreset TSequential::getClearPresetOutInv() {
    return clr_preset_out_inv_;
}
TFunction *TSequential::getClock() { return Object::addr<TFunction>(clock_); }
TFunction *TSequential::getData() { return Object::addr<TFunction>(data_); }
TFunction *TSequential::getClear() { return Object::addr<TFunction>(clear_); }
TFunction *TSequential::getPreset() { return Object::addr<TFunction>(preset_); }
TTerm *TSequential::getOutput() { return Object::addr<TTerm>(output_); }
TTerm *TSequential::getOutputInv() { return Object::addr<TTerm>(output_inv_); }
ObjectId TSequential::getGroupId(void) const { return group_id_; }

TCell *TSequential::getTCell(void) const {
    return Object::addr<TCell>(group_id_);
}

void TSequential::copy(TSequential const &rhs) {
    this->BaseType::copy(rhs);

    is_register_ = rhs.is_register_;
    clr_preset_out_ = rhs.clr_preset_out_;
    clr_preset_out_inv_ = rhs.clr_preset_out_inv_;
    group_id_ = rhs.group_id_;
    clock_ = rhs.clock_;
    data_ = rhs.data_;
    clear_ = rhs.clear_;
    preset_ = rhs.preset_;
    output_ = rhs.output_;
    output_inv_ = rhs.output_inv_;
}

void TSequential::move(TSequential &&rhs) {
    this->BaseType::move(std::move(rhs));

    is_register_ = std::move(rhs.is_register_);
    clr_preset_out_ = std::move(rhs.clr_preset_out_);
    clr_preset_out_inv_ = std::move(rhs.clr_preset_out_inv_);
    group_id_ = std::move(rhs.group_id_);
    clock_ = std::move(rhs.clock_);
    data_ = std::move(rhs.data_);
    clear_ = std::move(rhs.clear_);
    preset_ = std::move(rhs.preset_);
    output_ = std::move(rhs.output_);
    output_inv_ = std::move(rhs.output_inv_);
}

OStreamBase &operator<<(OStreamBase &os, TSequential const &rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");

    TSequential::BaseType const &base = rhs;
    os << base << DataDelimiter();

    os << DataFieldName("is_register_") << rhs.is_register_ << DataDelimiter();
    os << DataFieldName("clr_preset_out_") << rhs.clr_preset_out_
       << DataDelimiter();
    os << DataFieldName("clr_preset_out_inv_") << rhs.clr_preset_out_inv_
       << DataDelimiter();
    os << DataFieldName("group_id_") << rhs.group_id_ << DataDelimiter();
    os << DataFieldName("clock_") << rhs.clock_;
    {
        os << DataBegin("[");
        if (rhs.clock_ != UNINIT_OBJECT_ID) {
            auto p = Object::addr<TFunction>(rhs.clock_);
            if (p) os << *p;
        }
        os << DataEnd("]") << DataDelimiter();
    }
    os << DataFieldName("data_") << rhs.data_;
    {
        os << DataBegin("[");
        if (rhs.data_ != UNINIT_OBJECT_ID) {
            auto p = Object::addr<TFunction>(rhs.data_);
            if (p) os << *p;
        }
        os << DataEnd("]") << DataDelimiter();
    }
    os << DataFieldName("clear_") << rhs.clear_;
    {
        os << DataBegin("[");
        if (rhs.clear_ != UNINIT_OBJECT_ID) {
            auto p = Object::addr<TFunction>(rhs.clear_);
            if (p) os << *p;
        }
        os << DataEnd("]") << DataDelimiter();
    }
    os << DataFieldName("preset_") << rhs.preset_;
    {
        os << DataBegin("[");
        if (rhs.preset_ != UNINIT_OBJECT_ID) {
            auto p = Object::addr<TFunction>(rhs.preset_);
            if (p) os << *p;
        }
        os << DataEnd("]") << DataDelimiter();
    }
    os << DataFieldName("output_") << rhs.output_ << DataDelimiter();
    os << DataFieldName("output_inv_") << rhs.output_inv_ << DataDelimiter();

    os << DataEnd(")");
    return os;
}
}  // namespace db
}  // namespace open_edi
