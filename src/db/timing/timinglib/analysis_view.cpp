/**
 * @file analysis_view.cpp
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
#include "db/timing/timinglib/analysis_view.h"

#include <limits>
#include <unordered_set>
#include <vector>

#include "db/core/db.h"
#include "db/timing/timinglib/analysis_corner.h"
#include "db/timing/timinglib/analysis_mode.h"
#include "db/timing/timinglib/timinglib_cell.h"
#include "db/timing/timinglib/timinglib_lib.h"
#include "db/timing/timinglib/timinglib_timingarc.h"
#include "db/timing/timinglib/timinglib_timingtable.h"
#include "db/timing/timinglib/timinglib_units.h"

namespace open_edi {
namespace db {

using namespace open_edi::timing;

AnalysisView::AnalysisView()
    : AnalysisView::BaseType(),
      sdc_(nullptr),
      is_active_(false),
      is_setup_(false),
      is_hold_(false),
      is_max_cap_(false),
      is_max_tran_(false),
      is_min_cap_(false),
      is_leakage_power_(false),
      is_dynamic_power_(false),
      is_cell_em_(false),
      is_signal_em_(false),
      name_(0),
      analysis_corner_(UNINIT_OBJECT_ID),
      analysis_mode_(UNINIT_OBJECT_ID),
      group_id_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeAnalysisView);
}

AnalysisView::~AnalysisView() {
#if 0
    if (analysis_corner_ != UNINIT_OBJECT_ID) {
        AnalysisCorner* analysis_corner = dynamic_cast<AnalysisCorner*>(
            getObject<AnalysisCorner>(analysis_corner_));
        if (analysis_corner) destroyObject<AnalysisCorner>(analysis_corner);
    }

    if (analysis_mode_ != UNINIT_OBJECT_ID) {
        AnalysisMode* analysis_mode = dynamic_cast<AnalysisMode*>(
            getObject<AnalysisMode>(analysis_mode_));
        if (analysis_mode) destroyObject<AnalysisMode>(analysis_mode);
    }
#endif
}

AnalysisView::AnalysisView(Object* owner, AnalysisView::IndexType id)
    : AnalysisView::BaseType(owner, id),
      sdc_(nullptr),
      is_active_(false),
      is_setup_(false),
      is_hold_(false),
      is_max_cap_(false),
      is_max_tran_(false),
      is_min_cap_(false),
      is_leakage_power_(false),
      is_dynamic_power_(false),
      is_cell_em_(false),
      is_signal_em_(false),
      name_(0),
      analysis_corner_(UNINIT_OBJECT_ID),
      analysis_mode_(UNINIT_OBJECT_ID),
      group_id_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeAnalysisView);
}

AnalysisView::AnalysisView(AnalysisView const& rhs) { copy(rhs); }

AnalysisView::AnalysisView(AnalysisView&& rhs) noexcept {
    move(std::move(rhs));
}

AnalysisView& AnalysisView::operator=(AnalysisView const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

AnalysisView& AnalysisView::operator=(AnalysisView&& rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

void AnalysisView::copy(AnalysisView const& rhs) {
    this->BaseType::copy(rhs);
    sdc_ = rhs.sdc_, is_active_ = rhs.is_active_;
    is_setup_ = rhs.is_setup_;
    is_hold_ = rhs.is_hold_;
    is_max_cap_ = rhs.is_max_cap_;
    is_max_tran_ = rhs.is_max_tran_;
    is_min_cap_ = rhs.is_min_cap_;
    is_leakage_power_ = rhs.is_leakage_power_;
    is_dynamic_power_ = rhs.is_dynamic_power_;
    is_cell_em_ = rhs.is_cell_em_;
    is_signal_em_ = rhs.is_signal_em_;
    name_ = rhs.name_;
    analysis_corner_ = rhs.analysis_corner_;
    analysis_mode_ = rhs.analysis_mode_;
    group_id_ = rhs.group_id_;
}

void AnalysisView::move(AnalysisView&& rhs) {
    this->BaseType::move(std::move(rhs));
    sdc_ = std::move(rhs.sdc_);
    is_active_ = std::move(rhs.is_active_);
    is_setup_ = std::move(rhs.is_setup_);
    is_hold_ = std::move(rhs.is_hold_);
    is_max_cap_ = std::move(rhs.is_max_cap_);
    is_max_tran_ = std::move(rhs.is_max_tran_);
    is_min_cap_ = std::move(rhs.is_min_cap_);
    is_leakage_power_ = std::move(rhs.is_leakage_power_);
    is_dynamic_power_ = std::move(rhs.is_dynamic_power_);
    is_cell_em_ = std::move(rhs.is_cell_em_);
    is_signal_em_ = std::move(rhs.is_signal_em_);
    name_ = std::move(rhs.name_);
    analysis_corner_ = std::move(rhs.analysis_corner_);
    rhs.analysis_corner_ = 0;
    analysis_mode_ = std::move(rhs.analysis_mode_);
    rhs.analysis_mode_ = 0;
    group_id_ = std::move(rhs.group_id_);
    rhs.group_id_ = 0;
}

AnalysisView::IndexType AnalysisView::memory() const {
    IndexType ret = this->BaseType::memory();

    ret + sizeof(sdc_);
    ret += sizeof(is_active_);
    ret += sizeof(is_setup_);
    ret += sizeof(is_hold_);
    ret += sizeof(is_max_cap_);
    ret += sizeof(is_max_tran_);
    ret += sizeof(is_min_cap_);
    ret += sizeof(is_leakage_power_);
    ret += sizeof(is_dynamic_power_);
    ret += sizeof(is_cell_em_);
    ret += sizeof(is_signal_em_);
    ret += sizeof(name_);
    ret += sizeof(analysis_corner_);
    ret += sizeof(analysis_mode_);
    ret += sizeof(group_id_);

    return ret;
}

/// set
void AnalysisView::setName(const std::string& name) {
    Timing* timing_lib = getTimingLib();
    if (timing_lib) {
        SymbolIndex idx = timing_lib->getOrCreateSymbol(name.c_str());
        if (idx != kInvalidSymbolIndex) {
            name_ = idx;
            timing_lib->addSymbolReference(name_, this->getId());
        }
    }
}
void AnalysisView::setAnalysisCorner(ObjectId id) { analysis_corner_ = id; }
void AnalysisView::setAnalysisMode(ObjectId id) { analysis_mode_ = id; }
void AnalysisView::setActive(bool b) { is_active_ = b; }
void AnalysisView::setSetup(bool b) { is_setup_ = b; }
void AnalysisView::setHold(bool b) { is_hold_ = b; }
void AnalysisView::setMaxCap(bool b) { is_max_cap_ = b; }
void AnalysisView::setMaxTran(bool b) { is_max_tran_ = b; }
void AnalysisView::setMinCap(bool b) { is_min_cap_ = b; }
void AnalysisView::setLeakagePower(bool b) { is_leakage_power_ = b; }
void AnalysisView::setDynamicPower(bool b) { is_dynamic_power_ = b; }
void AnalysisView::setCellEm(bool b) { is_cell_em_ = b; }
void AnalysisView::setSignalEm(bool b) { is_signal_em_ = b; }
void AnalysisView::setGroupId(ObjectId id) { group_id_ = id; }

void AnalysisView::setVertexLevel(VertexId vertex_id, int level) {
    if (levelized_) {
        return;
    }
    vertex_levels_[vertex_id] = level;
}

void AnalysisView::setVertexInEdges(
    std::vector<unsigned int>& vertex_in_edges) {
    vertex_in_edges_.clear();
    vertex_in_edges_.reserve(vertex_in_edges.size());
    std::copy(vertex_in_edges.begin(), vertex_in_edges.end(),
              vertex_in_edges_.end());
}

void AnalysisView::setVertexOutEdges(
    std::vector<unsigned int>& vertex_out_edges) {
    vertex_out_edges_.clear();
    vertex_out_edges_.reserve(vertex_out_edges.size());
    std::copy(vertex_out_edges.begin(), vertex_out_edges.end(),
              vertex_out_edges_.end());
}

/// get
SymbolIndex AnalysisView::getNameIndex(void) { return name_; }
std::string AnalysisView::getName(void) const {
    Timing* timing_lib = getTimingLib();
    if (timing_lib) {
        return timing_lib->getSymbolByIndex(name_);
    }
    return "";
}
AnalysisCorner* AnalysisView::getAnalysisCorner(void) {
    if (analysis_corner_ == UNINIT_OBJECT_ID) return nullptr;
    return Object::addr<AnalysisCorner>(analysis_corner_);
}

AnalysisMode* AnalysisView::getAnalysisMode(void) {
    if (analysis_mode_ == UNINIT_OBJECT_ID) return nullptr;
    return Object::addr<AnalysisMode>(analysis_mode_);
}
ObjectId AnalysisView::getGroupId(void) { return group_id_; }
bool AnalysisView::isActive(void) { return is_active_; }
bool AnalysisView::isSetup(void) { return is_setup_; }
bool AnalysisView::isHold(void) { return is_hold_; }
bool AnalysisView::isMaxCap(void) { return is_max_cap_; }
bool AnalysisView::isMaxTran(void) { return is_max_tran_; }
bool AnalysisView::isMinCap(void) { return is_min_cap_; }
bool AnalysisView::isLeakagePower(void) { return is_leakage_power_; }
bool AnalysisView::isDynamicPower(void) { return is_dynamic_power_; }
bool AnalysisView::isCellEm(void) { return is_cell_em_; }
bool AnalysisView::isSignalEm(void) { return is_signal_em_; }

TFuncConstant AnalysisView::getVertexConstant(Vertex* v) const {
    auto found = vertex_constants_.find(v);
    if (found == vertex_constants_.end()) {
        return TFuncConstant::kUnknown;
    } else {
        return found->second;
    }
}

ConstantSource AnalysisView::getVertexConstantSource(Vertex* v) const {
    auto found = vertex_constant_source_.find(v);
    if (found == vertex_constant_source_.end()) {
        // return the value of none
        return static_cast<ConstantSource>(1);
    } else {
        return found->second;
    }
}

void AnalysisView::getConstantVertices(std::vector<Vertex*>& vertices) const {
    vertices.clear();
    vertices.reserve(vertex_constants_.size());
    for (auto kv : vertex_constants_) {
        if (kv.second != TFuncConstant::kUnknown) {
            vertices.push_back(kv.first);
        }
    }
}

int AnalysisView::getVertexLevel(VertexId vertex_id) {
    if (!levelized_) {
        return -1;
    }
    assert(vertex_id < vertex_levels_.size());
    return vertex_levels_[vertex_id];
}

bool AnalysisView::forwardPropNeedLock(VertexId vertex_id) const {
    assert(vertex_id < vertex_in_edges_.size());
    return vertex_in_edges_[vertex_id] > 1;
}

bool AnalysisView::backwardPropNeedLock(VertexId vertex_id) const {
    assert(vertex_id < vertex_out_edges_.size());
    return vertex_out_edges_[vertex_id] > 1;
}

double AnalysisView::getDriverCellRd(
    TTerm* pOutTerm, TTerm* pInTerm, double inSlew, double totalCap,
    TimingRiseFall dir /*=TimingRiseFall::kRise_Fall*/) {
    if (pOutTerm == nullptr || pInTerm == nullptr) return 0;
    if (std::fabs(inSlew) < std::numeric_limits<double>::epsilon()) return 0;
    TLib* pLib = getTLibByTTerm(pOutTerm);
    if (pLib == nullptr) return 0;

    std::vector<TimingArc*> arcs;
    pOutTerm->getTimingArcWithTTerm(pInTerm->getId(), arcs);
    if (arcs.empty()) {
        return 0;
    }
    std::unordered_set<TimingTable*> tables;
    for (auto arc : arcs) {
        TimingTable* pTable = nullptr;
        if (dir == TimingRiseFall::kRise_Fall ||
            dir == TimingRiseFall::kFall) {
            pTable = arc->getCellFall();
            if (pTable != nullptr) tables.insert(pTable);
        }
        if (dir == TimingRiseFall::kRise_Fall ||
            dir == TimingRiseFall::kRise) {
            pTable = arc->getCellRise();
            if (pTable != nullptr) tables.insert(pTable);
        }
    }
    if (tables.empty()) return 0;

    double timeUnitScale = 1.0;
    double capUnitScale = 1.0;
    TUnits* pUnits = pLib->getUnits();
    if (pUnits != nullptr) {
        TUnit& timeUnit = pUnits->getTimeUnit();
        timeUnitScale = timeUnit.scale;
        inSlew /= timeUnitScale;

        TUnit& capUnit = pUnits->getCapacitanceUnit();
        capUnitScale = capUnit.scale;
        totalCap /= capUnitScale;
    }

    float axis1Value = 0.0f;
    float axis2Value = 0.0f;
    float axis3Value = 0.0f;
    float axis1ValueScale = 1.0f;
    float axis2ValueScale = 1.0f;
    float axis3ValueScale = 1.0f;
    float cap1 = static_cast<float>(totalCap * 0.75);
    float cap2 = cap1 * 1.1;
    float delay1 = 0.0f;
    float delay2 = 0.0f;

    for (auto pTable : tables) {
        pTable->getAxisValueScaleByAxisVariable(
            timeUnitScale, capUnitScale, 1.0f, axis1ValueScale, axis2ValueScale,
            axis3ValueScale);
        pTable->getAxisValueByAxisVariable(inSlew, cap1, 0, axis1Value,
                                           axis2Value, axis3Value);
        delay1 = std::max(delay1,
                          pTable->findValue(axis1Value, axis2Value, axis3Value,
                                            timeUnitScale, axis1ValueScale,
                                            axis2ValueScale, axis3ValueScale));
        pTable->getAxisValueByAxisVariable(inSlew, cap2, 0, axis1Value,
                                           axis2Value, axis3Value);
        delay2 = std::max(delay2,
                          pTable->findValue(axis1Value, axis2Value, axis3Value,
                                            timeUnitScale, axis1ValueScale,
                                            axis2ValueScale, axis3ValueScale));
    }

    return std::fabs(delay1 - delay2) / ((cap2 - cap1) * capUnitScale);
}

