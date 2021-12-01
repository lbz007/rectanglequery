 /**
 * @file  root.cpp
 * @date  11/19/2020 01:38:36 PM CST
 * @brief Class Root and StorageUtil.
 *
 * Copyright (C) 2020 NiiCEDA Inc.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "db/core/cell.h"
#include "db/tech/tech.h"
#include "db/core/timing.h"

#include "db/util/symbol_table.h"
#include "util/polygon_table.h"
#include "db/util/property_manager.h"
#include "util/util.h"
#include "db/core/root.h"

namespace open_edi {
namespace db {
// Class Root (runtime object):
Root::Root() : tech_(nullptr), timing_(nullptr), top_cell_(nullptr) {
    hier_owner_vector_.push_back(0);
    hier_owner_map_.insert({0, 0});
}

Root::~Root() {
    if (top_cell_ != nullptr) {
        Object::destroyObject<Cell>(top_cell_);
    }
    if (timing_ != nullptr) {
        Object::destroyObject<Timing>(timing_);
    }
    if (tech_ != nullptr) {
        Object::destroyObject<Tech>(tech_);
    }
    hier_owner_vector_.clear();
    hier_owner_map_.clear();
}

void Root::initTopCell() {
    MemPool::initMemPool();
    // Top cell:
    StorageUtil *top_cell_storage = new StorageUtil(0);
    MemPagePool *top_cell_pool = top_cell_storage->getPool();
    if (top_cell_pool == nullptr) return;
    ObjectId cell_id = 0;
    top_cell_ = top_cell_pool->allocate<Cell>(kObjectTypeCell, cell_id);
    top_cell_->setId(cell_id);
    top_cell_->setObjectType(kObjectTypeCell);
    // also inserted HierOwnerId in HierData
    top_cell_->initHierData(top_cell_storage);
    top_cell_->setCellType(CellType::kHierCell);
    //insertHierOwnerId(cell_id);
    top_cell_->setOwner(cell_id);
    std::string cell_name = "DummyTopCell";
    top_cell_->setName(cell_name);
    MemPool::insertPagePool(cell_id, top_cell_pool);
}

void Root::initTechLib() {
    MemPool::initMemPool();
    // Tech:
    StorageUtil *tech_lib_storage = new StorageUtil(0);
    MemPagePool *tech_lib_pool = tech_lib_storage->getPool();
    if (tech_lib_pool == nullptr) return;
    ObjectId tech_id;
    tech_ = tech_lib_pool->allocate<Tech>(kObjectTypeTech, tech_id);
    tech_->setId(tech_id);
    tech_->setObjectType(kObjectTypeTech);
    tech_->setStorageUtil(tech_lib_storage);
    ObjectIndex index = insertHierOwnerId(tech_id);
    tech_->setHierOwnerIndex(index);
    tech_->setOwner(tech_id);
    MemPool::insertPagePool(tech_id, tech_lib_pool);
}

void Root::initTimingLib() {
    MemPool::initMemPool();
    // Timing:
    StorageUtil *timing_lib_storage = new StorageUtil(0);
    MemPagePool *timing_lib_pool = timing_lib_storage->getPool();
    if (timing_lib_pool == nullptr) return;
    ObjectId timing_id;
    timing_ = timing_lib_pool->allocate<Timing>(kObjectTypeTiming, timing_id);
    timing_->setId(timing_id);
    timing_->setObjectType(kObjectTypeTiming);
    timing_->setStorageUtil(timing_lib_storage);
    ObjectIndex index = insertHierOwnerId(timing_id);
    timing_->setHierOwnerIndex(index);
    timing_->setOwner(timing_id);
    MemPool::insertPagePool(timing_id, timing_lib_pool);
}

void Root::reset() {
    setTimingLib(nullptr);
    setTechLib(nullptr);
    setTopCell(nullptr);
    hier_owner_vector_.clear();
    hier_owner_map_.clear();
    hier_owner_vector_.push_back(0);
    hier_owner_map_.insert({0, 0});
}

void Root::setTechLib(Tech *v) {
    if (tech_ != nullptr) {
        Object::destroyObject<Tech>(tech_);
    }
    tech_ = v;
    if (v != nullptr) {
        insertHierOwnerId(v->getId());
    }
}

Tech* Root::getTechLib() const {
    return tech_;
}

void Root::setTimingLib(Timing *v) {
    if (timing_ != nullptr) {
        Object::destroyObject<Timing>(timing_);
    }
    timing_ = v;
    if (v != nullptr) {
        insertHierOwnerId(v->getId());
    }
}

Timing* Root::getTimingLib() const {
    return timing_;
}

void Root::setTopCell(Cell *v) {
    if (top_cell_ != nullptr) {
        Object::destroyObject<Cell>(top_cell_);
    }
    top_cell_ = v;
    if (v != nullptr) {
        insertHierOwnerId(v->getId());
    }
}

Cell* Root::getTopCell() const {
    return top_cell_;
}

ObjectIndex Root::getHierOwnerIndex(ObjectId owner_id) const {
    if (owner_id != kInvalidObjectId) {
        std::unordered_map<ObjectId, ObjectIndex>::const_iterator iter =
          hier_owner_map_.find(owner_id);
        if (iter != hier_owner_map_.end()) {
            return iter->second;
        }
    }
    return 0;
}

ObjectId Root::getHierOwnerObjectId(ObjectIndex owner_index) const {
    if (owner_index != 0 && owner_index < hier_owner_vector_.size()) {
        return hier_owner_vector_[owner_index];
    }
    return kInvalidObjectId;
}

ObjectIndex Root::insertHierOwnerId(ObjectId owner_id) {
    if (owner_id == kInvalidObjectId) {
        return 0;
    }
    hier_owner_vector_.push_back(owner_id);
    ObjectIndex index = hier_owner_vector_.size() - 1;
    hier_owner_map_.insert({owner_id, index});
    return index;
}

bool Root::deleteHierOwnerId(ObjectId owner_id) {
    if (owner_id == kInvalidObjectId) {
        return false;
    }
    ObjectIndex owner_index = getHierOwnerIndex(owner_id);
    if (owner_index == 0) return false;
    hier_owner_vector_[owner_index] = 0;
    hier_owner_map_.erase(owner_id);
    return true;
}

// for read_design:
bool Root::setHierOwnerId(ObjectIndex index, ObjectId owner_id) {
    if (owner_id == 0 || index <= 0) {
        return false;
    }
    // set the right size:
    if (index >= hier_owner_vector_.capacity()) {
        hier_owner_vector_.reserve(2 * index);
    }
    if (index >= hier_owner_vector_.size()) {
        hier_owner_vector_.resize(index + 1);
    }
    // removed the original owner_id:
    ObjectId origin_id = hier_owner_vector_[index];
    if (origin_id) {
        hier_owner_map_.erase(owner_id);
    }
    // set the new owner_id:
    hier_owner_vector_[index] = owner_id;
    hier_owner_map_.insert({owner_id, index});
    return true;
}

// Class StorageUtil (runtime object):
StorageUtil::StorageUtil() : 
  pool_(nullptr), symtbl_(nullptr), polytbl_(nullptr), propertytbl_(nullptr) {}

StorageUtil::StorageUtil(uint64_t cell_id) {
    initPool(cell_id);
    initSymbolTable();
    initPolygonTable();
    initPropertyManager();
}

StorageUtil::~StorageUtil() {
    if (polytbl_ != nullptr) {
        delete polytbl_;
    }
    if (symtbl_ != nullptr) {
        delete symtbl_;
    }
    // if (propertytbl_ != nullptr) {
    //     delete propertytbl_;
    // }
    // TODO (ly): pool_ ?
}

void StorageUtil::initPool(uint64_t cell_id) {
    if (cell_id == 0) {
        pool_ = MemPool::newPagePool();
    } else {
        pool_ = MemPool::newPagePool(cell_id);
    }
}

void StorageUtil::initSymbolTable() {
    symtbl_ = new SymbolTable;
}

void StorageUtil::initPolygonTable() {
    polytbl_ = new PolygonTable();
}

void StorageUtil::initPropertyManager() {
    propertytbl_ =  PropertyManager::getPropertyManager();
}

void StorageUtil::setPropertyManager(PropertyManager *pt) {
    if (propertytbl_ != nullptr) {
        delete propertytbl_;
    }
    propertytbl_ = pt;
}

PropertyManager *StorageUtil::getPropertyManager() const {
    return propertytbl_;
}

void StorageUtil::setSymbolTable(SymbolTable *stb) {
    if (symtbl_ != nullptr) {
        delete symtbl_;
    }
    symtbl_ = stb;
}

SymbolTable *StorageUtil::getSymbolTable() const {
    return symtbl_;
}

void StorageUtil::setPolygonTable(PolygonTable *pt) {
    if (polytbl_ != nullptr) {
        delete polytbl_;
    }
    polytbl_ = pt;
}

PolygonTable *StorageUtil::getPolygonTable() const {
    return polytbl_;
}

void StorageUtil::setPool(MemPagePool *p) {
    if (pool_ != nullptr) {
        // TODO (ly) free a memory pool?
    }
    pool_ = p;
}

MemPagePool *StorageUtil::getPool() const {
    return pool_;
}

}  // namespace db
}  // namespace open_edi
