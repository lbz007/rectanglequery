/**
 * @file  property.cpp
 * @date  08/21/2020 04:33:40 PM CST
 * @brief ""
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#include "db/tech/property.h"

#include "db/core/db.h"
#include "db/util/property_definition.h"
#include "db/util/symbol_table.h"
#include "util/util.h"

namespace open_edi {
namespace db {

Property::Property() : definition_id_(0), value_u_(0) {
}

Property::~Property() {
    PropDataType data_type = getDataType();
    if (data_type == PropDataType::kString ) {
        free((char*)value_u_);
    }
}

Property::Property(Property const &rhs) { copy(rhs); }

Property::Property(Property &&rhs) noexcept { move(std::move(rhs)); }

Property &Property::operator=(Property const &rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

Property &Property::operator=(Property &&rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

UInt32 Property::memory() const {
    UInt32 ret = 0;
    ret += sizeof(definition_id_);
    ret += sizeof(value_u_);
    return ret;
}

void Property::copy(Property const &rhs) {
    definition_id_ = rhs.definition_id_;
    PropDataType data_type = getDataType();
    if (data_type == PropDataType::kString) {
       std::string tmp = rhs.getStringValue();
       char* string_value_ = (char* )malloc(tmp.size() + 1);
       std::memcpy(string_value_, tmp.data(), tmp.size() + 1);
       value_u_ = (int64_t)string_value_;
    } else {
        value_u_ = rhs.value_u_;
    }
}

void Property::move(Property &&rhs) {
    definition_id_ = std::move(rhs.definition_id_);
    value_u_ = std::move(rhs.value_u_);
}

// Get
ObjectId Property::getDefinitionId() const { return definition_id_; }

const char *Property::getPropName() const {
    if (definition_id_) {
        PropertyDefinition *definition_obj =
            Object::addr<PropertyDefinition>(definition_id_);
        ediAssert(definition_obj != nullptr);
        return definition_obj->getPropName().c_str();
    }
    return "invalid_property";
}

PropDataType Property::getDataType() const {
    PropDataType ret = PropDataType::kUnknown;
    if (definition_id_) {
        PropertyDefinition *definition_obj =
            Object::addr<PropertyDefinition>(definition_id_);
        ediAssert(definition_obj != nullptr);
        ret = definition_obj->getDataType();
    }
    return ret;
}

bool Property::getHasRange() const {
    bool ret = false;
    if (definition_id_) {
        PropertyDefinition *definition_obj =
            Object::addr<PropertyDefinition>(definition_id_);
        ediAssert(definition_obj != nullptr);
        ret = definition_obj->getHasRange();
    }
    return ret;
}

int Property::getIntRangeMin() const {
    int ret = INT_MIN;
    if (definition_id_) {
        PropertyDefinition *definition_obj =
            Object::addr<PropertyDefinition>(definition_id_);
        ediAssert(definition_obj != nullptr);
        ret = definition_obj->getIntRangeMin();
    }
    return ret;
}

int Property::getIntRangeMax() const {
    int ret = INT_MAX;
    if (definition_id_) {
        PropertyDefinition *definition_obj =
            Object::addr<PropertyDefinition>(definition_id_);
        ediAssert(definition_obj != nullptr);
        ret = definition_obj->getIntRangeMax();
    }
    return ret;
}

double Property::getRealRangeMin() const {
    double ret = std::numeric_limits<double>::lowest();
    if (definition_id_) {
        PropertyDefinition *definition_obj =
            Object::addr<PropertyDefinition>(definition_id_);
        ediAssert(definition_obj != nullptr);
        ret = definition_obj->getRealRangeMin();
    }
    return ret;
}

double Property::getRealRangeMax() const {
    double ret = std::numeric_limits<double>::max();
    if (definition_id_) {
        PropertyDefinition *definition_obj =
            Object::addr<PropertyDefinition>(definition_id_);
        ediAssert(definition_obj != nullptr);
        ret = definition_obj->getRealRangeMax();
    }
    return ret;
}

int Property::getIntValue() const { return (int)value_u_; }

double Property::getRealValue() const { return *(double*)&value_u_; }

bool Property::getBoolValue() const { return (bool)value_u_;}

void* Property::getPointerValue() const { return (void*)value_u_;}

ObjectId Property::getObjectIdValue() const { return (ObjectId)value_u_;}

std::string Property::getStringValue() const {
    std::string value = (char*)value_u_;
    return value;
}

// Set
void Property::setDefinitionId( const std::string &name) {
    ObjectId prop_def =
        getTechLib()->getPropertyDefinitionId(name.c_str());
    // TODO: warning if the object id is invalid.
    definition_id_ = prop_def;
}

void Property::setDefinitionId(ObjectId v) { definition_id_ = v; }

void Property::setValue(int v) {
    PropDataType data_type = getDataType();
    if (data_type == PropDataType::kInt) {
        bool has_range = getHasRange();
        if (has_range) {
            double min = getIntRangeMin();
            double max = getIntRangeMax();
            if (v > max || v < min) {
                // warning message: out of range.
                return;
            }
        }
        value_u_ = (int64_t)v;
    }
    // warning message: data type mis-match.
}

void Property::setValue(double v) {
    PropDataType data_type = getDataType();
    if (data_type == PropDataType::kReal) {
        bool has_range = getHasRange();
        if (has_range) {
            double min = getRealRangeMin();
            double max = getRealRangeMax();
            if (v > max || v < min) {
                // warning message: out of range.
                return;
            }
        }
        value_u_ = *(int64_t*)&v;
    }
    // warning message: data type mis-match.
}

void Property::setValue(bool v) {
    PropDataType data_type = getDataType();
    if (data_type == PropDataType::kBool) {
        value_u_ = (int64_t)v;
    }
}

void Property::setValue(void* v) {
    PropDataType data_type = getDataType();
    if (data_type == PropDataType::kPointer) {
        value_u_ = (int64_t)v;
    }
}

void Property::setValue(ObjectId v) {
    PropDataType data_type = getDataType();
    if (data_type == PropDataType::kObjectId) {
        value_u_ = (int64_t)v;
    }
}

void Property::setValue(const char* v) {
    if (v == nullptr)
        return;
    PropDataType data_type = getDataType();
    if (data_type == PropDataType::kString) {
        if (value_u_ != 0) {
            free((char*)value_u_);
        }
        std::string tmp = v;
        char* string_value_ = (char* )malloc(tmp.size() + 1);
        std::memcpy(string_value_, tmp.data(), tmp.size() + 1);
        value_u_ = (int64_t)string_value_;
    }
}

void Property::printLEF(IOManager &io_manager) const {
    PropDataType data_type = getDataType();
    io_manager.write("  PROPERTY %s", getPropName());
    switch (data_type) {
        case (PropDataType::kInt):
            io_manager.write(" %d", getIntValue());
            break;
        case (PropDataType::kReal):
            io_manager.write(" %g", getRealValue());
            break;
        case (PropDataType::kString):
            io_manager.write(" \"%s\"", getStringValue().c_str());
            break;
    }
    io_manager.write(";\n");
}

void Property::printDEF(IOManager &io_manager) const {
    io_manager.write("  + PROPERTY %s ", getPropName());

    PropDataType data_type = getDataType();
    switch (data_type) {
        case (PropDataType::kInt):
            io_manager.write("%d", getIntValue());
            break;
        case (PropDataType::kReal): 
            io_manager.write("%g", getRealValue());
            break;
        case (PropDataType::kString):
            io_manager.write("\"%s\"", getStringValue().c_str());
            break;
    }
    io_manager.write("\n");
}

OStreamBase &operator<<(OStreamBase &os, Property const &rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");
    // os << DataFieldName("prop_type_") << toString(rhs.getPropType())
    //    << DataDelimiter();
    os << DataFieldName("prop_name_") << rhs.getPropName() << DataDelimiter();
    os << DataFieldName("data_type_") << toString(rhs.getDataType())
       << DataDelimiter();

    if (rhs.getHasRange()) {
        os << DataFieldName("has_range_") << "true" << DataDelimiter();
        switch (rhs.getDataType()) {
            case PropDataType::kInt:
                os << DataFieldName("range_u_") << DataDelimiter("[")
                   << rhs.getIntRangeMin() << DataDelimiter(" ")
                   << rhs.getIntRangeMax() << DataDelimiter("]")
                   << DataDelimiter();
                break;
            case PropDataType::kReal:
                os << DataFieldName("range_u_") << DataDelimiter("[")
                   << rhs.getRealRangeMin() << DataDelimiter(" ")
                   << rhs.getRealRangeMax() << DataDelimiter("]")
                   << DataDelimiter();
                break;
            default:
                break;
        }
    } else {
        os << DataFieldName("has_range_") << "false" << DataDelimiter();
    }
    switch (rhs.getDataType()) {
        case PropDataType::kString:
            os << DataFieldName("value_u_") << rhs.getStringValue();
            break;
        case PropDataType::kInt:
            os << DataFieldName("value_u_") << rhs.getIntValue();
            break;
        case PropDataType::kReal:
            os << DataFieldName("value_u_") << rhs.getRealValue();
            break;
        default:
            break;
    }
    os << DataEnd(")");
    return os;
}

}  // namespace db
}  // namespace open_edi
