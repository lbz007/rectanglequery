
/**
 * @file analysis_mode.h
 * @date 2020-09-08
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
#ifndef SRC_DB_TIMING_TIMINGLIB_ANALYSIS_MODE_H_
#define SRC_DB_TIMING_TIMINGLIB_ANALYSIS_MODE_H_

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "db/core/object.h"
#include "db/util/array.h"
#include "util/data_traits.h"

namespace open_edi {
namespace db {

class AnalysisView;

class AnalysisMode : public Object {
  public:
    using BaseType = Object;

    /// @brief default constructor
    AnalysisMode();

    /// @brief destructor
    ~AnalysisMode();

    /// @brief constructor
    AnalysisMode(Object *owner, IndexType id);

    /// @brief copy constructor
    AnalysisMode(AnalysisMode const &rhs);

    /// @brief move constructor
    AnalysisMode(AnalysisMode &&rhs) noexcept;

    /// @brief copy assignment
    AnalysisMode &operator=(AnalysisMode const &rhs);

    /// @brief move assignment
    AnalysisMode &operator=(AnalysisMode &&rhs) noexcept;

    /// @brief summarize memory usage of the object in bytes
    IndexType memory() const;

    /// set
    void setName(const std::string &name);
    void addConstraintFile(const std::string &file);
    void addAnalysisView(AnalysisView *p);

    /// get
    SymbolIndex getNameIndex(void) const;
    std::string getName(void) const;
    std::vector<std::string> getConstraintFiles(void) const;
    std::string getConstraintFile(SymbolIndex index) const;

    /// @brief get all analysis views
    std::vector<AnalysisView *> getAnalysisViews(void) const;

    /// @brief get the analysis view that contains this mode by id
    AnalysisView *getAnalysisView(ObjectId viewId) const;

    /// @brief get the analysis view that contains this mode by name
    AnalysisView *getAnalysisView(const std::string &viewName) const;

    int numContraintFiles(void) const;

  protected:
    /// @brief copy object
    void copy(AnalysisMode const &rhs);
    /// @brief move object
    void move(AnalysisMode &&rhs);
    /// @brief overload output stream
    friend OStreamBase &operator<<(OStreamBase &os, AnalysisMode const &rhs);

  private:
    SymbolIndex name_;
    ObjectId constraint_files_;
    ObjectId view_ids_;
};
}  // namespace db
}  // namespace open_edi

#endif  // SRC_DB_TIMING_TIMINGLIB_ANALYSIS_MODE_H_