double AnalysisView::getDriverCellDelay(
    TTerm* pOutTerm, TTerm* pInTerm, double inSlew, double totalCap,
    TimingRiseFall dir /*=TimingRiseFall::kRise_Fall*/) {
    return getDriverCellDelayByTable(pOutTerm, pInTerm, inSlew, totalCap, 1,
                                     dir);
}

double AnalysisView::getDriverCellIntrinsicDelay(
    TTerm* pOutTerm, TTerm* pInTerm, double inSlew,
    TimingRiseFall dir /*=TimingRiseFall::kRise_Fall*/) {
    return getDriverCellDelayByTable(pOutTerm, pInTerm, inSlew, 0.0, 1, dir);
}

float AnalysisView::getPinCapacitance(Pin* pin) {
    if (pin == nullptr) return 0.0f;
    TTerm* pTerm = getPinTTerm(pin);

    return getPinCapacitance(pTerm);
}

float AnalysisView::getPinCapacitance(TTerm* pTerm) {
    if (pTerm == nullptr) return 0.0f;
    TLib* pLib = getTLibByTTerm(pTerm);
    if (pLib == nullptr) return 0;

    float result =
        std::max(pTerm->getCapacitance(), pTerm->getRiseCapacitance());
    result = std::max(result, pTerm->getFallCapacitance());

    TUnits* pUnits = pLib->getUnits();
    if (pUnits != nullptr) {
        TUnit& capUnit = pUnits->getCapacitanceUnit();
        result *= capUnit.scale;
    }
    return result;
}

