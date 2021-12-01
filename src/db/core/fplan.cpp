/**
 * @file  fplan.cpp
 * @date  08/24/2020 09:54:42 AM CST
 * @brief "Implementation of Floorplan related classes"
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "db/core/fplan.h"

#include "db/core/cell.h"
#include "db/core/db.h"
#include "db/util/array.h"
#include "db/util/symbol_table.h"
#include "util/util_mem.h"

namespace open_edi {
namespace db {
using IdArray = ArrayObject<ObjectId>;

/// @brief Row
Row::Row() {
    memset((void *)this, 0, sizeof(Row));
    row_name_index_ = 0;
    site_name_index_ = 0;
    bbox_ = Box();
    legal_orient_ = Orient::kN;
    orient_ = Orient::kN;
    site_count_ = 0;
}

/// @brief Row
///
/// @param r
Row::Row(Row &r) { copy(r); }

/// @brief Row
///
/// @param r
Row::Row(Row &&r) { move(std::move(r)); }

/// @brief ~Row
Row::~Row() {}

/// @brief getName
///
/// @return
std::string &Row::getName() {
    return getStorageUtil()->getSymbolTable()->getSymbolByIndex(row_name_index_);
}

/// @brief setName
///
/// @param name
void Row::setName(SymbolIndex &row_name_index) {
    row_name_index_ = row_name_index;
    getStorageUtil()->getSymbolTable()->addReference(row_name_index_,
                                                       this->getId());
}

/// @brief setName
///
/// @param name
///
/// @return
bool Row::setName(std::string &name) {
    return setName(name.c_str());
}

/// @brief setName
///
/// @param name
///
/// @return
bool Row::setName(const char *name) {
    SymbolIndex index = 
        getStorageUtil()->getSymbolTable()->getOrCreateSymbol(name);
    if (index == kInvalidSymbolIndex) return false;

    setName(index);
    return true;  
}

/// @brief &getBox
///
/// @return
Box Row::getBox() const { return bbox_; }

/// @brief setBox
///
/// @param b
void Row::setBox(Box &b) { bbox_ = b; }

/// @brief getLegalOrient
///
/// @return
Orient Row::getLegalOrient() const { return legal_orient_; }

/// @brief setLegalOrient
///
/// @param o
void Row::setLegalOrient(Orient &o) { legal_orient_ = o; }

/// @brief getOrient
///
/// @return
Orient Row::getOrient() const { return orient_; }

/// @brief setOrient
///
/// @param o
void Row::setOrient(Orient &o) { orient_ = o; }

void Row::setSiteId(ObjectId &site_id) { site_id_ = site_id; }

Site *Row::getSite() const { return addr<Site>(site_id_); }

ObjectId Row::getSiteId() const { return site_id_; }

/// @brief getSiteCount
///
/// @return
int32_t Row::getSiteCount() const { return site_count_; }

/// @brief setSiteCount
///
/// @param sc
void Row::setSiteCount(int32_t &sc) { site_count_ = sc; }

/// @brief copy
///
/// @param r
void Row::copy(Row const &r) {
    this->BaseType::copy(r);
    bbox_ = r.bbox_;
    legal_orient_ = r.legal_orient_;
    orient_ = r.orient_;
    site_name_index_ = r.site_id_;
    site_count_ = r.site_count_;
}

/// @brief move
///
/// @param rhs
void Row::move(Row &&r) {
    this->BaseType::move(std::move(r));
    row_name_index_ = std::move(r.row_name_index_);
    bbox_ = std::move(r.bbox_);
    legal_orient_ = std::move(r.legal_orient_);
    orient_ = std::move(r.orient_);
    site_name_index_ = std::move(r.site_name_index_);
    site_count_ = std::move(r.site_count_);
}

/// @brief setFloorplan
///
/// @param fp
void Row::setFloorplan(ObjectId fp) { floorplan_ = fp; }

/// @brief getFloorplan
///
/// @return
Floorplan *Row::getFloorplan() { return addr<Floorplan>(floorplan_); }

void Row::setSiteName(const char *site_name) {
    if (!site_name) {
        message->issueMsg("DB", 13, kError);
        return;
    }

    site_name_index_ =
        getStorageUtil()->getSymbolTable()->getOrCreateSymbol(site_name);
}

std::string &Row::getSiteName() {
    SymbolTable *sym_tbl = getStorageUtil()->getSymbolTable();
    ediAssert(sym_tbl != nullptr && site_name_index_ != kInvalidSymbolIndex);
    return sym_tbl->getSymbolByIndex(site_name_index_);
}

void Row::print() {
    message->info("ROW %s %s %d %d %s", getName().c_str(),
                  getSiteName().c_str(), getOrigX(), getOrigY(),
                  toString(getSiteOrient()).c_str());
    if (getHasDo()) {
        message->info(" DO %d BY %d", getNumX(), getNumY());
        if (getHasDoStep()) {
            message->info(" STEP %d %d ;\n", getStepX(), getStepY());
        } else {
            message->info(" ;\n");
        }
    }
}

void Row::print(IOManager &io_manager) {
    io_manager.write("ROW %s %s %d %d %s",
        getName().c_str(), getSiteName().c_str(),
        getOrigX(), getOrigY(), toString(getSiteOrient()).c_str());

    if (getHasDo()) {
        io_manager.write(" DO %d BY %d",
            getNumX(), getNumY());
        if (getHasDoStep()) {
            io_manager.write(" STEP %d %d",
                getStepX(), getStepY());
        }
    }

    Cell *top_cell = getTopCell();
    if (top_cell) {
        PropertyManager* table = top_cell->getPropertyManager(); 
        std::vector<std::string> properties;
        table->getSparsePropertyList<Row>(this, properties);
        if (properties.size() > 0) {
            for(int i = 0; i < properties.size(); i++) {
                Property* prop = table->getSparseProperty<Row>(this, properties.at(i).c_str());
                prop->printDEF(io_manager);
            }
        }
    }

    io_manager.write(" ;\n");
}

/// @brief Track default constructor
Track::Track() { memset((void *)this, 0, sizeof(Track)); }

/// @brief Track copy constructor
///
/// @param t
Track::Track(Track &t) { copy(t); }

/// @brief Track move constructor
///
/// @param rhs
Track::Track(Track &&rhs) { move(std::move(rhs)); }

/// @brief ~Track
Track::~Track() {}

void Track::setFloorplan(ObjectId fp) { floorplan_ = fp; }

Floorplan *Track::getFloorplan() { return addr<Floorplan>(floorplan_); }

void Track::setDirectionX(bool direction_x) { direction_x_ = direction_x; }
bool Track::getDirectionX() { return direction_x_; }

void Track::setStart(int32_t start) { start_ = start; }
int32_t Track::getStart() { return start_; }

void Track::setNumTracks(uint32_t num_tracks) { num_tracks_ = num_tracks; }
uint32_t Track::getNumTracks() { return num_tracks_; }

void Track::setSpace(uint32_t space) { space_ = space; }
uint32_t Track::getSpace() { return space_; }

void Track::setHasMask(bool has_mask) { has_mask_ = has_mask; }
bool Track::getHasMask() { return has_mask_; }

void Track::setMask(uint8_t mask) { mask_ = mask; }
uint8_t Track::getMask() { return mask_; }

void Track::setHasSameMask(bool has_same_mask) {
    has_same_mask_ = has_same_mask;
}
bool Track::getHasSameMask() { return has_same_mask_; }

void Track::addLayer(ObjectIndex layer_index) {
    ArrayObject<Int32> *id_array_ptr = nullptr;
    ObjectId owner_id = getOwnerId();

    if (layers_ == 0) {
        id_array_ptr = Object::createObject<ArrayObject<Int32>>(
                        kObjectTypeArray, owner_id);
        if (id_array_ptr == nullptr) return;
        id_array_ptr->setPool(getStorageUtil()->getPool());
        id_array_ptr->reserve(256);
        layers_ = id_array_ptr->getId();
    } else {
        id_array_ptr = addr<ArrayObject<Int32>>(layers_);
    }

    if (id_array_ptr) id_array_ptr->pushBack(layer_index);
}

void Track::addLayer(const char *layer_name) {
    Tech *tech_lib = getTechLib();
    if (!tech_lib) {
        message->issueMsg("DB", 14, kError);
        return;
    }

    ObjectIndex layer_index = tech_lib->getLayerLEFIndexByName(layer_name);
    if (layer_index == kInvalidLayerIndex) {
        message->issueMsg("DB", 15, kError, layer_name);
        return;
    }
    addLayer(layer_index);
}

void Track::addLayer(std::string &layer_name) {
    addLayer(layer_name.c_str());
}

ObjectId Track::getLayers() const { return layers_; }

void Track::print() {
    Tech *tech_lib = getTechLib();
    if (!tech_lib) {
        message->issueMsg("DB", 16, kError);
        return;
    }
    message->info("TRACKS %c", getDirectionX() ? 'X' : 'Y');
    message->info(" %d", getStart());
    message->info(" DO %d", getNumTracks());
    message->info(" STEP %d", getSpace());

    if (getHasMask()) {
        message->info(" MASK %d", getMask());
        if (getHasSameMask()) {
            message->info(" SAMEMASK");
        }
    }

    ArrayObject<Int32> *id_array_ptr = addr<ArrayObject<Int32>>(layers_);
    for (int i = 0; i < id_array_ptr->getSize(); ++i) {
        Int32 layer_index = (*id_array_ptr)[i];
        Layer *layer = tech_lib->getLayer(layer_index);
        if (0 == i) {
            message->info(" LAYER");
        }
        message->info(" %s", layer->getName());
    }
    message->info(" ;\n");
}

void Track::print(IOManager &io_manager) {
    Tech *tech_lib = getTechLib();

    if (!tech_lib) {
        message->issueMsg("DB", 16, kError);
        return;
    }
    io_manager.write("TRACKS %c %d DO %d STEP %d",
        (getDirectionX() ? 'X' : 'Y'),
        getStart(), getNumTracks(), getSpace());

    if (getHasMask()) {
        io_manager.write(" MASK %d", getMask());
        if (getHasSameMask()) {
            io_manager.write(" SAMEMASK");
        }
    }

    if (layers_ > 0) {
        ArrayObject<Int32> *id_array_ptr = addr<ArrayObject<Int32>>(layers_);
        for (int i = 0; i < id_array_ptr->getSize(); ++i) {
            Int32 layer_index = (*id_array_ptr)[i];
            Layer *layer = tech_lib->getLayer(layer_index);
            if (0 == i) {
                io_manager.write(" LAYER");
            }
            io_manager.write(" %s", layer->getName());
        }
    }
    io_manager.write(" ;\n");
}

void Track::copy(Track const &t) {
    this->BaseType::copy(t);
    direction_x_ = t.direction_x_;
    start_ = t.start_;
    num_tracks_ = t.num_tracks_;
    space_ = t.space_;
    has_mask_ = t.has_mask_;
    mask_ = t.mask_;
    has_same_mask_ = t.has_same_mask_;
    layers_ = t.layers_;
}

/// @brief move
///
/// @param rhs
#if 0
void Track::move(Track &&rhs)
{
    this->BaseType::move(std::move(rhs));
    mask_ = std::move(rhs.mask_);
    layer_ = std::move(rhs.layer_);
    spacing_ = std::move(rhs.spacing_);
    width_ = std::move(rhs.width_);
    offset_ = std::move(rhs.offset_);
}
#endif

Grid::Grid() { memset((void *)this, 0, sizeof(Grid)); }

/// @brief Grid
///
/// @param t
Grid::Grid(Grid::GridType t) {
    memset((void *)this, 0, sizeof(Grid));
    grid_type_ = t;
}

/// @brief Grid
///
/// @param rhs
Grid::Grid(Grid &&rhs) { move(std::move(rhs)); }

Grid::~Grid() {}

/// @brief setGridType
///
/// @param t
void Grid::setGridType(Grid::GridType t) { grid_type_ = t; }

/// @brief getGridType
///
/// @return
Grid::GridType Grid::getGridType() { return grid_type_; }

void Grid::setFloorplan(ObjectId fp) { floorplan_ = fp; }

Floorplan *Grid::getFloorplan() { return addr<Floorplan>(floorplan_); }

void Grid::setDirectionX(bool direction_x) { direction_x_ = direction_x; }
bool Grid::getDirectionX() { return direction_x_; }

void Grid::setStart(int32_t start) { start_ = start; }
int32_t Grid::getStart() { return start_; }

void Grid::setNumber(uint32_t number) { number_ = number; }
uint32_t Grid::getNumber() { return number_; }

void Grid::setSpace(uint32_t space) { space_ = space; }
uint32_t Grid::getSpace() { return space_; }

void Grid::print() {
    message->info("GCELLGRID %c", getDirectionX() ? 'X' : 'Y');
    message->info(" %d", getStart());
    message->info(" DO %d", getNumber() + 1);  // +1 based on DEF syntax.
    message->info(" STEP %d", getSpace());

    message->info(" ;\n");
}

void Grid::print(IOManager &io_manager) {
    io_manager.write("GCELLGRID %c %d DO %d STEP %d ;\n",
        (getDirectionX() ? 'X' : 'Y'),
        getStart(), (getNumber() + 1), getSpace());
}

void Grid::copy(Grid const &t) {
    this->BaseType::copy(t);
    direction_x_ = t.direction_x_;
    start_ = t.start_;
    number_ = t.number_;
    space_ = t.space_;
}

/// @brief move
///
/// @param rhs
#if 0
void Grid::move(Grid &&rhs)
{
    this->BaseType::move(std::move(rhs));
    mask_ = std::move(rhs.mask_);
    layer_ = std::move(rhs.layer_);
    spacing_ = std::move(rhs.spacing_);
}
#endif

uint64_t Floorplan::getNumOfRows() const {
    if (rows_ == 0) return 0;
    IdArray *obj_vector = addr<IdArray>(rows_);
    if (obj_vector == nullptr) return 0;
    return obj_vector->getSize();
}

Row *Floorplan::createRow() {
    Row *row = Object::createObject<Row>(kObjectTypeRow, getOwnerId());
    row->setFloorplan(this->getId());
    if (rows_ == 0) {
        rows_ = __createObjectIdArray(64);
    }
    IdArray *row_vector = addr<IdArray>(rows_);
    row_vector->pushBack(row->getId());
    return row;
}

ObjectId Floorplan::getRows() const { return rows_; }

uint64_t Floorplan::getNumOfTracks() const {
    if (tracks_ == 0) return 0;
    IdArray *obj_vector = addr<IdArray>(tracks_);
    if (obj_vector == nullptr) return 0;
    return obj_vector->getSize();
}

Track *Floorplan::createTrack() {
    Track *track = Object::createObject<Track>(kObjectTypeTrack, getOwnerId());
    track->setFloorplan(this->getId());
    if (tracks_ == 0) {
        tracks_ = __createObjectIdArray(64);
    }
    IdArray *track_vector = addr<IdArray>(tracks_);
    track_vector->pushBack(track->getId());
    return track;
}

ObjectId Floorplan::getTracks() const { return tracks_; }

uint64_t Floorplan::getNumOfGrids() const {
    if (gcell_grids_ == 0) return 0;
    IdArray *obj_vector = addr<IdArray>(gcell_grids_);
    if (obj_vector == nullptr) return 0;
    return obj_vector->getSize();
}

Grid *Floorplan::createGcellGrid() {
    Grid *grid = Object::createObject<Grid>(kObjectTypeGrid, getOwnerId());
    grid->setGridType(Grid::kGridGcell);
    grid->setFloorplan(this->getId());
    if (gcell_grids_ == 0) {
        gcell_grids_ = __createObjectIdArray(64);
    }
    IdArray *grid_vector = addr<IdArray>(gcell_grids_);
    grid_vector->pushBack(grid->getId());
    return grid;
}

ObjectId Floorplan::getGcellGrids() const { return gcell_grids_; }

void Floorplan::setDieArea(ObjectIndex die_area) { die_area_ = die_area; }

ObjectIndex Floorplan::getDieArea() { return die_area_; }
Polygon *Floorplan::getDieAreaPolygon() {
    if (die_area_ == -1) {
        return nullptr;
    }
    StorageUtil *storage_util = getStorageUtil();
    ediAssert(storage_util != nullptr);
    PolygonTable *poly_tbl = storage_util->getPolygonTable();
    if (!poly_tbl) {
        message->issueMsg("DB", 17, kError);
        return nullptr;
    }

    return poly_tbl->getPolygonByIndex(die_area_);
}

// Floorplan class
/// @brief Floorplan
Floorplan::Floorplan() {
    memset((void *)this, 0, sizeof(Floorplan));
    die_area_ = -1;
}

/// @brief Floorplan
///
/// @param fp
Floorplan::Floorplan(Floorplan &fp) { copy(fp); }
/// @brief Floorplan
///
/// @param fp
Floorplan::Floorplan(Floorplan &&fp) { move(std::move(fp)); }
/// @brief ~Floorplan
Floorplan::~Floorplan() {}

/// @brief getCoreBox
///
/// @return
Box Floorplan::getCoreBox() const { return core_box_; }
/// @brief setCoreBox
///
/// @param box
void Floorplan::setCoreBox(Box &box) { core_box_ = box; }

/// @brief getXOffset
///
/// @return
int32_t Floorplan::getXOffset() const { return x_offset_; }
/// @brief setXOffset
///
/// @param offset
void Floorplan::setXOffset(int32_t &offset) { x_offset_ = offset; }

/// @brief getYOffset
///
/// @return
int32_t Floorplan::getYOffset() const { return y_offset_; }
/// @brief setYOffset
///
/// @param offset
void Floorplan::setYOffset(int32_t &offset) { y_offset_ = offset; }

/// @brief getCoreSite
///
/// @return
Site *Floorplan::getCoreSite() const { return addr<Site>(core_site_id_); }
/// @brief getCoreSiteId
///
/// @return
ObjectId Floorplan::getCoreSiteId() const { return core_site_id_; }
/// @brief setCoreSite
///
/// @param id
void Floorplan::setCoreSiteId(ObjectId &id) { core_site_id_ = id; }

/// @brief copy
///
/// @param fp
void Floorplan::copy(Floorplan const &fp) {
    this->BaseType::copy(fp);
    core_box_ = fp.core_box_;
    x_offset_ = fp.x_offset_;
    y_offset_ = fp.y_offset_;
    core_site_id_ = fp.core_site_id_;
    // TODO: copy tracks & rows
}

/// @brief move
///
/// @param fp
void Floorplan::move(Floorplan &&fp) {
    this->BaseType::move(std::move(fp));
    core_box_ = std::move(fp.core_box_);
    x_offset_ = std::move(fp.x_offset_);
    y_offset_ = std::move(fp.y_offset_);
    core_site_id_ = std::move(fp.core_site_id_);
    // TODO: move tracks & rows
}

Constraint *Floorplan::createPlaceBlockage() {
    IdArray *id_array_ptr = nullptr;
    Constraint *cons = nullptr;

    if (place_blockages_ == 0) {
        place_blockages_ = __createObjectIdArray(32);
    }
    ediAssert(place_blockages_ != 0);
    id_array_ptr = addr<IdArray>(place_blockages_);
    if (id_array_ptr == nullptr) return nullptr;

    cons = Object::createObject<Constraint>(
            kObjectTypePhysicalConstraint, getOwnerId());
    ediAssert(cons != nullptr);
    cons->setFloorplan(this->getId());
    cons->setConstraintType(Constraint::kConstraintPBlkg);
    id_array_ptr->pushBack(cons->getId());

    return cons;
}

Constraint *Floorplan::createRegion(const char *name) {
    IdArray *id_array_ptr = nullptr;
    Constraint *region = nullptr;

    if (regions_ == 0) {
        regions_ = __createObjectIdArray(32);
    }
    ediAssert(regions_ != 0);
    id_array_ptr = addr<IdArray>(regions_);
    if (id_array_ptr == nullptr) return nullptr;

    region = Object::createObject<Constraint>(kObjectTypeRegion, getOwnerId());
    ediAssert(region != nullptr);

    region->setFloorplan(this->getId());
    region->setName(name);
    region->setConstraintType(Constraint::kConstraintRegion);
    id_array_ptr->pushBack(region->getId());

    return region;
}
uint64_t Floorplan::getNumOfRegions() const {
    if (regions_ == 0) return 0;
    IdArray *obj_vector = addr<IdArray>(regions_);
    if (obj_vector == nullptr) return 0;
    return obj_vector->getSize();
}
ObjectId Floorplan::getRegions() const { return regions_; }
Constraint *Floorplan::getRegion(std::string &name) const {
    if (regions_ == 0) return nullptr;

    StorageUtil *storage_util = getStorageUtil();
    ediAssert(storage_util != nullptr);
    SymbolTable *symtbl = storage_util->getSymbolTable();
    ediAssert(symtbl != nullptr);

    SymbolIndex symbol_index = symtbl->isSymbolInTable(name);
    if (symbol_index == kInvalidSymbolIndex) return nullptr;

    std::vector<ObjectId> object_vector =
        symtbl->getReferences(symbol_index);
    for (auto iter = object_vector.begin(); iter != object_vector.end();
         iter++) {
        Constraint *region = addr<Constraint>(*iter);
        if (region && (region->getObjectType() == kObjectTypeRegion))
            return region;
    }

    return nullptr;
}

uint64_t Floorplan::getNumOfPlaceBlockages() const {
    if (place_blockages_ == 0) return 0;
    IdArray *obj_vector = addr<IdArray>(place_blockages_);
    if (obj_vector == nullptr) return 0;
    return obj_vector->getSize();
}
ObjectId Floorplan::getPlaceBlockages() const { return place_blockages_; }

Constraint *Floorplan::createRouteBlockage() {
    IdArray *id_array_ptr = nullptr;
    Constraint *cons = nullptr;

    if (route_blockages_ == 0) {
        route_blockages_ = __createObjectIdArray(32);
    }
    ediAssert(route_blockages_ != 0);
    id_array_ptr = addr<IdArray>(route_blockages_);
    if (id_array_ptr == nullptr) return nullptr;

    cons = Object::createObject<Constraint>(
              kObjectTypePhysicalConstraint, getOwnerId());
    ediAssert(cons != nullptr);
    cons->setFloorplan(this->getId());
    cons->setConstraintType(Constraint::kConstraintRBlkg);
    id_array_ptr->pushBack(cons->getId());

    return cons;
}
uint64_t Floorplan::getNumOfRouteBlockages() const {
    if (route_blockages_ == 0) return 0;
    IdArray *obj_vector = addr<IdArray>(route_blockages_);
    if (obj_vector == nullptr) return 0;
    return obj_vector->getSize();
}

ObjectId Floorplan::getRouteBlockages() const { return route_blockages_; }
// Constraint class
//
using ConstraintType = Constraint::ConstraintType;
using ConstraintSubType = Constraint::ConstraintSubType;

Constraint::Constraint() { type_ = kConstraintNone; }

Constraint::Constraint(const char *name, ConstraintType t /*, Shape &s*/) {
    StorageUtil *storage_util = getStorageUtil();
    ediAssert(storage_util != nullptr);
    SymbolTable *symtbl = storage_util->getSymbolTable();
    ediAssert(symtbl != nullptr);
    name_ = symtbl->getOrCreateSymbol(name);
    type_ = t;
}

