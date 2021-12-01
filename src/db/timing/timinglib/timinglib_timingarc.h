/**
 * @file timinglib_timingarc.h
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
#ifndef SRC_DB_TIMING_TIMINGLIB_TIMINGLIB_TIMINGARC_H_
#define SRC_DB_TIMING_TIMINGLIB_TIMINGLIB_TIMINGARC_H_

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "db/core/object.h"
#include "db/timing/timinglib/timinglib_commondef.h"

namespace open_edi {
namespace db {

class TTerm;
class TFunction;
class TimingTable;

class TimingArcData : public Object {
  public:
    using BaseType = Object;

    /// @brief default constructor
    TimingArcData();

    /// @brief destructor
    ~TimingArcData();

    /// @brief constructor
    TimingArcData(Object* owner, IndexType id);

    /// @brief copy constructor
    TimingArcData(TimingArcData const& rhs);

    /// @brief move constructor
    TimingArcData(TimingArcData&& rhs) noexcept;

    /// @brief copy assignment
    TimingArcData& operator=(TimingArcData const& rhs);

    /// @brief move assignment
    TimingArcData& operator=(TimingArcData&& rhs) noexcept;

    /// @brief summarize memory usage of the object in bytes
    IndexType memory() const;

    /// set
    void setTimingSense(TimingSense ts);
    void setTimingType(TimingType tt);
    TFunction* setWhen(const std::string& str);
    void setCellRise(ObjectId id);
    void setCellFall(ObjectId id);
    void setRiseTransition(ObjectId id);
    void setFallTransition(ObjectId id);
    void setRiseConstraint(ObjectId id);
    void setFallConstraint(ObjectId id);
    void setGroupId(ObjectId id);

    /// get
    TimingSense getTimingSense(void) const;
    TimingType getTimingType(void) const;
    TFunction* getWhen(void) const;
    TimingTable* getCellRise(void) const;
    TimingTable* getCellFall(void) const;
    TimingTable* getRiseTransition(void) const;
    TimingTable* getFallTransition(void) const;
    TimingTable* getRiseConstraint(void) const;
    TimingTable* getFallConstraint(void) const;
    ObjectId getGroupId(void) const;

  protected:
    /// @brief copy object
    void copy(TimingArcData const& rhs);
    /// @brief move object
    void move(TimingArcData&& rhs);
    /// @brief overload output stream
    friend OStreamBase& operator<<(OStreamBase& os, TimingArcData const& rhs);

  private:
    TimingSense timing_sense_;
    TimingType timing_type_;
    ObjectId when_;
    ObjectId cell_rise_;
    ObjectId cell_fall_;
    ObjectId rise_transition_;
    ObjectId fall_transition_;
    ObjectId rise_constraint_;
    ObjectId fall_constraint_;
    ObjectId group_id_;
};

class TimingArc : public Object {
  public:
    using BaseType = Object;

    /// @brief default constructor
    TimingArc();

    /// @brief destructor
    ~TimingArc();

    /// @brief constructor
    TimingArc(Object* owner, IndexType id);

    /// @brief copy constructor
    TimingArc(TimingArc const& rhs);

    /// @brief move constructor
    TimingArc(TimingArc&& rhs) noexcept;

    /// @brief copy assignment
    TimingArc& operator=(TimingArc const& rhs);

    /// @brief move assignment
    TimingArc& operator=(TimingArc&& rhs) noexcept;

    /// @brief summarize memory usage of the object in bytes
    IndexType memory() const;

    /// set
    void setRelatedPin(ObjectId id);
    void setGroupId(ObjectId id);
    TimingArcData* createData(void);
    void setDisabled(void);

    /// set
    void setTimingSense(TimingSense ts);
    void setTimingType(TimingType tt);
    TFunction* setWhen(const std::string& str);
    void setCellRise(ObjectId id);
    void setCellFall(ObjectId id);
    void setRiseTransition(ObjectId id);
    void setFallTransition(ObjectId id);
    void setRiseConstraint(ObjectId id);
    void setFallConstraint(ObjectId id);
    static TimingTable* createTimingTable(ObjectType type);

    /// get
    TTerm* getRelatedPin(void) const;
    ObjectId getGroupId(void) const;
    TimingArcData* getData(void) const;
    bool isDisabled(void) const;

    /// get
    TimingSense getTimingSense(void) const;
    TimingType getTimingType(void) const;
    TFunction* getWhen(void) const;
    TimingTable* getCellRise(void) const;
    TimingTable* getCellFall(void) const;
    TimingTable* getRiseTransition(void) const;
    TimingTable* getFallTransition(void) const;
    TimingTable* getRiseConstraint(void) const;
    TimingTable* getFallConstraint(void) const;

  protected:
    /// @brief copy object
    void copy(TimingArc const& rhs);
    /// @brief move object
    void move(TimingArc&& rhs);
    /// @brief overload output stream
    friend OStreamBase& operator<<(OStreamBase& os, TimingArc const& rhs);

  private:
    ObjectId related_pin_;
    ObjectId group_id_;
    ObjectId data_;
    bool is_disabled_;
};

}  // namespace db
}  // namespace open_edi

#endif  // SRC_DB_TIMING_TIMINGLIB_TIMINGLIB_TIMINGARC_H_
