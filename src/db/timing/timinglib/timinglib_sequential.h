/**
 * @file timinglib_sequential.h
 * @date 2021-06-26
 * @brief TTerm Class
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#ifndef SRC_DB_TIMING_TIMINGLIB_TIMINGLIB_SEQUENTIAL_H_
#define SRC_DB_TIMING_TIMINGLIB_TIMINGLIB_SEQUENTIAL_H_

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "db/core/object.h"
#include "db/timing/timinglib/timinglib_commondef.h"

namespace open_edi {
namespace db {

class TFunction;
class TTerm;
class TCell;

class TSequential : public Object {
  public:
    using BaseType = Object;

    /// @brief default constructor
    TSequential();

    /// @brief destructor
    ~TSequential();

    /// @brief constructor
    TSequential(Object *owner, IndexType id);

    /// @brief copy constructor
    TSequential(TSequential const &rhs);

    /// @brief move constructor
    TSequential(TSequential &&rhs) noexcept;

    /// @brief copy assignment
    TSequential &operator=(TSequential const &rhs);

    /// @brief move constructor
    TSequential &operator=(TSequential &&rhs) noexcept;

    /// @brief summarize memory usage of the object in bytes
    IndexType memory() const;

    void setLatch(bool b);
    void setRegister(bool b);
    void setClearPresetOut(TimingClearPreset c);
    void setClearPresetOutInv(TimingClearPreset c);
    TFunction* setClock(const std::string &str);
    TFunction* setData(const std::string &str);
    TFunction* setClear(const std::string &str);
    TFunction* setPreset(const std::string &str);
    void setOutput(ObjectId id);
    void setOutputInv(ObjectId id);
    void setGroupId(ObjectId id);

    bool isLatch();
    bool isRegister();
    TimingClearPreset getClearPresetOut();
    TimingClearPreset getClearPresetOutInv();
    TFunction* getClock();
    TFunction* getData();
    TFunction* getClear();
    TFunction* getPreset();
    TTerm* getOutput();
    TTerm* getOutputInv();
    ObjectId getGroupId(void) const;
    TCell* getTCell(void) const;

  protected:
    /// @brief copy object
    void copy(TSequential const &rhs);
    /// @brief move object
    void move(TSequential &&rhs);
    /// @brief overload output stream
    friend OStreamBase &operator<<(OStreamBase &os, TSequential const &rhs);

  private:
    bool is_register_;
    TimingClearPreset clr_preset_out_;
    TimingClearPreset clr_preset_out_inv_;
    ObjectId group_id_;
    ObjectId clock_;
    ObjectId data_;
    ObjectId clear_;
    ObjectId preset_;
    ObjectId output_;
    ObjectId output_inv_;
};

}  // namespace db
}  // namespace open_edi

#endif  // SRC_DB_TIMING_TIMINGLIB_TIMINGLIB_SEQUENTIAL_H_
