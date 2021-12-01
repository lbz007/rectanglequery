/**
 * @file analysis_corner.h
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
#ifndef SRC_DB_TIMING_TIMINGLIB_ANALYSIS_CORNER_H_
#define SRC_DB_TIMING_TIMINGLIB_ANALYSIS_CORNER_H_

#include <algorithm>
#include <string>
#include <utility>
#include <unordered_map>

#include "db/core/object.h"
#include "db/core/pin.h"
#include "db/timing/spef/design_parasitics.h"
#include "db/timing/timinglib/timinglib_term.h"
#include "util/data_traits.h"

namespace open_edi {
namespace db {

class LibSet;
class DesignParasitics;
class AnalysisView;

class AnalysisCorner : public Object {
  public:
    using BaseType = Object;

    /// @brief default constructor
    AnalysisCorner();

    /// @brief destructor
    ~AnalysisCorner();

    /// @brief constructor
    AnalysisCorner(Object *owner, IndexType id);

    /// @brief copy constructor
    AnalysisCorner(AnalysisCorner const &rhs);

    /// @brief move constructor
    AnalysisCorner(AnalysisCorner &&rhs) noexcept;

    /// @brief copy assignment
    AnalysisCorner &operator=(AnalysisCorner const &rhs);

    /// @brief move assignment
    AnalysisCorner &operator=(AnalysisCorner &&rhs) noexcept;

    /// @brief summarize memory usage of the object in bytes
    IndexType memory() const;

    /// set
    void setName(const std::string &name);
    void setRcTechFile(const std::string &file);
    void setLibset(ObjectId id);
    void setDesignParasitics(ObjectId id);
    void addAnalysisView(AnalysisView *p);
    void setGroupId(ObjectId id);
    void buildTermTTermMap(void);


    /// get
    SymbolIndex getNameIndex(void);
    SymbolIndex getRcTechFileIndex(void);
    std::string getName(void) const;
    std::string getRcTechFile(void) const;

    LibSet *getLibset(void);
    DesignParasitics *getDesignParasitics(void);

    /// @brief get all analysis views
    std::vector<AnalysisView *> getAnalysisViews(void) const;

    /// @brief get the analysis view that contains this mode by id
    AnalysisView *getAnalysisView(ObjectId viewId) const;

    /// @brief get the analysis view that contains this mode by name
    AnalysisView *getAnalysisView(const std::string &viewName) const;
    ObjectId getGroupId(void);
    TTerm* getPinTTerm(Pin* pin) const;
    TTerm *getTTerm(Term *pTerm) const;

  protected:
    /// @brief copy object
    void copy(AnalysisCorner const &rhs);
    /// @brief move object
    void move(AnalysisCorner &&rhs);
    /// @brief overload output stream
    friend OStreamBase &operator<<(OStreamBase &os, AnalysisCorner const &rhs);

  private:
    std::unordered_map<Term*, TTerm*> term_tterm_map_;
    SymbolIndex name_;
    SymbolIndex rc_tech_file_;
    ObjectId libset_;
    ObjectId design_parasitics_;
    ObjectId view_ids_;
};

}  // namespace db
}  // namespace open_edi

#endif  // SRC_DB_TIMING_TIMINGLIB_ANALYSIS_CORNER_H_