float AnalysisView::getPinMaxCapacitance(TTerm* pTerm) {
    if (pTerm == nullptr) return 0.0f;
    if (pTerm->getDirection() != PinDirection::kOutput &&
        pTerm->getDirection() != PinDirection::kInout)
        return 0.0f;

    TLib* pLib = getTLibByTTerm(pTerm);
    if (pLib == nullptr) return 0;

    float result = pTerm->getMaxCapacitance();
    if (std::fabs(result) < std::numeric_limits<float>::epsilon())
        result = pLib->getDefaultMaxCapacitance();

    TUnits* pUnits = pLib->getUnits();
    if (pUnits != nullptr) {
        TUnit& capUnit = pUnits->getCapacitanceUnit();
        result *= capUnit.scale;
    }
    return result;
}

float AnalysisView::getPinMaxTransition(TTerm* pTerm) {
    if (pTerm == nullptr) return 0.0f;
    if (pTerm->getDirection() != PinDirection::kInput &&
        pTerm->getDirection() != PinDirection::kOutput)
        return 0.0f;
    TLib* pLib = getTLibByTTerm(pTerm);
    if (pLib == nullptr) return 0;

    float result = pTerm->getMaxTransition();
    if (std::fabs(result) < std::numeric_limits<float>::epsilon())
        result = pLib->getDefaultMaxTransition();

    TUnits* pUnits = pLib->getUnits();
    if (pUnits != nullptr) {
        TUnit& timeUnit = pUnits->getTimeUnit();
        result *= timeUnit.scale;
    }
    return result;
}

