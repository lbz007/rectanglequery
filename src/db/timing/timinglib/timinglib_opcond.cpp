/**
 * @file timinglib_opcond.cpp
 * @date 2020-08-26
 * @brief OperatingConditions Class
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#include "db/timing/timinglib/timinglib_opcond.h"

#include "db/core/db.h"

namespace open_edi {
namespace db {

OperatingConditions::OperatingConditions()
    : OperatingConditions::BaseType(),
      wire_load_tree_(WireLoadTree::kUnknown),
      name_(0),
      group_id_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeOperatingConditions);
}

OperatingConditions::~OperatingConditions() {}

OperatingConditions::OperatingConditions(Object* owner,
                                         OperatingConditions::IndexType id)
    : OperatingConditions::BaseType(owner, id),
      wire_load_tree_(WireLoadTree::kUnknown),
      name_(0),
      group_id_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeOperatingConditions);
}

OperatingConditions::OperatingConditions(OperatingConditions const& rhs) {
    copy(rhs);
}

OperatingConditions::OperatingConditions(OperatingConditions&& rhs) noexcept {
    move(std::move(rhs));
}

OperatingConditions& OperatingConditions::operator=(
    OperatingConditions const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

OperatingConditions& OperatingConditions::operator=(
    OperatingConditions&& rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

void OperatingConditions::print(std::ostream& stream) {
    stream << "OperatingConditions: name: " << getName()
           << " wire_load_tree: " << toString(wire_load_tree_);
    this->BaseType::print(stream);
}

void OperatingConditions::copy(OperatingConditions const& rhs) {
    this->BaseType::copy(rhs);
    name_ = rhs.name_;
    wire_load_tree_ = rhs.wire_load_tree_;
    group_id_ = rhs.group_id_;
}

void OperatingConditions::move(OperatingConditions&& rhs) {
    this->BaseType::move(std::move(rhs));
    name_ = std::move(rhs.name_);
    wire_load_tree_ = std::move(rhs.wire_load_tree_);
    group_id_ = std::move(rhs.group_id_);
}

OperatingConditions::IndexType OperatingConditions::memory() const {
    IndexType ret = this->BaseType::memory();

    ret += sizeof(name_);
    ret += sizeof(wire_load_tree_);
    ret += sizeof(group_id_);

    return ret;
}

void OperatingConditions::setName(const std::string& name) {
    Timing* timing_lib = getTimingLib();
    if (timing_lib) {
        SymbolIndex index = timing_lib->getOrCreateSymbol(name.c_str());
        if (index != kInvalidSymbolIndex) {
            name_ = index;
            timing_lib->addSymbolReference(name_, this->getId());
        }
    }
}
void OperatingConditions::setWireLoadTree(WireLoadTree ty) {
    wire_load_tree_ = ty;
}
void OperatingConditions::setGroupId(ObjectId id) { group_id_ = id; }
std::string OperatingConditions::getName(void) const {
    Timing* timing_lib = getTimingLib();
    if (timing_lib) return timing_lib->getSymbolByIndex(name_);
    return "";
}
SymbolIndex OperatingConditions::getNameIndex(void) { return name_; }
WireLoadTree OperatingConditions::getWireLoadTree(void) {
    return wire_load_tree_;
}
ObjectId OperatingConditions::getGroupId(void) { return group_id_; }

OStreamBase& operator<<(OStreamBase& os, OperatingConditions const& rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");

    OperatingConditions::BaseType const& base = rhs;
    os << base << DataDelimiter();

    os << DataFieldName("name_") << rhs.getName() << DataDelimiter();
    os << DataFieldName("wire_load_tree_") << rhs.wire_load_tree_
       << DataDelimiter();
    os << DataFieldName("group_id_") << rhs.group_id_;

    os << DataEnd(")");
    return os;
}
}  // namespace db
}  // namespace open_edi
