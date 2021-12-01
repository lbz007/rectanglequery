/* @file  term.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "db/core/term.h"

#include "db/core/cell.h"
#include "db/core/db.h"
#include "db/util/symbol_table.h"
#include "db/util/array.h"
#include "util/util.h"
#include "db/util/transform.h"

namespace open_edi {
namespace db {
using IdArray = ArrayObject<ObjectId>;

PinAccess::PinAccess() {
    via_master_ = 0;
    term_ = 0;
    layer_no_ = 0;
    access_type_ = AccessType::kOnAccessGrid;
    orient_ = 0;
    access_style_ = AccessStyle::kWirePinAccess;
    set_ = 0;
    is_preferred_ = 0;
}

PinAccess::~PinAccess() {
}

Point PinAccess::getPosition() {
    Point&& pos = __tansformPoint(pos_);
    return pos;
}

Box PinAccess::getBoundingBox() {
    ViaMaster* via_master = addr<ViaMaster>(via_master_);
    if (!via_master_) {
        return Box(0, 0, 0, 0);
    }
    Box&& rect   = via_master->getBoundingBox();
    Box&& bb_box = __calculateBBox(rect);
    return bb_box;
}

Box PinAccess::getBotBox() {
    ViaMaster* via_master = addr<ViaMaster>(via_master_);
    if (!via_master_) {
        return Box(0, 0, 0, 0);
    }
    Box&& rect   = via_master->getBotBox();
    Box&& bb_box = __calculateBBox(rect);
    return bb_box;
}

Box PinAccess::getTopBox() {
    ViaMaster* via_master = addr<ViaMaster>(via_master_);
    if (!via_master_) {
        return Box(0, 0, 0, 0);
    }
    Box&& rect   = via_master->getTopBox();
    Box&& bb_box = __calculateBBox(rect);
    return bb_box;
}

Box PinAccess::__calculateBBox(Box& rect) {

    Point&& point = __tansformPoint(pos_);


    //add center post offset
    Box bb_box(point.getX() + rect.getLLX(),
               point.getY() + rect.getLLY(),
               point.getX() + rect.getURX(),
               point.getY() + rect.getURY());


    return bb_box;
}

Point PinAccess::__tansformPoint(Point& placed_pt) {

    Cell* cell = getCell();
    if (!cell) {
        return Point(placed_pt.getX(), placed_pt.getY());
    }

    int size_x = cell->getSizeX();
    int size_y = cell->getSizeY();

    Point origin;
    origin.setX(cell->getOriginX());
    origin.setY(cell->getOriginY());

    Orient orient = static_cast<Orient>(orient_);

    Transform transform;
    Point     point = placed_pt;
    transform.transform(point, orient, origin, size_x, size_y);
    return point;
}

Box PinAccess::__tansformBox(Box& rect) 
{
    Cell* cell = getCell();
    if (!cell) {
        return Box(rect.getLLX(), rect.getLLY(), rect.getURX(), rect.getURY());
    }

    int size_x = cell->getSizeX();
    int size_y = cell->getSizeY();

    Point origin;
    origin.setX(cell->getOriginX());
    origin.setY(cell->getOriginY());

    Orient orient = static_cast<Orient>(orient_);

    Transform transform;

    Box box = rect;

    transform.transform(box, orient, origin, size_x, size_y);

    return box;
}



Box PinAccess::getPatchBox() {

    Box patch = __tansformBox(patch_);

    return patch;
}

Cell* PinAccess::getCell() {
    Term* term = getTerm();
    return term->getCell();
}

Term* PinAccess::getTerm() {
    return addr<Term>(term_);
}

Layer* PinAccess::getLayer() {
    Tech* lib = getTechLib();
    ediAssert(lib != nullptr);
    return lib->getLayer(layer_no_);
}

ViaMaster* PinAccess::getViaMaster() {
    ViaMaster* via_master = addr<ViaMaster>(via_master_);
    return via_master;
}

AccessStyle PinAccess::getAccessStyle() {
    return static_cast<AccessStyle>(access_style_);
}

AccessType PinAccess::getAccessType() {
    return static_cast<AccessType>(access_type_);
}

unsigned char PinAccess::getSet() {
    return set_;
}

Orient PinAccess::getOrient() {
    return static_cast<Orient>(orient_);
}

bool PinAccess::getIsOnAccessGrid() {
    auto v = access_type_ & (1 << 0);
    return (bool)v;
}

bool PinAccess::getIsOnPinGrid() {
    auto v = access_type_ & (1 << 1);
    return (bool)v;
}

bool PinAccess::getIsNearBoundary() {
    auto v = access_type_ & (1 << 2);
    return (bool)v;
}

bool PinAccess::getIsEolBoundary() {
    auto v = access_type_ & (1 << 3);
    return (bool)v;
}

bool PinAccess::getIsSubsumed() {
    auto v = access_type_ & (1 << 4);
    return (bool)v;
}

bool PinAccess::getIsWirePinAccess() {
    auto v = access_style_ & (1 << 0);
    return (bool)v;
}

bool PinAccess::getIsViaPinAccess() {
    auto v = access_style_ & (1 << 1);
    return (bool)v;
}

bool PinAccess::getIsPreferred() {
    return (bool)is_preferred_;
}

void PinAccess::setIsOnAccessGrid(bool v) {
    int value;
    if (v) {
        value = access_type_ | (1 << 0);
    } else {
        value = access_type_ & (~(1 << 0));
    }
    access_type_ = static_cast<AccessType>(value);
}

void PinAccess::setIsOnPinGrid(bool v) {
    int value;
    if (v) {
        value = access_type_ | (1 << 1);
    } else {
        value = access_type_ & (~(1 << 1));
    }
    access_type_ = static_cast<AccessType>(value);
}

void PinAccess::setIsNearBoundary(bool v) {
    int value;
    if (v) {
        value = access_type_ | (1 << 2);
    } else {
        value = access_type_ & (~(1 << 2));
    }
    access_type_ = static_cast<AccessType>(value);
}

void PinAccess::setIsEolBoundary(bool v) {
    int value;
    if (v) {
        value = access_type_ | (1 << 3);
    } else {
        value = access_type_ & (~(1 << 3));
    }
    access_type_ = static_cast<AccessType>(value);
}

void PinAccess::setIsSubsumed(bool v) {
    int value;
    if (v) {
        value = access_type_ | (1 << 4);
    } else {
        value = access_type_ & (~(1 << 4));
    }
    access_type_ = static_cast<AccessType>(value);
}

void PinAccess::setIsWirePinAccess(bool v) {
    int value;
    if (v) {
        value = access_style_ | (1 << 0);
    } else {
        value = access_style_ & (~(1 << 0));
    }
    access_style_ = static_cast<AccessStyle>(value);
}

void PinAccess::setIsViaPinAccess(bool v) {
    int value;
    if (v) {
        value = access_style_ | (1 << 1);
    } else {
        value = access_style_ & (~(1 << 1));
    }
    access_style_ = static_cast<AccessStyle>(value);
}

void PinAccess::setIsPreferred(bool v) {
    is_preferred_ = v;
}

void PinAccess::setOrient(Orient o) {
    orient_ = static_cast<Bits8>(o);
}

void PinAccess::setAccessStyle(AccessStyle s) {
    access_style_ = s;
}

void PinAccess::setAccessType(AccessType t) {
    access_type_ = t;
}

void PinAccess::setTerm(Term* term) {
    term_ = term->getId();
}

void PinAccess::setPatchBox(Box& box) {
    patch_ = box;
}

void PinAccess::setPosition(Point& pos) {
    pos_ = pos;
}

void PinAccess::setViaMaster(ViaMaster* vm) {
    via_master_ = vm->getId();
}

void PinAccess::setLayer(Layer* l) {

    Tech* lib = getTopCell()->getTechLib();
    ediAssert(lib != nullptr);
    ObjectIndex index = lib->getLayerLEFIndexByName(l->getName());
    layer_no_         = static_cast<uint8_t>(index);
}

void PinAccess::setSet(unsigned char s) {
    set_ = s;
}

Term::Term() : Term::BaseType() {
    name_index_ = 0;
    ports_ = 0;
    cell_id_ = 0;
    has_range_ = false;
    antenna_partial_metal_areas_ = 0;
    antenna_partial_metal_side_areas_ = 0;
    antenna_partial_cut_areas_ = 0;
    antenna_diff_areas_ = 0;
    setDirection(SignalDirection::kUnknown);
    setType(SignalType::kUnknown);
}

Term::Term(Object *owner, Term::IndexType id) : Term::BaseType(owner, id) {
    setDirection(SignalDirection::kUnknown);
    setType(SignalType::kUnknown);
}

Term::Term(Term const &rhs) { 
    copy(rhs);
    cell_id_ = rhs.cell_id_;
}

Term::Term(Term &&rhs) noexcept { move(std::move(rhs)); }

Term::~Term() {
    for (int i = 0; i < kMaxOrientationNum; ++i) {
        for (int j = 0; j < kMaxSetNum; ++j) {
            for (unsigned int k = 0; k < accesses_[i][j].size(); ++k) {
                if (accesses_[i][j][k]) delete accesses_[i][j][k];
            }
            accesses_[i][j].clear();
        }
    }
}

void Term::setName(std::string name) {
    Cell *owner_cell = getCell();
    if (!owner_cell) {
        message->issueMsg("DB", 24, kError, name.c_str());
        return;
    }
    name_index_ = owner_cell->getOrCreateSymbol(name);
    owner_cell->addSymbolReference(name_index_, this->getId());
}

std::string &Term::getName() const {
    Cell *owner_cell = getCell();
    return owner_cell->getSymbolTable()->getSymbolByIndex(name_index_);
}

Term &Term::operator=(Term const &rhs) {
    if (this != &rhs) {
        copy(rhs);
        cell_id_ = rhs.cell_id_;
    }
    return *this;
}

Term &Term::operator=(Term &&rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

void Term::copy(Term const &rhs) {
    this->BaseType::copy(rhs);
    cell_id_ = rhs.cell_id_;
}

void Term::move(Term &&rhs) { this->BaseType::move(std::move(rhs)); }

Term::IndexType Term::memory() const { return this->BaseType::memory(); }

OStreamBase &operator<<(OStreamBase &os, Term const &rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");

    Term::BaseType const &base = rhs;
    os << base << DataDelimiter();

    os << DataFieldName("name_") << rhs.getName();

    os << DataEnd(")");
    return os;
}

IStreamBase &operator>>(IStreamBase &is, Term &rhs) {
    is >> DataTypeName(className(rhs)) >> DataBegin("(");

    Term::BaseType &base = rhs;
    is >> base >> DataDelimiter();

    // read name_
    std::string name;
    is >> DataFieldName("name_") >> name;
    rhs.setName(name);

    is >> DataEnd(")");
    return is;
}

std::string const &Term::getTaperRule() const {
    Cell *owner_cell = getCell();
    return owner_cell->getSymbolTable()->getSymbolByIndex(taper_rule_index_);
}

void Term::setTaperRule(const char *v) {
    Cell *owner_cell = getCell();
    if (!owner_cell) {
        message->issueMsg("DB", 25, kError);
        return;
    }
    taper_rule_index_ = owner_cell->getOrCreateSymbol(v);
    owner_cell->addSymbolReference(taper_rule_index_, this->getId());
}

bool Term::hasDirection() const { 
    if (direction_ != SignalDirection::kUnknown) {
        return true;
    }
    return false;
}

bool Term::isInput() {
    return (direction_ == SignalDirection::kInput);
}

bool Term::isOutput() {
    return (direction_ == SignalDirection::kOutput);
}

bool Term::isInOut() {
    return (direction_ == SignalDirection::kInout);
}

bool Term::isFeedthru() {
    return (direction_ == SignalDirection::kFeedThrough);
}

void Term::setDirection(const char *v) {
    direction_ = toEnumByString<SignalDirection>(v);
}

void Term::setDirection(SignalDirection v) {
    direction_ = v;
}

/// @brief getDirection 
///
/// @return 
SignalDirection Term::getDirection() const {
    return direction_;
}

std::string Term::getDirectionStr() const {
    return toString(direction_);
}

bool Term::hasUse() const { 
    if (type_ != SignalType::kUnknown) {
        return true;
    }
    return false;
}

void Term::setUse(const char *v) {
    setTypeStr(v);
}

std::string Term::getTypeStr() const {
    return toString(type_);
}

SignalType Term::getType() const {
    return type_;
}

void Term::setType(SignalType v) {
    type_ = v;
}

void Term::setTypeStr(const char *v) {
    type_ = toEnumByString<SignalType>(v);
}

bool Term::isPGType() const {
    if (type_ == SignalType::kPower || type_ == SignalType::kGround) {
        return true;
    }
    return false;
}

//SymbolIndex Term::getNetExprIndex() const { return net_expr_index_; }

std::string Term::getNetExpr() const {
     Cell *owner_cell = getTopCell();
    // return owner_cell->getSymbolTable()->getSymbolByIndex(net_expr_index_);
    PropertyManager* table = owner_cell->getPropertyManager();
    if (table->isPropertySet<Term>(this, "netExpr")) {
        //std::cout << "pin " << getName() << " get net expr " << table->getPropertyString<Term>(this, "netExpr") << std::endl;
        return table->getPropertyString<Term>(this, "netExpr");
    } else {
        return std::string("");
    }
}

void Term::setNetExpr(const char *v) {
    Cell *owner_cell = getCell();
    if (!owner_cell) {
        message->issueMsg("DB", 25, kError);
        return;
    }

    PropertyManager* table = owner_cell->getPropertyManager();
    if (!table->isPropertyDefined("netExpr")) {
       table->createProperty("netExpr", kPropSparse, kPropLef, kPropPersistent, PropDataType::kString); 
    }
    table->setProperty<Term>(this, "netExpr", v);
}

SymbolIndex Term::getSupplySensitivityIndex() const {
    return supply_sensitivity_index_;
}

std::string const &Term::getSupplySensitivity() const {
    Cell *owner_cell = getCell();
    return owner_cell->getSymbolTable()->getSymbolByIndex(
        supply_sensitivity_index_);
}

void Term::setSupplySensitivity(const char *v) {
    Cell *owner_cell = getCell();
    if (!owner_cell) {
        message->issueMsg("DB", 25, kError);
        return;
    }
    supply_sensitivity_index_ = owner_cell->getOrCreateSymbol(v);
    // shouldn't set the reference to the term.
    // this is a name of power pin.
    // owner_cell->addSymbolReference(supply_sensitivity_index_, this->getId());
}

SymbolIndex Term::getGroundSensitivityIndex() const {
    return ground_sensitivity_index_;
}

std::string const &Term::getGroundSensitivity() const {
    Cell *owner_cell = getCell();
    return owner_cell->getSymbolTable()->getSymbolByIndex(
        ground_sensitivity_index_);
}

void Term::setGroundSensitivity(const char *v) {
    Cell *owner_cell = getCell();
    if (!owner_cell) {
        message->issueMsg("DB", 25, kError);
        return;
    }
    ground_sensitivity_index_ = owner_cell->getOrCreateSymbol(v);
    // shouldn't set the reference to the term.
    // this is a name of ground pin.    
    // owner_cell->addSymbolReference(ground_sensitivity_index_, this->getId());
}

std::string const &Term::getMustjoin() const {
    Cell *owner_cell = getCell();
    return owner_cell->getSymbolTable()->getSymbolByIndex(mustjoin_index_);
}

void Term::setMustjoin(const char *v) {
    Cell *owner_cell = getCell();
    if (!owner_cell) {
        message->issueMsg("DB", 25, kError);
        return;
    }
    mustjoin_index_ = owner_cell->getOrCreateSymbol(v);
    // shouldn't set the reference to the term.
    // this is a name of must-join pin.    
    // owner_cell->addSymbolReference(mustjoin_index_, this->getId());
}

std::string const &Term::getShape() const {
    Cell *owner_cell = getCell();
    return owner_cell->getSymbolTable()->getSymbolByIndex(shape_index_);
}

void Term::setShape(const char *v) {
    Cell *owner_cell = getCell();
    if (!owner_cell) {
        message->issueMsg("DB", 25, kError);
        return;
    }
    shape_index_ = owner_cell->getOrCreateSymbol(v);

    // owner_cell->addSymbolReference(shape_index_, this->getId());
}

void Term::addAntennaModelTerm(int index, AntennaModelTerm *am) {
    memcpy(&antenna_models_[index], am, sizeof(AntennaModelTerm));
    antenna_models_[index].setValid(true);
}

void AntennaArea::setLayerByName(const char *value) {
    Tech *lib = getTopCell()->getTechLib();
    ediAssert(lib != nullptr);
    ObjectIndex index = lib->getLayerLEFIndexByName(value);
    if (index == kInvalidLayerIndex) {
        message->issueMsg("DB", 26, kError, value);
        return;
    }
    if (index > 255) {
        message->issueMsg("DB", 27, kError, index);
        return;
    }
    
    layer_idx_ = index;
}

Layer* AntennaArea::getLayer() const {
    Tech *lib = getTopCell()->getTechLib();
    ediAssert(lib != nullptr);
    return lib->getLayer(layer_idx_);
}

void AntennaModelTerm::addAntennaGateArea(ObjectId aa) {
    IdArray *id_array_ptr = nullptr;
    if (antenna_gate_areas_ == 0) {
        antenna_gate_areas_ = __createObjectIdArray(32);
    }
    if (antenna_gate_areas_ == 0) return;
    id_array_ptr = Object::addr<IdArray>(antenna_gate_areas_);
    if (id_array_ptr) id_array_ptr->pushBack(aa);
}

AntennaArea *AntennaModelTerm::getAntennaGateArea(int index) const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_gate_areas_ == 0)
        return nullptr;
    else
        id_array_ptr = Object::addr<IdArray>(antenna_gate_areas_);
    if (id_array_ptr) {
        AntennaArea *obj_data =
            Object::addr<AntennaArea>((*id_array_ptr)[index]);
        if (obj_data) {
            return obj_data;
        }
    }
    return nullptr;
}

int AntennaModelTerm::getAntennaGateAreaNum() const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_gate_areas_ == 0)
        return 0;
    else
        id_array_ptr = Object::addr<IdArray>(antenna_gate_areas_);
    if (id_array_ptr)
        return id_array_ptr->getSize();
    else
        return 0;
}

void AntennaModelTerm::addAntennaMaxAreaCar(ObjectId aa) {
    IdArray *id_array_ptr = nullptr;
    if (antenna_max_area_cars_ == 0) {
        antenna_max_area_cars_ = __createObjectIdArray(32);
    }
    if (antenna_max_area_cars_ == 0) return;
    id_array_ptr = Object::addr<IdArray>(antenna_max_area_cars_);
    if (id_array_ptr) id_array_ptr->pushBack(aa);
}

AntennaArea *AntennaModelTerm::getAntennaMaxAreaCar(int index) const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_max_area_cars_ == 0)
        return nullptr;
    else
        id_array_ptr = Object::addr<IdArray>(antenna_max_area_cars_);
    if (id_array_ptr) {
        AntennaArea *obj_data =
            Object::addr<AntennaArea>((*id_array_ptr)[index]);
        if (obj_data) {
            return obj_data;
        }
    }
    return nullptr;
}

int AntennaModelTerm::getAntennaMaxAreaCarNum() const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_max_area_cars_ == 0)
        return 0;
    else
        id_array_ptr = Object::addr<IdArray>(antenna_max_area_cars_);
    if (id_array_ptr)
        return id_array_ptr->getSize();
    else
        return 0;
}

void AntennaModelTerm::addAntennaMaxSideAreaCar(ObjectId aa) {
    IdArray *id_array_ptr = nullptr;
    if (antenna_max_side_area_cars_ == 0) {
        antenna_max_side_area_cars_ = __createObjectIdArray(32);
    }
    if (antenna_max_side_area_cars_ == 0) return;
    id_array_ptr = Object::addr<IdArray>(antenna_max_side_area_cars_);
    if (id_array_ptr) id_array_ptr->pushBack(aa);
}

AntennaArea *AntennaModelTerm::getAntennaMaxSideAreaCar(int index) const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_max_side_area_cars_ == 0)
        return nullptr;
    else
        id_array_ptr =
            Object::addr<IdArray>(antenna_max_side_area_cars_);
    if (id_array_ptr) {
        AntennaArea *obj_data =
            Object::addr<AntennaArea>((*id_array_ptr)[index]);
        if (obj_data) {
            return obj_data;
        }
    }
    return nullptr;
}

int AntennaModelTerm::getAntennaMaxSideAreaCarNum() const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_max_side_area_cars_ == 0)
        return 0;
    else
        id_array_ptr =
           Object:: addr<IdArray>(antenna_max_side_area_cars_);
    if (id_array_ptr)
        return id_array_ptr->getSize();
    else
        return 0;
}

void AntennaModelTerm::addAntennaMaxCutCar(ObjectId aa) {
    IdArray *id_array_ptr = nullptr;
    if (antenna_max_cut_cars_ == 0) {
        antenna_max_cut_cars_ = __createObjectIdArray(32);
    }
    if (antenna_max_cut_cars_ == 0) return;
    id_array_ptr = Object::addr<IdArray>(antenna_max_cut_cars_);
    if (id_array_ptr) id_array_ptr->pushBack(aa);
}

AntennaArea *AntennaModelTerm::getAntennaMaxCutCar(int index) const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_max_cut_cars_ == 0)
        return nullptr;
    else
        id_array_ptr = Object::addr<IdArray>(antenna_max_cut_cars_);
    if (id_array_ptr) {
        AntennaArea *obj_data =
            Object::addr<AntennaArea>((*id_array_ptr)[index]);
        if (obj_data) {
            return obj_data;
        }
    }
    return nullptr;
}

int AntennaModelTerm::getAntennaMaxCutCarNum() const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_max_cut_cars_ == 0)
        return 0;
    else
        id_array_ptr = Object::addr<IdArray>(antenna_max_cut_cars_);
    if (id_array_ptr)
        return id_array_ptr->getSize();
    else
        return 0;
}

void Term::addAntennaPartialMetalArea(ObjectId aa) {
    IdArray *id_array_ptr = nullptr;
    if (antenna_partial_metal_areas_ == 0) {
        antenna_partial_metal_areas_ = __createObjectIdArray(32);
    }
    if (antenna_partial_metal_areas_ == 0) return;
    id_array_ptr = Object::addr<IdArray>(antenna_partial_metal_areas_);
    if (id_array_ptr) id_array_ptr->pushBack(aa);
}

AntennaArea *Term::getAntennaPartialMetalArea(int index) const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_partial_metal_areas_ == 0)
        return nullptr;
    else
        id_array_ptr =
            addr<IdArray>(antenna_partial_metal_areas_);
    if (id_array_ptr) {
        AntennaArea *obj_data =
            addr<AntennaArea>((*id_array_ptr)[index]);
        if (obj_data) {
            return obj_data;
        }
    }
    return nullptr;
}

int Term::getAntennaPartialMetalAreaNum() const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_partial_metal_areas_ == 0)
        return 0;
    else
        id_array_ptr =
            addr<IdArray>(antenna_partial_metal_areas_);
    if (id_array_ptr)
        return id_array_ptr->getSize();
    else
        return 0;
}

void Term::addAntennaPartialMetalSideArea(ObjectId aa) {
    IdArray *id_array_ptr = nullptr;
    if (antenna_partial_metal_side_areas_ == 0) {
        antenna_partial_metal_side_areas_ = __createObjectIdArray(32);
    }
    if (antenna_partial_metal_side_areas_ == 0) return;
    id_array_ptr = Object::addr<IdArray>(antenna_partial_metal_side_areas_);
    if (id_array_ptr) id_array_ptr->pushBack(aa);
}

AntennaArea *Term::getAntennaPartialMetalSideArea(int index) const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_partial_metal_side_areas_ == 0)
        return nullptr;
    else
        id_array_ptr = addr<IdArray>(
            antenna_partial_metal_side_areas_);
    if (id_array_ptr) {
        AntennaArea *obj_data =
            addr<AntennaArea>((*id_array_ptr)[index]);
        if (obj_data) {
            return obj_data;
        }
    }
    return nullptr;
}

int Term::getAntennaPartialMetalSideAreaNum() const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_partial_metal_side_areas_ == 0)
        return 0;
    else
        id_array_ptr = addr<IdArray>(
            antenna_partial_metal_side_areas_);
    if (id_array_ptr)
        return id_array_ptr->getSize();
    else
        return 0;
}

void Term::addAntennaPartialCutArea(ObjectId aa) {
    IdArray *id_array_ptr = nullptr;
    if (antenna_partial_cut_areas_ == 0) {
        antenna_partial_cut_areas_ = __createObjectIdArray(32);
    }
    if (antenna_partial_cut_areas_ == 0) return;
    id_array_ptr = Object::addr<IdArray>(antenna_partial_cut_areas_);
    if (id_array_ptr) id_array_ptr->pushBack(aa);
}

AntennaArea *Term::getAntennaPartialCutArea(int index) const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_partial_cut_areas_ == 0)
        return nullptr;
    else
        id_array_ptr = addr<IdArray>(antenna_partial_cut_areas_);
    if (id_array_ptr) {
        AntennaArea *obj_data =
            addr<AntennaArea>((*id_array_ptr)[index]);
        if (obj_data) {
            return obj_data;
        }
    }
    return nullptr;
}

int Term::getAntennaPartialCutAreaNum() const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_partial_cut_areas_ == 0)
        return 0;
    else
        id_array_ptr = addr<IdArray>(antenna_partial_cut_areas_);
    if (id_array_ptr)
        return id_array_ptr->getSize();
    else
        return 0;
}

void Term::addAntennaDiffArea(ObjectId aa) {
    IdArray *id_array_ptr = nullptr;
    if (antenna_diff_areas_ == 0) {
        antenna_diff_areas_ = __createObjectIdArray(32);
    }
    if (antenna_diff_areas_ == 0) return;
    id_array_ptr = Object::addr<IdArray>(antenna_diff_areas_);
    if (id_array_ptr) id_array_ptr->pushBack(aa);
}

AntennaArea *Term::getAntennaDiffArea(int index) const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_diff_areas_ == 0)
        return nullptr;
    else
        id_array_ptr = addr<IdArray>(antenna_diff_areas_);
    if (id_array_ptr) {
        AntennaArea *obj_data =
            addr<AntennaArea>((*id_array_ptr)[index]);
        if (obj_data) {
            return obj_data;
        }
    }
    return nullptr;
}

int Term::getAntennaDiffAreaNum() const {
    IdArray *id_array_ptr = nullptr;
    if (antenna_diff_areas_ == 0)
        return 0;
    else
        id_array_ptr = addr<IdArray>(antenna_diff_areas_);
    if (id_array_ptr)
        return id_array_ptr->getSize();
    else
        return 0;
}

void Term::addPort(ObjectId p) {
    IdArray *id_array_ptr = nullptr;
    if (ports_ == 0) {
        ports_ = __createObjectIdArray(32);
    }
    if (ports_ == 0) return;
    id_array_ptr = Object::addr<IdArray>(ports_);
    if (id_array_ptr) id_array_ptr->pushBack(p);
}

Port *Term::getPort(int index) const {
    IdArray *id_array_ptr = nullptr;
    if (ports_ == 0)
        return nullptr;
    else
        id_array_ptr = addr<IdArray>(ports_);
    if (id_array_ptr) {
        Port *obj_data = addr<Port>((*id_array_ptr)[index]);
        if (obj_data) {
            return obj_data;
        }
    }
    return nullptr;
}

int Term::getPortNum() const {
    IdArray *id_array_ptr = nullptr;
    if (ports_ == 0)
        return 0;
    else
        id_array_ptr = addr<IdArray>(ports_);
    if (id_array_ptr)
        return id_array_ptr->getSize();
    else
        return 0;
}

void Term::setCellId(ObjectId cell_id) {
    cell_id_ = cell_id;
}

ObjectId Term::getCellId() {
    return cell_id_;
}

Cell* Term::getCell() const {
    if (cell_id_ != 0) {
        return addr<Cell>(cell_id_);
    }
    return nullptr;
}

AntennaModelTerm::AntennaModelTerm() {
    antenna_gate_areas_ = 0;
    antenna_max_area_cars_ = 0;
    antenna_max_side_area_cars_ = 0;
    antenna_max_cut_cars_ = 0;
}

Port::Port() {
    is_real_ = true;
    has_placement_ = false;
    location_.set(0, 0);
    class_index_ = -1;
    status_ = PlaceStatus::kUnknown;
    orient_ = Orient::kUnknown;
}

Port::~Port() {}

void Port::setTermId(ObjectId term_id) {
    term_id_ = term_id;
}

Term *Port::getTerm() const {
    return addr<Term>(term_id_);
}

bool Port::getHasPlacement() const { return has_placement_; }

void Port::setHasPlacement(bool flag) { has_placement_ = flag; }

PlaceStatus Port::getStatus() const { return status_; }

void Port::setStatus(PlaceStatus s) { status_ = s; }

Point Port::getLocation() const { return location_; }

void Port::setLocation(Point &p) { location_ = p; }

Orient Port::getOrient() const { return orient_; }

void Port::setOrient(Orient o) { orient_ = o; }

void Port::setClass(const char *v) {
    Cell *owner_cell = getTerm()->getCell();
    if (!owner_cell) {
        message->issueMsg("DB", 24, kError, v);
        return;
    }
    class_index_ = owner_cell->getOrCreateSymbol(v);
    // owner_cell->addSymbolReference(class_index_, this->getId());
}

std::string &Port::getClass() const {
    Cell *owner_cell = getTerm()->getCell();
    return owner_cell->getSymbolTable()->getSymbolByIndex(class_index_);
}

void Port::addLayerGeometry(ObjectId v) {
    IdArray *id_array_ptr = nullptr;
    if (layer_geometries_ == 0) {
        layer_geometries_ = __createObjectIdArray(32);
    }
    if (layer_geometries_ == 0) return;
    id_array_ptr = Object::addr<IdArray>(layer_geometries_);
    if (id_array_ptr) id_array_ptr->pushBack(v);
}

LayerGeometry *Port::getLayerGeometry(int index) const {
    IdArray *id_array_ptr = nullptr;
    if (layer_geometries_ == 0)
        return nullptr;
    else
        id_array_ptr = addr<IdArray>(layer_geometries_);
    if (id_array_ptr) {
        LayerGeometry *obj_data =
            addr<LayerGeometry>((*id_array_ptr)[index]);
        if (obj_data) {
            return obj_data;
        }
    }
    return nullptr;
}

int Port::getLayerGeometryNum() const {
    IdArray *id_array_ptr = nullptr;
    if (layer_geometries_ == 0)
        return 0;
    else
        id_array_ptr = addr<IdArray>(layer_geometries_);
    if (id_array_ptr)
        return id_array_ptr->getSize();
    else
        return 0;
}

void Term::printLEF(IOManager &io_manager) const {
    Tech *lib = getTechLib();
    io_manager.write("   PIN %s\n", getName().c_str());
    if (taper_rule_index_ != 0)
        io_manager.write("      TAPERRULE %s ;\n", getTaperRule().c_str());

    std::string str_value = "temp";
    if (hasDirection()) {
        str_value.assign(getDirectionStr());
        toUpper(str_value);
        io_manager.write("      DIRECTION %s ;\n", str_value.c_str());
    }
    if (hasUse()) {
        str_value.assign(getTypeStr());
        toUpper(str_value); 
        io_manager.write("      USE %s ;\n", str_value.c_str());
    }
    if (getNetExpr().size() != 0) {
        io_manager.write("      NETEXPR \"%s\" ;\n", getNetExpr().c_str());
    }
    if (supply_sensitivity_index_ != 0)
        io_manager.write("      SUPPLYSENSITIVITY %s ;\n",
            getSupplySensitivity().c_str());
    if (ground_sensitivity_index_ != 0)
        io_manager.write("      GROUNDSENSITIVITY %s ;\n",
            getGroundSensitivity().c_str());
    if (shape_index_ != 0)
        io_manager.write("      SHAPE %s ;\n", getShape().c_str());
    if (mustjoin_index_ != 0)
        io_manager.write("      MUSTJOIN %s ;\n", getMustjoin().c_str());

    if (getPortNum() > 0) {
        for (int i = 0; i < getPortNum(); i++) {
            Port *p = getPort(i);
            io_manager.write("      PORT\n");
            if (p->getClass().size() > 0) {
                io_manager.write("         CLASS %s ;\n",
                    p->getClass().c_str());
            }
            if (p->getLayerGeometryNum() > 0) {
                for (int j = 0; j < p->getLayerGeometryNum(); j++) {
                    LayerGeometry *lg = p->getLayerGeometry(j);
                    lg->printLEF(io_manager, true);
                }
            }
            io_manager.write("      END\n");
        }
    }

    if (getAntennaPartialMetalAreaNum() > 0) {
        for (int i = 0; i < getAntennaPartialMetalAreaNum(); i++) {
            AntennaArea *a = getAntennaPartialMetalArea(i);
            io_manager.write("      ANTENNAPARTIALMETALAREA %g",
                lib->areaDBUToMicrons(a->getArea()));
            Layer * layer = a->getLayer();
            if (layer != nullptr)
                io_manager.write(" LAYER %s", layer->getName());
            io_manager.write(" ;\n");
        }
    }
    if (getAntennaPartialMetalSideAreaNum() > 0) {
        for (int i = 0; i < getAntennaPartialMetalSideAreaNum(); i++) {
            AntennaArea *a = getAntennaPartialMetalSideArea(i);
            io_manager.write("      ANTENNAPARTIALMETALSIDEAREA %g",
                lib->areaDBUToMicrons(a->getArea()));
            Layer * layer = a->getLayer();
            if (layer != nullptr)
                io_manager.write(" LAYER %s", layer->getName());
            io_manager.write(" ;\n");
        }
    }
    if (getAntennaPartialCutAreaNum() > 0) {
        for (int i = 0; i < getAntennaPartialCutAreaNum(); i++) {
            AntennaArea *a = getAntennaPartialCutArea(i);
            io_manager.write("      ANTENNAPARTIALCUTAREA %g",
                lib->areaDBUToMicrons(a->getArea()));
            Layer * layer = a->getLayer();
            if (layer != nullptr)
                io_manager.write(" LAYER %s", layer->getName());
            io_manager.write(" ;\n");
        }
    }
    if (getAntennaDiffAreaNum() > 0) {
        for (int i = 0; i < getAntennaDiffAreaNum(); i++) {
            AntennaArea *a = getAntennaDiffArea(i);
            io_manager.write("      ANTENNADIFFAREA %g",
                lib->areaDBUToMicrons(a->getArea()));
            Layer * layer = a->getLayer();
            if (layer != nullptr)
                io_manager.write(" LAYER %s", layer->getName());
            io_manager.write(" ;\n");
        }
    }

    for (int i = 0; i < 6; i++) {
        const AntennaModelTerm *am = GetAntennaModelTerm(i);
        if (am->getValid()) {
            io_manager.write("      ANTENNAMODEL OXIDE%d ;\n", i);
            am->printLEF(io_manager);
        }
    }

    io_manager.write("   END " + getName() + "\n");
}

void AntennaModelTerm::printLEF(IOManager &io_manager) const {
    Tech *lib = getTechLib();
    if (getAntennaGateAreaNum() > 0) {
        for (int i = 0; i < getAntennaGateAreaNum(); i++) {
            AntennaArea *a = getAntennaGateArea(i);
            io_manager.write("      ANTENNAGATEAREA %g",
                lib->areaDBUToMicrons(a->getArea()));
            Layer * layer = a->getLayer();
            if (layer != nullptr)
                io_manager.write(" LAYER %s", layer->getName());
            io_manager.write(" ;\n");
        }
    }

    if (getAntennaMaxAreaCarNum() > 0) {
        for (int i = 0; i < getAntennaMaxAreaCarNum(); i++) {
            AntennaArea *a = getAntennaMaxAreaCar(i);
            io_manager.write("      ANTENNAMAXAREACAR %g",
                lib->areaDBUToMicrons(a->getArea()));
            Layer * layer = a->getLayer();
            if (layer != nullptr)
                io_manager.write(" LAYER %s", layer->getName());
            io_manager.write(" ;\n");
        }
    }

    if (getAntennaMaxSideAreaCarNum() > 0) {
        for (int i = 0; i < getAntennaMaxSideAreaCarNum(); i++) {
            AntennaArea *a = getAntennaMaxSideAreaCar(i);
            io_manager.write("      ANTENNAMAXSIDEAREACAR %g",
                lib->areaDBUToMicrons(a->getArea()));
            Layer * layer = a->getLayer();
            if (layer != nullptr)
                io_manager.write(" LAYER %s", layer->getName());
            io_manager.write(" ;\n");
        }
    }

    if (getAntennaMaxCutCarNum() > 0) {
        for (int i = 0; i < getAntennaMaxCutCarNum(); i++) {
            AntennaArea *a = getAntennaMaxCutCar(i);
            io_manager.write("      ANTENNAMAXCUTCAR %g",
                lib->areaDBUToMicrons(a->getArea()));
            Layer * layer = a->getLayer();
            if (layer != nullptr)
                io_manager.write(" LAYER %s", layer->getName());
            io_manager.write(" ;\n");
        }
    }
}

std::vector<PinAccess*>* Term::getAccesses(const Orient ori, const unsigned char set) {

    unsigned char index = static_cast<unsigned char>(ori);

    if (index >= kMaxOrientationNum) {
        return nullptr;
    }

    if (set >= kMaxSetNum) {
        return nullptr;
    }

    return &(accesses_[index][set]);
}

} // namespace db
}  // namespace open_edi