Constraint::Constraint(SymbolIndex &name, ConstraintType t /*, Shape &s*/) {
    name_ = name;
    type_ = t;
}

/* TODO
Constraint::Constraint(Shape &s)
{
}
*/
Constraint::Constraint(Constraint::ConstraintType &t) { type_ = t; }

Constraint::~Constraint() {
    type_ = kConstraintNone;
}

SymbolIndex Constraint::getNameIndex() { return name_; }

std::string &Constraint::getName() {
    StorageUtil *storage_util = getStorageUtil();
    ediAssert(storage_util != nullptr);
    SymbolTable *symtbl = storage_util->getSymbolTable();
    ediAssert(symtbl != nullptr);  
    return symtbl->getSymbolByIndex(name_);
}

bool Constraint::setName(std::string &name) {
    return setName(name.c_str());
}

bool Constraint::setName(const char *name) {
    StorageUtil *storage_util = getStorageUtil();
    ediAssert(storage_util != nullptr);
    SymbolTable *symtbl = storage_util->getSymbolTable();
    ediAssert(symtbl != nullptr);
    SymbolIndex index = symtbl->getOrCreateSymbol(name);
    if (index == kInvalidSymbolIndex) return false;

    name_ = index;
    symtbl->addReference(name_, this->getId());
    return true;
}

