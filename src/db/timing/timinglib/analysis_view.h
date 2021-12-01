/**
 * @file analysis_view.h
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
#ifndef SRC_DB_TIMING_TIMINGLIB_ANALYSIS_VIEW_H_
#define SRC_DB_TIMING_TIMINGLIB_ANALYSIS_VIEW_H_

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "db/core/object.h"
#include "db/timing/sdc/sdc.h"
#include "db/timing/timinglib/analysis_corner.h"
#include "db/timing/timinglib/timinglib_function.h"
#include "util/data_traits.h"

namespace open_edi {
namespace timing {
class Vertex;
class Edge;
enum class ConstantSource : uint32_t;
enum class EdgeDisabledReason : uint32_t;
typedef uint64_t VertexId;
}  // namespace timing

namespace db {

class AnalysisMode;
class TimingArc;
class TLib;
class AnalysisView : public Object {
  public:
    using BaseType = Object;

    /// @brief default constructor
    AnalysisView();

    /// @brief destructor
    ~AnalysisView();

    /// @brief constructor
    AnalysisView(Object* owner, IndexType id);

    /// @brief copy constructor
    AnalysisView(AnalysisView const& rhs);

    /// @brief move constructor
    AnalysisView(AnalysisView&& rhs) noexcept;

    /// @brief copy assignment
    AnalysisView& operator=(AnalysisView const& rhs);

    /// @brief move assignment
    AnalysisView& operator=(AnalysisView&& rhs) noexcept;

    /// @brief summarize memory usage of the object in bytes
    IndexType memory() const;

    /// set
    void setName(const std::string& name);

    void setAnalysisCorner(ObjectId id);
    void setAnalysisMode(ObjectId id);
    void setActive(bool b);
    void setSetup(bool b);
    void setHold(bool b);
    void setMaxCap(bool b);
    void setMaxTran(bool b);
    void setMinCap(bool b);
    void setLeakagePower(bool b);
    void setDynamicPower(bool b);
    void setCellEm(bool b);
    void setSignalEm(bool b);
    void setGroupId(ObjectId id);
    inline void setVertexConstant(open_edi::timing::Vertex* v,
                                  TFuncConstant c) {
        vertex_constants_[v] = c;
    }
    inline void setVertexConstantSource(open_edi::timing::Vertex* v,
                                        open_edi::timing::ConstantSource s) {
        vertex_constant_source_[v] = s;
    }
    inline void setEdgeDisabledReason(open_edi::timing::Edge* e,
                                      open_edi::timing::EdgeDisabledReason r) {
        edge_disabled_reason_[e] = r;
    }
    inline void setLevelized(bool l) { levelized_ = l; }
    void setVertexLevel(open_edi::timing::VertexId vertex_id, int level);
    void setVertexInEdges(std::vector<unsigned int>& vertex_in_edges);
    void setVertexOutEdges(std::vector<unsigned int>& vertex_out_edges);
    void setSdc(const SdcPtr& sdc) { sdc_ = sdc; }

    void createSdc() { sdc_ = std::make_shared<Sdc>(this->getId()); }

    /// get
    SymbolIndex getNameIndex(void);
    std::string getName(void) const;

    AnalysisCorner* getAnalysisCorner(void);
    AnalysisMode* getAnalysisMode(void);
    ObjectId getGroupId(void);
    bool isActive(void);
    bool isSetup(void);
    bool isHold(void);
    bool isMaxCap(void);
    bool isMaxTran(void);
    bool isMinCap(void);
    bool isLeakagePower(void);
    bool isDynamicPower(void);
    bool isCellEm(void);
    bool isSignalEm(void);
    bool forwardPropNeedLock(open_edi::timing::VertexId vertex_id) const;
    bool backwardPropNeedLock(open_edi::timing::VertexId vertex_id) const;

    inline TTerm* getPinTTerm(Pin* pin) {
        AnalysisCorner* pCorner = getAnalysisCorner();
        if (pCorner != nullptr)
            return pCorner->getPinTTerm(pin);
        else
            return nullptr;
    }

    TFuncConstant getVertexConstant(open_edi::timing::Vertex* v) const;
    open_edi::timing::ConstantSource getVertexConstantSource(
        open_edi::timing::Vertex* v) const;
    void getConstantVertices(
        std::vector<open_edi::timing::Vertex*>& vertices) const;
    int getVertexLevel(open_edi::timing::VertexId vertex_id);
    inline bool isLevelized() { return levelized_; }
    std::vector<unsigned int>& getVertexInEdges() { return vertex_in_edges_; }
    std::vector<unsigned int>& getVertexOutEdges() { return vertex_out_edges_; }
    std::vector<std::vector<open_edi::timing::Vertex*>>& getLevelQueue() {
        return level_queue_;
    }
    std::vector<open_edi::timing::Vertex*>& getVerticesByLevel(int level) {
        return level_queue_[level];
    }
    SdcPtr& getSdc(void) { return sdc_; }

    /// The unit of the inSlew is s
    /// The unit of the totalCap is F
    /// The unit of the return value is Ohm
    double getDriverCellRd(
        TTerm* outTerm, TTerm* inTerm, double inSlew, double totalCap,
        TimingRiseFall dir = TimingRiseFall::kRise_Fall);
    /// The unit of the return value is s
    double getDriverCellDelay(
        TTerm* outTerm, TTerm* inTerm, double inSlew, double totalCap,
        TimingRiseFall dir = TimingRiseFall::kRise_Fall);
    double getDriverCellIntrinsicDelay(
        TTerm* outTerm, TTerm* inTerm, double inSlew,
        TimingRiseFall dir = TimingRiseFall::kRise_Fall);
    /// The unit of the return value is F
    float getPinCapacitance(Pin* pin);
    float getPinCapacitance(TTerm* pTerm);

    /// The unit if the return value is F
    float getPinMaxCapacitance(TTerm* pTerm);

    /// The unit of the return value is s
    float getPinMaxTransition(TTerm* pTerm);

    /// The unit of the return value is s
    double getDriverCellTransition(
        TTerm* outTerm, TTerm* inTerm, double inSlew, double totalCap,
        TimingRiseFall dir = TimingRiseFall::kRise_Fall);

    bool isTriStateEnable(TTerm* outTerm, TTerm* inTerm);

    TTerm* getTTerm(Term* pTerm);

    void getTimingArcs(TTerm* outTerm, TTerm* inTerm,
                       std::vector<TimingArc*>* arcs);
    TUnits* getTUnitsByTTerm(TTerm* pTerm);

  protected:
    /// @brief copy object
    void copy(AnalysisView const& rhs);
    /// @brief move object
    void move(AnalysisView&& rhs);
    /// @brief overload output stream
    friend OStreamBase& operator<<(OStreamBase& os, AnalysisView const& rhs);

  private:
    TLib* getTLibByTTerm(TTerm* pTerm);

    /// type == 1 : cell delay
    /// type == 2 : cell transition
    double getDriverCellDelayByTable(
        TTerm* outTerm, TTerm* inTerm, double inSlew, double totalCap,
        int type = 1, TimingRiseFall dir = TimingRiseFall::kRise_Fall);

  private:
    std::unordered_map<open_edi::timing::Vertex*, TFuncConstant>
        vertex_constants_;
    std::unordered_map<open_edi::timing::Vertex*,
                       open_edi::timing::ConstantSource>
        vertex_constant_source_;
    std::unordered_map<open_edi::timing::Edge*,
                       open_edi::timing::EdgeDisabledReason>
        edge_disabled_reason_;
    std::vector<std::vector<open_edi::timing::Vertex*>> level_queue_;
    std::vector<int> vertex_levels_;
    std::vector<unsigned int> vertex_in_edges_;
    std::vector<unsigned int> vertex_out_edges_;
    SdcPtr sdc_;
    bool is_active_;
    bool is_setup_;
    bool is_hold_;
    bool is_max_cap_;
    bool is_max_tran_;
    bool is_min_cap_;
    bool is_leakage_power_;
    bool is_dynamic_power_;
    bool is_cell_em_;
    bool is_signal_em_;
    bool levelized_;
    SymbolIndex name_;
    ObjectId analysis_corner_;
    ObjectId analysis_mode_;
    ObjectId group_id_;
};

}  // namespace db
}  // namespace open_edi

#endif  // SRC_DB_TIMING_TIMINGLIB_ANALYSIS_VIEW_H_