double AnalysisView::getDriverCellTransition(
    TTerm* pOutTerm, TTerm* pInTerm, double inSlew, double totalCap,
    TimingRiseFall dir /*=TimingRiseFall::kRise_Fall*/) {
    return getDriverCellDelayByTable(pOutTerm, pInTerm, inSlew, totalCap, 2,
                                     dir);
}

TLib* AnalysisView::getTLibByTTerm(TTerm* pTerm) {
    if (pTerm == nullptr) return nullptr;
    ObjectId cellId = pTerm->getGroupId();
    TCell* pCell = Object::addr<TCell>(cellId);
    if (pCell == nullptr) return nullptr;
    ObjectId libId = pCell->getGroupId();
    return Object::addr<TLib>(libId);
}

double AnalysisView::getDriverCellDelayByTable(
    TTerm* pOutTerm, TTerm* pInTerm, double inSlew, double totalCap,
    int type /*=1*/,
    TimingRiseFall dir /*=TimingRiseFall::kRise_Fall*/) {
    if (pOutTerm == nullptr || pInTerm == nullptr) return 0;
    TLib* pLib = getTLibByTTerm(pOutTerm);
    if (pLib == nullptr) return 0;

    std::vector<TimingArc*> arcs;
    pOutTerm->getTimingArcWithTTerm(pInTerm->getId(), arcs);
    if (arcs.empty()) {
        return 0;
    }
    std::unordered_set<TimingTable*> tables;
    for (auto arc : arcs) {
        if (type == 1) {
            TimingTable* pTable = nullptr;
            if (dir == TimingRiseFall::kRise_Fall ||
                dir == TimingRiseFall::kFall) {
                pTable = arc->getCellFall();
                if (pTable != nullptr) tables.insert(pTable);
            }
            if (dir == TimingRiseFall::kRise_Fall ||
                dir == TimingRiseFall::kRise) {
                pTable = arc->getCellRise();
                if (pTable != nullptr) tables.insert(pTable);
            }
        } else if (type == 2) {
            TimingTable* pTable = nullptr;
            if (dir == TimingRiseFall::kRise_Fall ||
                dir == TimingRiseFall::kFall) {
                pTable = arc->getFallTransition();
                if (pTable != nullptr) tables.insert(pTable);
            }
            if (dir == TimingRiseFall::kRise_Fall ||
                dir == TimingRiseFall::kRise) {
                pTable = arc->getRiseTransition();
                if (pTable != nullptr) tables.insert(pTable);
            }
        }
    }
    if (tables.empty()) return 0;

    double timeUnitScale = 1.0;
    double capUnitScale = 1.0;
    float axis1ValueScale = 1.0f;
    float axis2ValueScale = 1.0f;
    float axis3ValueScale = 1.0f;
    TUnits* pUnits = pLib->getUnits();
    if (pUnits != nullptr) {
        TUnit& timeUnit = pUnits->getTimeUnit();
        timeUnitScale = timeUnit.scale;
        inSlew /= timeUnitScale;

        TUnit& capUnit = pUnits->getCapacitanceUnit();
        capUnitScale = capUnit.scale;
        totalCap /= capUnitScale;
    }
    float axis1Value = 0.0f;
    float axis2Value = 0.0f;
    float axis3Value = 0.0f;
    float result = 0.0f;
    for (auto pTable : tables) {
        pTable->getAxisValueScaleByAxisVariable(
            timeUnitScale, capUnitScale, 1.0f, axis1ValueScale, axis2ValueScale,
            axis3ValueScale);
        pTable->getAxisValueByAxisVariable(inSlew, totalCap, 0, axis1Value,
                                           axis2Value, axis3Value);
        result = std::max(result,
                          pTable->findValue(axis1Value, axis2Value, axis3Value,
                                            timeUnitScale, axis1ValueScale,
                                            axis2ValueScale, axis3ValueScale));
    }

    return result;
}