Constraint::ConstraintType Constraint::getConstraintType() { return type_; }

void Constraint::setConstraintType(ConstraintType t) { type_ = t; }

ConstraintSubType Constraint::getConstraintSubType() { return sub_type_; }

void Constraint::setConstraintSubType(ConstraintSubType st) { sub_type_ = st; }

int32_t Constraint::getUtilization() { return utilization_; }

void Constraint::setUtilization(int32_t &u) { utilization_ = u; }

/* TODO
Shape Constraint::getShape()
{
}
void Constraint::setShape(Shape &s)
{
}
*/

/// @brief setFloorplan
///
/// @param fp
void Constraint::setFloorplan(ObjectId fp) { floorplan_ = fp; }

/// @brief getFloorplan
///
/// @return
Floorplan *Constraint::getFloorplan() { return addr<Floorplan>(floorplan_); }


/// @brief __createLayerGeometry
///
/// @param
void Constraint::__createLayerGeometry() {
    if (layer_geometry_id_ != 0) {
        return;
    }
    LayerGeometry *obj_data = Object::createObject<LayerGeometry>(
                    kObjectTypeLayerGeometry, getOwnerId());

    ediAssert(obj_data != nullptr);
    layer_geometry_id_ = obj_data->getId();
}

/// @brief __getLayerGeometry
///
/// @param
LayerGeometry* Constraint::__getLayerGeometry() const {
    if (layer_geometry_id_ == 0) {
        return nullptr;
    }
    LayerGeometry *obj_data = addr<LayerGeometry>(layer_geometry_id_);
    ediAssert(obj_data != nullptr);
    return obj_data;
}

