/**
 * @file analysis_corner.cpp
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

#include "db/timing/timinglib/analysis_corner.h"

#include "db/core/db.h"
#include "db/timing/timinglib/analysis_view.h"
#include "db/timing/timinglib/libset.h"
#include "db/timing/timinglib/timinglib_lib.h"
#include "db/timing/timinglib/timinglib_cell.h"

namespace open_edi {
namespace db {

AnalysisCorner::AnalysisCorner()
    : AnalysisCorner::BaseType(),
      name_(0),
      rc_tech_file_(0),
      libset_(UNINIT_OBJECT_ID),
      design_parasitics_(UNINIT_OBJECT_ID),
      view_ids_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeAnalysisCorner);
}

AnalysisCorner::~AnalysisCorner() {
#if 0
    if (libset_ != UNINIT_OBJECT_ID) {
        LibSet* libset = dynamic_cast<LibSet*>(getObject<LibSet>(libset_));
        if (libset) destroyObject<LibSet>(libset);
    }
#endif
}

AnalysisCorner::AnalysisCorner(Object* owner, AnalysisCorner::IndexType id)
    : AnalysisCorner::BaseType(owner, id),
      name_(0),
      rc_tech_file_(0),
      libset_(UNINIT_OBJECT_ID),
      design_parasitics_(UNINIT_OBJECT_ID),
      view_ids_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeAnalysisCorner);
}

AnalysisCorner::AnalysisCorner(AnalysisCorner const& rhs) { copy(rhs); }

AnalysisCorner::AnalysisCorner(AnalysisCorner&& rhs) noexcept {
    move(std::move(rhs));
}

AnalysisCorner& AnalysisCorner::operator=(AnalysisCorner const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

AnalysisCorner& AnalysisCorner::operator=(AnalysisCorner&& rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

void AnalysisCorner::copy(AnalysisCorner const& rhs) {
    this->BaseType::copy(rhs);
    name_ = rhs.name_;
    rc_tech_file_ = rhs.rc_tech_file_;
    libset_ = rhs.libset_;
    design_parasitics_ = rhs.design_parasitics_;
    view_ids_ = rhs.view_ids_;
}

void AnalysisCorner::move(AnalysisCorner&& rhs) {
    this->BaseType::move(std::move(rhs));
    name_ = std::move(rhs.name_);
    rc_tech_file_ = std::move(rhs.rc_tech_file_);
    libset_ = std::move(rhs.libset_);
    design_parasitics_ = std::move(rhs.design_parasitics_);
    view_ids_ = std::move(rhs.view_ids_);
}

AnalysisCorner::IndexType AnalysisCorner::memory() const {
    IndexType ret = this->BaseType::memory();

    ret += sizeof(name_);
    ret += sizeof(rc_tech_file_);
    ret += sizeof(libset_);
    ret += sizeof(design_parasitics_);
    ret += sizeof(view_ids_);

    return ret;
}

/// set
void AnalysisCorner::setName(const std::string& name) {
    Timing* timing_lib = getTimingLib();
    if (timing_lib) {
        SymbolIndex idx = timing_lib->getOrCreateSymbol(name.c_str());
        if (idx != kInvalidSymbolIndex) {
            name_ = idx;
            timing_lib->addSymbolReference(name_, this->getId());
        }
    }
}
void AnalysisCorner::setRcTechFile(const std::string& file) {
    Timing* timing_lib = getTimingLib();
    if (timing_lib) {
        SymbolIndex idx = timing_lib->getOrCreateSymbol(file.c_str());
        if (idx != kInvalidSymbolIndex) {
            rc_tech_file_ = idx;
            timing_lib->addSymbolReference(rc_tech_file_, this->getId());
        }
    }
}
void AnalysisCorner::setLibset(ObjectId id) { libset_ = id; }
void AnalysisCorner::setDesignParasitics(ObjectId id) {
    design_parasitics_ = id;
}
void AnalysisCorner::addAnalysisView(AnalysisView* view) {
    if (view == nullptr) return;
    Timing* timing_lib = getTimingLib();
    if (timing_lib) {
        ArrayObject<ObjectId>* p = nullptr;
        if (view_ids_ == UNINIT_OBJECT_ID) {
            p = Object::createObject<ArrayObject<ObjectId>>(
                kObjectTypeArray, timing_lib->getId());
            if (p != nullptr) {
                view_ids_ = p->getId();
                p->setPool(timing_lib->getPool());
                p->reserve(16);
            }
        } else {
            p = Object::addr<ArrayObject<ObjectId>>(view_ids_);
        }
        if (p != nullptr) {
            p->pushBack(view->getId());
        }
    }
}

void
AnalysisCorner::buildTermTTermMap(void)
{
    if (term_tterm_map_.size() > 0) {return;}
    std::unordered_map<Cell*, TCell*> lib_cell_map;

    ArrayObject<ObjectId>* all_ref_cells = getRoot()->getTechLib()->getCellArray();
    if (all_ref_cells == nullptr) {return;}
    for (int i=0; i<all_ref_cells->getSize(); i++) {
        ObjectId ref_cell_id = all_ref_cells->operator[](i);
        Cell* ref_cell = Object::addr<Cell>(ref_cell_id);
        if (!(ref_cell->isHierCell())) {
            if (lib_cell_map.find(ref_cell) == lib_cell_map.end()) {
                TCell* lib_cell = nullptr;
                auto libset = this->getLibset();
                auto libs = libset->getTimingLibs();
                for (TLib* lib : libs) {
                    TCell* lc = lib->getTimingCell(ref_cell->getName());
                    if (lc != nullptr) {
                        lib_cell = lc;
                        break;
                    }
                }
                if (lib_cell != nullptr) {
                    lib_cell_map[ref_cell] = lib_cell;
                } 
            }
        }
    }
    
    std::unordered_map<Cell*, std::unordered_map<Term*, int>> cell_term_index;
    for (auto kv : lib_cell_map) {
        Cell* cell = kv.first;
        ArrayObject<ObjectId>* terms = Object::addr<ArrayObject<ObjectId>>(cell->getTerms());
        TCell* lib_cell = lib_cell_map[cell];
        std::vector<TTerm*> lib_pins = lib_cell->getTerms();
        //std::unordered_map<Term*, int> term_idx;
        for (int i=0; i<terms->getSize(); i++) {
            ObjectId id = terms->operator[](i);
            Term* term = Object::addr<Term>(id);
            TTerm* tterm = lib_pins[i];
            term_tterm_map_[term] = tterm;
        }
    }
}

/// get
SymbolIndex AnalysisCorner::getNameIndex(void) { return name_; }
SymbolIndex AnalysisCorner::getRcTechFileIndex(void) { return rc_tech_file_; }
std::string AnalysisCorner::getName(void) const {
    Timing* timing_lib = getTimingLib();
    if (timing_lib) {
        return timing_lib->getSymbolByIndex(name_);
    }
    return "";
}
std::string AnalysisCorner::getRcTechFile(void) const {
    Timing* timing_lib = getTimingLib();
    if (timing_lib) {
        return timing_lib->getSymbolByIndex(rc_tech_file_);
    }
    return "";
}
LibSet* AnalysisCorner::getLibset(void) {
    if (libset_ != UNINIT_OBJECT_ID)
        return Object::addr<LibSet>(libset_);
    else
        return nullptr;
}
DesignParasitics* AnalysisCorner::getDesignParasitics(void) {
    if (design_parasitics_ != UNINIT_OBJECT_ID)
        return Object::addr<DesignParasitics>(design_parasitics_);
    else
        return nullptr;
}
std::vector<AnalysisView*> AnalysisCorner::getAnalysisViews(void) const {
    std::vector<AnalysisView*> viewVec;
    if (view_ids_ != UNINIT_OBJECT_ID) {
        auto p = Object::addr<ArrayObject<ObjectId>>(view_ids_);
        if (p != nullptr) {
            int64_t count = p->getSize();
            if (count != 0) viewVec.reserve(count);
            for (int64_t i = 0; i < count; ++i) {
                auto view = Object::addr<AnalysisView>((*p)[i]);
                if (view != nullptr) viewVec.emplace_back(view);
            }
        }
    }
    return viewVec;
}
AnalysisView* AnalysisCorner::getAnalysisView(ObjectId viewId) const {
    if (view_ids_ != UNINIT_OBJECT_ID) {
        auto p = Object::addr<ArrayObject<ObjectId>>(view_ids_);
        if (p != nullptr) {
            for (int64_t i = 0; i < p->getSize(); ++i) {
                if (viewId == (*p)[i])
                    return Object::addr<AnalysisView>((*p)[i]);
            }
        }
    }
    return nullptr;
}

AnalysisView* AnalysisCorner::getAnalysisView(
    const std::string& viewName) const {
    if (view_ids_ != UNINIT_OBJECT_ID) {
        auto p = Object::addr<ArrayObject<ObjectId>>(view_ids_);
        if (p != nullptr) {
            for (int64_t i = 0; i < p->getSize(); ++i) {
                auto view = Object::addr<AnalysisView>((*p)[i]);
                if (view != nullptr) {
                    if (viewName == view->getName()) return view;
                }
            }
        }
    }
    return nullptr;
}

TTerm*
AnalysisCorner::getPinTTerm(Pin* pin) const
{
    Term* term = pin->getTerm();
    auto found = term_tterm_map_.find(term);
    if (found == term_tterm_map_.end()) {
        return nullptr;
    } else {
        return found->second;
    }
}

TTerm* AnalysisCorner::getTTerm(Term* pTerm) const {
    auto found = term_tterm_map_.find(pTerm);
    if (found == term_tterm_map_.end()) {
        return nullptr;
    } else {
        return found->second;
    }
}

OStreamBase& operator<<(OStreamBase& os, AnalysisCorner const& rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");

    AnalysisCorner::BaseType const& base = rhs;
    os << base << DataDelimiter();

    os << DataFieldName("name_") << rhs.getName() << DataDelimiter();
    os << DataFieldName("rc_tech_file_") << rhs.getRcTechFile()
       << DataDelimiter();

    os << DataFieldName("libset_");
    os << rhs.libset_;
    os << DataBegin("[");
    {
        LibSet* libset = nullptr;
        if (rhs.libset_ != UNINIT_OBJECT_ID)
            libset = Object::addr<LibSet>(rhs.libset_);
        if (libset) os << *libset;
    }
    os << DataEnd("]") << DataDelimiter();

    os << DataFieldName("design_parasitics_") << rhs.design_parasitics_
       << DataDelimiter();
    os << DataFieldName("view_ids_");
    {
        ArrayObject<ObjectId>* p = nullptr;
        if (rhs.view_ids_ != UNINIT_OBJECT_ID)
            p = Object::addr<ArrayObject<ObjectId>>(rhs.view_ids_);
        if (p != nullptr)
            os << p->getSize();
        else
            os << 0;
        os << DataBegin("[");
        if (p != nullptr) {
            auto delimiter = DataDelimiter("");
            for (int64_t i = 0; i < p->getSize(); ++i) {
                auto view = Object::addr<AnalysisView>((*p)[i]);
                if (view != nullptr)
                    os << delimiter << view->getName();
                else
                    os << delimiter << "";
                delimiter = DataDelimiter();
            }
        }
        os << DataEnd("]");
    }

    os << DataEnd(")");
    return os;
}
}  // namespace db
}  // namespace open_edi