bool AnalysisView::isTriStateEnable(TTerm* outTerm, TTerm* inTerm)
{
    if (outTerm == nullptr || inTerm == nullptr) return false;
    if (outTerm->getDirection() != PinDirection::kOutput) return false;
    return outTerm->isThreeState();
}

TTerm* AnalysisView::getTTerm(Term* pTerm) {
    if (pTerm == nullptr) return nullptr;
    AnalysisCorner* pCorner = getAnalysisCorner();
    if (pCorner != nullptr)
        return pCorner->getTTerm(pTerm);
    else
        return nullptr;
}

void AnalysisView::getTimingArcs(TTerm* pOutTerm, TTerm* pInTerm,
                                 std::vector<TimingArc*>* arcs) {
    if (pOutTerm == nullptr || pInTerm == nullptr || arcs == nullptr) return;

    pOutTerm->getTimingArcWithTTerm(pInTerm->getId(), *arcs);
}

TUnits* AnalysisView::getTUnitsByTTerm(TTerm* pTerm) {
    TLib* pLib = getTLibByTTerm(pTerm);
    if (pLib == nullptr) return nullptr;

    return pLib->getUnits();
}

OStreamBase& operator<<(OStreamBase& os, AnalysisView const& rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");

    AnalysisView::BaseType const& base = rhs;
    os << base << DataDelimiter();

    os << DataFieldName("name_") << rhs.getName() << DataDelimiter();

    os << DataFieldName("analysis_corner_");
    os << rhs.analysis_corner_;
    os << DataBegin("[");
    {
        if (rhs.analysis_corner_ != UNINIT_OBJECT_ID) {
            AnalysisCorner* analysis_corner =
                Object::addr<AnalysisCorner>(rhs.analysis_corner_);
            if (analysis_corner) os << *analysis_corner;
        }
    }
    os << DataEnd("]") << DataDelimiter();

    os << DataFieldName("analysis_mode_");
    os << rhs.analysis_mode_;
    os << DataBegin("[");
    {
        if (rhs.analysis_mode_ != UNINIT_OBJECT_ID) {
            AnalysisMode* analysis_mode =
                Object::addr<AnalysisMode>(rhs.analysis_mode_);
            if (analysis_mode) os << *analysis_mode;
        }
    }
    os << DataEnd("]") << DataDelimiter();

    os << DataFieldName("is_active_") << rhs.is_active_ << DataDelimiter();
    os << DataFieldName("is_setup_") << rhs.is_setup_ << DataDelimiter();
    os << DataFieldName("is_hold_") << rhs.is_hold_ << DataDelimiter();
    os << DataFieldName("is_max_cap_") << rhs.is_max_cap_ << DataDelimiter();
    os << DataFieldName("is_max_tran_") << rhs.is_max_tran_ << DataDelimiter();
    os << DataFieldName("is_min_cap_") << rhs.is_min_cap_ << DataDelimiter();
    os << DataFieldName("is_leakage_power_") << rhs.is_leakage_power_
       << DataDelimiter();
    os << DataFieldName("is_dynamic_power_") << rhs.is_dynamic_power_
       << DataDelimiter();
    os << DataFieldName("is_cell_em_") << rhs.is_cell_em_ << DataDelimiter();
    os << DataFieldName("is_signal_em_") << rhs.is_signal_em_
       << DataDelimiter();
    os << DataFieldName("group_id_") << rhs.group_id_;

    os << DataEnd(")");
    return os;
}
}  // namespace db
}  // namespace open_edi