/// @brief __getOrCreateLayerGeometry
///
/// @param
LayerGeometry* Constraint::__getOrCreateLayerGeometry() {
    __createLayerGeometry();
    return __getLayerGeometry();
}


// void Constraint::setHasLayer(bool has_layer) { has_layer_ = has_layer; }

bool Constraint::setLayer(const char *name) {
    LayerGeometry *layer_geom = __getOrCreateLayerGeometry();
    ediAssert(layer_geom != nullptr);
    layer_geom->setLayer(name);
    return true;
}

void Constraint::setPlacement(bool has_placement) {
    has_placement_ = has_placement;
}

void Constraint::setHasComponent(bool has_component) {
    has_component_ = has_component;
}

bool Constraint::setComponent(const char *name) {
    if (!name) {
        message->issueMsg("DB", 18, kError);
        return false;
    }
    Cell *owner_cell = addr<Cell>(getOwnerId());
    Inst *instance = owner_cell->getInstance(name);
    if (!instance) {
        message->issueMsg("DB", 19, kError, name);
        return false;
    }
    component_id_ = instance->getId();

    return true;
}

void Constraint::setSlots(bool has_slots) { has_slots_ = has_slots; }

void Constraint::setFills(bool has_fills) { has_fills_ = has_fills; }

void Constraint::setPushdown(bool has_pushdown) {
    has_pushdown_ = has_pushdown;
}

