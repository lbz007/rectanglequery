/* @file  group.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#include "db/core/group.h"

#include "db/core/cell.h"
#include "db/core/db.h"
#include "db/util/array.h"

namespace open_edi {
namespace db {
using IdArray = ArrayObject<ObjectId>;
using SymIdxArray = ArrayObject<SymbolIndex>;

Group::Group() { memset((void *)this, 0, sizeof(Group)); }

Group::~Group() {}

void Group::setName(std::string &name) {
    StorageUtil *storage_util = getStorageUtil();
    ediAssert(storage_util != nullptr);

    SymbolTable *symtbl = storage_util->getSymbolTable();
    ediAssert(symtbl != nullptr);
    SymbolIndex symbol_index = symtbl->getOrCreateSymbol(name.c_str());
    name_index_ = symbol_index;
}

std::string Group::getName() const {
    StorageUtil *storage_util = getStorageUtil();
    ediAssert(storage_util != nullptr);

    SymbolTable *symtbl = storage_util->getSymbolTable();
    ediAssert(symtbl != nullptr);

    return symtbl->getSymbolByIndex(name_index_);
}

void Group::setNameIndex(SymbolIndex name_index) { name_index_ = name_index; }
SymbolIndex Group::getNameIndex() const { return name_index_; }

void Group::addPattern(const char *pattern_name) {
    StorageUtil *storage_util = getStorageUtil();
    ediAssert(storage_util != nullptr);
    SymbolTable *symtbl = storage_util->getSymbolTable();
    ediAssert(symtbl != nullptr);
    SymbolIndex symbol_index = symtbl->getOrCreateSymbol(pattern_name);
    SymIdxArray *sym_idx_array_ptr = nullptr;
    if (pattern_names_ == 0) {
        sym_idx_array_ptr =
          Object::createObject<SymIdxArray>(kObjectTypeArray, getOwnerId());
        ediAssert(sym_idx_array_ptr != nullptr);
        sym_idx_array_ptr->setPool(storage_util->getPool());
        sym_idx_array_ptr->reserve(8);
        pattern_names_ = sym_idx_array_ptr->getId();
    } else {
        sym_idx_array_ptr = addr<SymIdxArray>(pattern_names_);
        ediAssert(sym_idx_array_ptr != nullptr);
    }
    
    sym_idx_array_ptr->pushBack(symbol_index);
}

ObjectId Group::getPatternNames() const {
    return pattern_names_;
}

void Group::setRegion(ObjectId v) {
    region_ = v;
}

Constraint *Group::getRegion() const {
    if (region_ != 0) {
        return addr<Constraint>(region_);
    }
    return nullptr;
}

void Group::addInstance(ObjectId instance_id) {
    if (instances_ == 0) {
        instances_ = __createObjectIdArray(8);
    }
    ediAssert(instances_ != 0);
    IdArray *id_array_ptr = addr<IdArray>(instances_);
    ediAssert(id_array_ptr != nullptr);
    id_array_ptr->pushBack(instance_id);
}

ObjectId Group::getInstances() const { 
    return instances_;
}

void Group::print(IOManager &io_manager) {
    io_manager.write("- %s", getName().c_str());

    if (instances_) {
        IdArray *inst_array = addr<IdArray>(instances_);
        for (int i = 0; i < inst_array->getSize(); ++i) {
            Inst *instance = addr<Inst>((*inst_array)[i]);
            if (!instance) {
                message->issueMsg("DB", 20, kError, inst_array[i]);
                continue;
            }
            io_manager.write(" %s", instance->getName().c_str());
        }
    }
    if (region_) {
        Constraint *region = addr<Constraint>(region_);
        ediAssert(region != nullptr);
        io_manager.write("\n  + REGION %s", region->getName().c_str());
    }

    Cell *top_cell = getTopCell();
    if (top_cell) {
        PropertyManager* table = top_cell->getPropertyManager(); 
        std::vector<std::string> properties;
        table->getSparsePropertyList<Group>(this, properties);
        if (properties.size() > 0) {
            for(int i = 0; i < properties.size(); i++) {
                Property* prop = table->getSparseProperty<Group>(this, properties.at(i).c_str());
                prop->printDEF(io_manager);
            }
        }
    }
    io_manager.write(" ;\n");
}
}  // namespace db
}  // namespace open_edi
