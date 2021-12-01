/**
 * @file  property.h
 * @date  08/21/2020 04:34:28 PM CST
 * @brief ""
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef EDI_DB_TECH_PROPERTY_H_
#define EDI_DB_TECH_PROPERTY_H_

#include "db/core/object.h"
#include "db/tech/type_def.h"
#include "db/util/array.h"
#include "util/util.h"
#include "util/io_manager.h"

namespace open_edi {
namespace db {

/**
 * @class Property
 * @brief for PROPERTY
 * [PROPERTY propName propVal ;]
 * */
class Property {
  public:
    using BaseType = Object;

    /// @brief default constructor
    Property();

    /// @brief destructor
    ~Property();

    /// @brief copy constructor
    Property(Property const &rhs);

    /// @brief move constructor
    Property(Property &&rhs) noexcept;

    /// @brief copy assignment
    Property &operator=(Property const &rhs);

    /// @brief move constructor
    Property &operator=(Property &&rhs) noexcept;
    /// @brief clone current object for extensibility.
    /// Users need to manage the pointer to avoid memory leakage.
    /// @return the pointer to the new object
    Property *clone() const;

    /// @brief summarize memory usage of the object in bytes
    UInt32 memory() const;

    // ACCESS
    // Get
    ObjectId getDefinitionId() const;
    const char *getPropName() const;
    PropDataType getDataType() const;
    bool getHasRange() const;
    int getIntRangeMin() const;
    int getIntRangeMax() const;
    double getRealRangeMin() const;
    double getRealRangeMax() const;
    int getIntValue() const;
    bool getBoolValue() const;
    void* getPointerValue() const;
    ObjectId getObjectIdValue() const;
    double getRealValue() const;
    std::string getStringValue() const;
    template<typename T>
    T getValue() const {
      return (T)value_u_;
    }
    // Set
    void setDefinitionId(const std::string &name);
    void setDefinitionId(ObjectId v);
    void setValue(int v);
    void setValue(bool v);
    void setValue(void* v);
    void setValue(ObjectId v);
    void setValue(double v);
    void setValue(const char* v);
    // Print
    void printLEF(IOManager &io_manager) const;
    void printDEF(IOManager &io_manager) const;

  protected:
    /// @brief copy object
    void copy(Property const &rhs);
    /// @brief move object
    void move(Property &&rhs);
    /// @brief overload output stream
    friend OStreamBase &operator<<(OStreamBase &os, Property const &rhs);
    /// @brief overload input stream
    // friend IStreamBase &operator>>(IStreamBase &is, Property &rhs) {}

  private:
    // DATA
    ObjectId definition_id_;
    int64_t value_u_;
};

}  // namespace db
}  // namespace open_edi

#endif