void Constraint::setSoft(bool has_soft) { has_soft_ = has_soft; }

void Constraint::setPartial(bool has_partial) { has_partial_ = has_partial; }

void Constraint::setDensity(double max_density) { max_density_ = max_density; }

void Constraint::setExceptpgnet(bool has_exceptpgnet) {
    LayerGeometry *layer_geom = __getOrCreateLayerGeometry();
    ediAssert(layer_geom != nullptr);
    layer_geom->setExceptPgNet(has_exceptpgnet);
    // has_exceptpgnet_ = has_exceptpgnet;
}

void Constraint::setMinSpacing(Bits64 min_spacing) {
    LayerGeometry *layer_geom = __getOrCreateLayerGeometry();
    ediAssert(layer_geom != nullptr);
    layer_geom->setMinSpacing(min_spacing);
    //min_spacing_ = min_spacing;
}

void Constraint::setEffectiveWidth(Bits64 effective_width) {
    LayerGeometry *layer_geom = __getOrCreateLayerGeometry();
    ediAssert(layer_geom != nullptr);
    layer_geom->setDesignRuleWidth(effective_width);
    //effective_width_ = effective_width;
}

//void Constraint::setMask(bool has_mask) { has_mask_ = has_mask; }
void Constraint::setMaskNum(Bits64 mask_num) {
    LayerGeometry *layer_geom = __getOrCreateLayerGeometry();
    ediAssert(layer_geom != nullptr);
    layer_geom->setMaskNum(mask_num);
    //mask_num_ = mask_num; 
}

