/**
 * @file  root.h
 * @date  11/19/2020 01:38:36 PM CST
 * @brief Class Root and StorageUtil
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#ifndef SRC_DB_CORE_ROOT_H_
#define SRC_DB_CORE_ROOT_H_

#include <string>
#include <vector>
#include <unordered_map>
#include "db/core/cell.h"
#include "db/tech/tech.h"
#include "db/core/timing.h"

#include "db/util/symbol_table.h"
#include "util/util.h"


namespace open_edi {
namespace db {

class Timing;
class PropertyManager;
const ObjectIndex kInvalidOwnerIndex = 0;

/// @brief root class: runtime
class Root {
  public:
    static Root & getInstance() {
        static Root root_inst;
        return root_inst;
    }
    void initTechLib();
    void initTimingLib();
    void initTopCell();
    void reset();
    void setTechLib(Tech *v);
    Tech* getTechLib() const;
    void setTimingLib(Timing *v);
    Timing* getTimingLib() const;
    void setTopCell(Cell *v);
    Cell* getTopCell() const;
    ObjectIndex getHierOwnerIndex(ObjectId owner_id) const;
    ObjectId getHierOwnerObjectId(ObjectIndex owner_index) const;
    ObjectIndex insertHierOwnerId(ObjectId owner_id);
    bool deleteHierOwnerId(ObjectId owner_id);
    bool setHierOwnerId(ObjectIndex index, ObjectId owner_id);

  private:
    Root();
    ~Root();

    Tech *tech_;
    Timing *timing_;
    Cell *top_cell_;
    std::vector<ObjectId> hier_owner_vector_;
    std::unordered_map<ObjectId, ObjectIndex> hier_owner_map_;
};

class StorageUtil {
  public:
    StorageUtil();
    StorageUtil(uint64_t cell_id);
    ~StorageUtil();

    void initPool(uint64_t cell_id);
    void initSymbolTable();
    void initPolygonTable();
    void initPropertyManager();

    void setSymbolTable(SymbolTable *stb);
    SymbolTable *getSymbolTable() const;
    void setPolygonTable(PolygonTable *pt);
    PolygonTable *getPolygonTable() const;
    void setPool(MemPagePool *p);
    MemPagePool *getPool() const;
    void setPropertyManager(PropertyManager *pt);
    PropertyManager *getPropertyManager() const;

  private:
    MemPagePool *pool_;  ///< use the memory pool to allocate object
    SymbolTable *symtbl_;
    PolygonTable *polytbl_;
    PropertyManager *propertytbl_;

};

}  // namespace db
}  // namespace open_edi

#endif  // SRC_DB_CORE_ROOT_H_