//Starting hasXXX() functions:
bool Constraint::hasPlacement() const { return has_placement_; }
bool Constraint::hasComponent() const { return has_component_; }
bool Constraint::hasSlots() const { return has_slots_; }
bool Constraint::hasFills() const { return has_fills_; }
bool Constraint::hasPushdown() const { return has_pushdown_; }
// 5.7
bool Constraint::hasSoft() const { return has_soft_; }
// 5.7
bool Constraint::hasPartial() const { return has_partial_; }

// 5.7
bool Constraint::hasExceptpgnet() const {
    LayerGeometry *obj_data = __getLayerGeometry();
    if (obj_data) {
        return obj_data->isExceptPgNet();
    }
    return false;  
    //return has_exceptpgnet_;
}

bool Constraint::hasLayer() const {
    LayerGeometry *obj_data = __getLayerGeometry();
    if (obj_data) {
        return obj_data->isLayer();
    }
    return false;
    //return has_layer_; 
}

bool Constraint::hasSpacing() const {
    LayerGeometry *obj_data = __getLayerGeometry();
    if (obj_data) {
        return obj_data->hasSpacing();
    }
    return false;    
    //return has_spacing_; 
}

bool Constraint::hasDesignRuleWidth() const {
    LayerGeometry *obj_data = __getLayerGeometry();
    if (obj_data) {
        return obj_data->hasDRW();
    }
    return false;
  //return has_design_rule_width_; 
}

bool Constraint::hasMask() const {
    LayerGeometry *obj_data = __getLayerGeometry();
    if (obj_data) {
        return (obj_data->getMaskNum() != 0);
    }
    return false;
    // return has_mask_;
}

//Starting getXXX() functions:
// 5.7
double Constraint::getPlacementMaxDensity() const { return max_density_; }

/// @brief component get component pointer
///
/// @return
Inst *Constraint::getComponent() const { return addr<Inst>(component_id_); }

Bits64 Constraint::getMinSpacing() const {
    LayerGeometry *obj_data = __getLayerGeometry();
    if (obj_data) {
        return obj_data->getMinSpacing();
    }
    return kInvalidLengthValue;
    //return min_spacing_; 
}

Bits64 Constraint::getEffectiveWidth() const {
    LayerGeometry *obj_data = __getLayerGeometry();
    if (obj_data) {
        return obj_data->getDesignRuleWidth();
    }
    return kInvalidLengthValue;    
    //return effective_width_; 
}

Bits64 Constraint::getMaskNum() const {
    LayerGeometry *obj_data = __getLayerGeometry();
    if (obj_data) {
        return obj_data->getMaskNum();
    }
    return 0;
    // return mask_num_;
}

Layer *Constraint::__getLayer() const {
    LayerGeometry *obj_data = __getLayerGeometry();
    if (obj_data) {
        return obj_data->getLayer();
    }
    return nullptr;
}

ObjectIndex Constraint::getLayerId() const {
    Layer *layer = __getLayer();
    if (layer) {
        return layer->getIndexInLef();
    }
    return kInvalidLayerIndex;
}

const char* Constraint::getLayerName() const {
    Layer *layer = __getLayer();
    if (layer) {
        return layer->getName();
    }
    return nullptr;
}

ObjectId Constraint::getComponentId() const { return component_id_; }

std::string Constraint::getComponentName() const { 
    Inst *component_inst = getComponent();
    if (component_inst) {
        return component_inst->getName();
    }
    std::string tmp("");
    return tmp;
}

void Constraint::createBox(int64_t xl, int64_t yl, int64_t xh, int64_t yh) {
    Geometry *geom = Object::createObject<Geometry>(
                      kObjectTypeGeometry, getOwnerId());
    ediAssert(geom != nullptr);
    Box box(xl, yl, xh, yh);
    geom->setBox(&box);
    LayerGeometry *layer_geom = __getOrCreateLayerGeometry();
    layer_geom->addGeometry(geom->getId());
}

/// @brief addPolygon
///
/// @param polygon:
///
/// @return
void Constraint::addPolygon(Polygon *polygon) {
    ediAssert(polygon != nullptr);
    Geometry *geom = Object::createObject<Geometry>(kObjectTypeGeometry, getOwnerId());
    ediAssert(geom != nullptr);
    geom->setPolygon(polygon);
    LayerGeometry *layer_geom = __getOrCreateLayerGeometry();
    layer_geom->addGeometry(geom->getId());
}

/// @brief printBlockage
///
/// @return
void Constraint::printBlockage() const {
    LayerGeometry *layer_geom = __getLayerGeometry();

    if (hasLayer() && layer_geom != nullptr) {
        message->info("    - LAYER %s", getLayerName());
        if (hasSlots()) message->info(" + SLOTS");
        if (hasFills()) message->info(" + FILLS");
        if (hasPushdown()) message->info(" + PUSHDOWN");
        if (hasExceptpgnet()) message->info(" + EXCEPTPGNET");
        if (hasComponent()) {
            message->info(" + COMPONENT %s", getComponentName().c_str());
        }
        if (hasSpacing()) {
            message->info(" + SPACING %d", getMinSpacing());
        }
        if (hasDesignRuleWidth()) {
            message->info(" + DESIGNRULEWIDTH %d", getEffectiveWidth());
        }
        if (hasMask()) {
            message->info(" + MASK %d", getMaskNum());
        }
        message->info("\n");
    }
    if (hasPlacement()) {
        message->info("    - PLACEMENT");
        if (hasSoft()) message->info(" + SOFT");
        if (hasPartial())
            message->info(" + PARTIAL %.2f", getPlacementMaxDensity());
        if (hasPushdown()) message->info(" + PUSHDOWN");
        if (hasComponent()) {
            message->info(" + COMPONENT %s", getComponentName().c_str());
        }
        message->info("\n");
    }
    //print boxes:
    if (layer_geom != nullptr) {
        bool has_one = 0;
        auto iter_box = layer_geom->getBoxIter();
        
        for (Box *box = iter_box.getNext(); box != nullptr;
              box = iter_box.getNext()) {
            if (has_one) {
                message->info("\n");
            } else {
                has_one = 1;
            }
            message->info("        RECT ( %d %d ) ( %d %d )",
                          box->getLLX(), box->getLLY(), box->getURX(),
                          box->getURY());
            
        }
        if (has_one) {
            message->info(";\n");
        }
    }
    // print polygons:
    if (layer_geom != nullptr) {
        auto iter_polygon = layer_geom->getPolygonIter();
        
        for (Polygon *polygon = iter_polygon.getNext(); polygon != nullptr;
              polygon = iter_polygon.getNext()) {      
            message->info("        POLYGON");
            for (int index = 0; index < polygon->getNumPoints(); ++index) {
                Point point = polygon->getPoint(index);
                message->info(" ( %d %d )", point.getX(), point.getY());
            }
            message->info(" ;\n");
        }
    }
}

void Constraint::printBlockage(IOManager &io_manager) const {
    LayerGeometry *layer_geom = __getLayerGeometry();

    if (hasLayer() && layer_geom != nullptr) {
        io_manager.write("    - LAYER %s", getLayerName());
        if (hasSlots()) {
            io_manager.write(" + SLOTS");
        }
        if (hasFills()) {
            io_manager.write(" + FILLS");
        }
        if (hasPushdown()) {
            io_manager.write(" + PUSHDOWN");
        }
        if (hasExceptpgnet()) {
            io_manager.write(" + EXCEPTPGNET");
        }
        if (hasComponent()) {
            io_manager.write(" + COMPONENT " + getComponentName());
        }
        if (hasSpacing()) {
            io_manager.write(" + SPACING %d", getMinSpacing());
        }
        if (hasDesignRuleWidth()) {
            io_manager.write(" + DESIGNRULEWIDTH %d",
                getEffectiveWidth());
        }
        if (hasMask()) {
            io_manager.write(" + MASK %d", getMaskNum());
        }
        io_manager.write("\n");
    }
    if (hasPlacement()) {
        io_manager.write("    - PLACEMENT");
        if (hasSoft()) {
            io_manager.write(" + SOFT");
        }
        if (hasPartial()) {
            io_manager.write(" + PARTIAL %.2f", getPlacementMaxDensity());
        }
        if (hasPushdown()) {
            io_manager.write(" + PUSHDOWN");
        }
        if (hasComponent()) {
            io_manager.write(" + COMPONENT " + getComponentName());
        }
        io_manager.write("\n");
    }

    //print boxes:
    if (layer_geom != nullptr) {
        bool has_one = 0;
        auto iter_box = layer_geom->getBoxIter();
        
        for (Box *box = iter_box.getNext(); box != nullptr;
              box = iter_box.getNext()) {        
            if (has_one) {
                io_manager.write("\n");
            } else {
                has_one = 1;
            }
            io_manager.write("        RECT ( %d %d ) ( %d %d )",
                box->getLLX(), box->getLLY(),
                box->getURX(), box->getURY());
        }
        if (has_one) {
            io_manager.write(" ;\n");
        }
    }
    // print polygons:
    if (layer_geom != nullptr) {
        auto iter_polygon = layer_geom->getPolygonIter();
        
        for (Polygon *polygon = iter_polygon.getNext(); polygon != nullptr;
              polygon = iter_polygon.getNext()) {      
            io_manager.write("        POLYGON");
            for (int index = 0; index < polygon->getNumPoints(); ++index) {
                Point point = polygon->getPoint(index);
                io_manager.write(" ( %d %d )",
                    point.getX(), point.getY());
            }
            io_manager.write(" ;\n");
        }
    }
}

void Constraint::printRegion(IOManager &io_manager) {
    std::string name = getName();
    io_manager.write("- %s", name.c_str());
    //print boxes:
    LayerGeometry *layer_geom = __getLayerGeometry();
    if (layer_geom != nullptr) {
        auto iter_box = layer_geom->getBoxIter();
        
        for (Box *box = iter_box.getNext(); box != nullptr;
              box = iter_box.getNext()) {      
            if (!box) {
                continue;
            }
            io_manager.write(" ( %d %d ) ( %d %d )",
                box->getLLX(), box->getLLY(),
                box->getURX(), box->getURY());
        }
    }
    
    io_manager.write("\n");
    if (getConstraintSubType() == ConstraintSubType::kRegionFence) {
        io_manager.write("  + TYPE FENCE");
    } else if (getConstraintSubType() == ConstraintSubType::kRegionGuide) {
        io_manager.write("  + TYPE GUIDE");
    }

    Cell *top_cell = getTopCell();
    if (top_cell) {
        PropertyManager* table = top_cell->getPropertyManager(); 
        std::vector<std::string> properties;
        table->getSparsePropertyList<Constraint>(this, properties);
        if (properties.size() > 0) {
            for(int i = 0; i < properties.size(); i++) {
                Property* prop = table->getSparseProperty<Constraint>(this, properties.at(i).c_str());
                prop->printDEF(io_manager);
            }
        }
    }
    io_manager.write(" ;\n");
}
}  // namespace db
}  // namespace open_edi
