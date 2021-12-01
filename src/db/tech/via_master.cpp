/* @file  via_master.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#include "db/tech/via_master.h"

#include "db/core/db.h"
#include "db/util/array.h"

namespace open_edi {
namespace db {
using IdArray = ArrayObject<ObjectId>;

/**
 * @brief Construct a new Via Layer:: Via Layer object
 *
 */
ViaLayer::ViaLayer() { name_index_ = 0; }

/**
 * @brief Get the Name Index object
 *
 * @return SymbolIndex
 */
SymbolIndex ViaLayer::getNameIndex() const { return name_index_; }

/**
 * @brief Set the Name object
 *
 * @param name
 */
bool ViaLayer::setName(std::string const& name) {
    SymbolIndex index = getTechLib()->getOrCreateSymbol(name.c_str());
    if (index == kInvalidSymbolIndex) return false;

    name_index_ = index;
    getTechLib()->addSymbolReference(name_index_, this->getId());
    return true;
}
/**
 * @brief Get the Name object
 *
 * @return const char*
 */
std::string const& ViaLayer::getName() {
    return getTechLib()->getSymbolByIndex(name_index_);
}

/**
 * @brief add mask
 *
 * @param num
 */
void ViaLayer::addMask(int num) { masks_.push_back(num); }

/**
 * @brief Get the Mask Num object
 *
 * @param num
 * @return int
 */
int ViaLayer::getMaskNum(int num) { return masks_[num]; }

/**
 * @brief add rect
 *
 * @param rect
 */
void ViaLayer::addRect(Box rect) { rects_.push_back(rect); }

/**
 * @brief Get the Rect object
 *
 * @param num
 * @return Box*
 */
Box ViaLayer::getRect(int num) { return rects_[num]; }

/**
 * @brief Get the Rects object
 *
 * @return std::vector<Box*>
 */
std::vector<Box> ViaLayer::getRects() { return rects_; }

/**
 * @brief print out
 *
 * @param is_def
 */
void ViaLayer::print(int is_def) {
    if (!is_def) {
        message->info("   LAYER %s ;\n", getName().c_str());
        std::vector<Box> rects = getRects();
        for (int i = 0; i < rects.size(); ++i) {
            Box box = getRect(i);
            message->info("      RECT  %d %d %d %d ;\n", box.getLLX(),
                          box.getLLY(), box.getURX(), box.getURY());
        }
    } else {
        message->info("   + RECT %s ", getName().c_str());
        std::vector<Box> rects = getRects();
        for (int i = 0; i < rects.size(); ++i) {
            Box box = getRect(i);
            if (getMaskNum(i)) {
                message->info(" + MASK  %d ", getMaskNum(i));
            }
            message->info(" ( %d %d ) ( %d %d ) \n", box.getLLX(), box.getLLY(),
                          box.getURX(), box.getURY());
        }
    }
}

void ViaLayer::printLEF(IOManager &io_manager, uint32_t num_spaces) {
    Tech* lib = getTechLib();
    std::string space_str = getSpaceStr(num_spaces);

    io_manager.write("%s   LAYER %s ;\n",
        space_str.c_str(), getName().c_str());
    std::vector<Box> rects = getRects();
    for (int i = 0; i < rects.size(); ++i) {
        Box box = getRect(i);
        io_manager.write("%s      RECT  %g %g %g %g ;\n",
            space_str.c_str(), lib->dbuToMicrons(box.getLLX()),
            lib->dbuToMicrons(box.getLLY()),
            lib->dbuToMicrons(box.getURX()),
            lib->dbuToMicrons(box.getURY()));
    }
}

void ViaLayer::printDEF(IOManager& io_manager) {
    io_manager.write("\n   + RECT %s ", getName().c_str());

    std::vector<Box> rects = getRects();
    for (int i = 0; i < rects.size(); ++i) {
        Box box = getRect(i);
        if (getMaskNum(i)) {
            io_manager.write(" + MASK  %d ", getMaskNum(i));
        }
        io_manager.write(" ( %d %d ) ( %d %d )",
            box.getLLX(), box.getLLY(),
            box.getURX(), box.getURY());
    }
}

/**
 * @brief Construct a new Via Master:: Via Master object
 *
 */
ViaMaster::ViaMaster() {
    name_index_ = 0;
    pattern_index_ = 0;
    is_default_ = 0;
    has_via_rule_ = 0;
    is_array_ = 0;
    is_from_def_ = 0;
    has_origin_ = 0;
    has_offset_ = 0;
    has_cut_pattern_ = 0;
    has_resistance_ = 0;
    x_size_ = 0;
    y_size_ = 0;
    via_rule_ = 0;
    lower_layer_ = 0;
    cut_layer_ = 0;
    upper_layer_ = 0;
    x_cut_spacing_ = 0;
    y_cut_spacing_ = 0;
    x_lower_enclosure_ = 0;
    y_lower_enclosure_ = 0;
    x_upper_enclosure_ = 0;
    y_upper_enclosure_ = 0;
    rows_ = 1;
    columns_ = 1;
    x_offset_ = 0;
    y_offset_ = 0;
    x_lower_offset_ = 0;
    y_lower_offset_ = 0;
    x_upper_offset_ = 0;
    y_upper_offset_ = 0;
    cut_patten_index_ = 0;
    resistance_ = 0;
    is_from_ndr_ = 0;
}
/**
 * @brief Get the Name Index object
 *
 * @return SymbolIndex
 */
SymbolIndex ViaMaster::getNameIndex() const { return name_index_; }

/**
 * @brief Get the Name object
 *
 * @return std::string const&
 */
std::string const& ViaMaster::getName() {
    return getTechLib()->getSymbolByIndex(name_index_);
}

/**
 * @brief Set the Name object
 *
 * @param name
 */
bool ViaMaster::setName(std::string const& name) {
    SymbolIndex index = getTechLib()->getOrCreateSymbol(name.c_str());
    if (index == kInvalidSymbolIndex) return false;

    name_index_ = index;
    getTechLib()->addSymbolReference(name_index_, this->getId());
    return true;
}

/**
 * @brief Get the Name object
 *
 * @return std::string const&
 */
std::string const& ViaMaster::getPattern() {
    return getTechLib()->getSymbolByIndex(pattern_index_);
}

/**
 * @brief Set the Name object
 *
 * @param name
 */
bool ViaMaster::setPattern(std::string const& pattern) {
    SymbolIndex index = getTechLib()->getOrCreateSymbol(pattern.c_str());
    if (index == kInvalidSymbolIndex) return false;

    pattern_index_ = index;
    getTechLib()->addSymbolReference(pattern_index_, this->getId());
    return true;
}
/**
 * @brief Get the Cut Size X object
 *
 * @return int
 */
int ViaMaster::getCutSizeX() const { return x_size_; }

/**
 * @brief Get the Cut Size Y object
 *
 * @return int
 */
int ViaMaster::getCutSizeY() const { return y_size_; }

/**
 * @brief Get the Cut Spacing X object
 *
 * @return int
 */
int ViaMaster::getCutSpacingX() const { return x_cut_spacing_; }

/**
 * @brief Get the Cut Spacing Y object
 *
 * @return int
 */
int ViaMaster::getCutSpacingY() const { return y_cut_spacing_; }

/**
 * @brief Get the Lower Enc X object
 *
 * @return int
 */
int ViaMaster::getLowerEncX() const { return x_lower_enclosure_; }

/**
 * @brief Get the Lower Enc Y object
 *
 * @return int
 */
int ViaMaster::getLowerEncY() const { return y_lower_enclosure_; }

/**
 * @brief Get the Upper Enc X object
 *
 * @return int
 */
int ViaMaster::getUpperEncX() const { return x_upper_enclosure_; }

/**
 * @brief Get the Upper Enc Y object
 *
 * @return int
 */
int ViaMaster::getUpperEncY() const { return y_upper_enclosure_; }

/**
 * @brief Get the Row object
 *
 * @return int
 */
int ViaMaster::getRow() const { return rows_; }

/**
 * @brief Get the Col object
 *
 * @return int
 */
int ViaMaster::getCol() const { return columns_; }

/**
 * @brief Get the Offset X object
 *
 * @return int
 */
int ViaMaster::getOffsetX() const { return x_offset_; }

/**
 * @brief Get the Offset Y object
 *
 * @return int
 */
int ViaMaster::getOffsetY() const { return y_offset_; }

/**
 * @brief Get the Lower Offset X object
 *
 * @return int
 */
int ViaMaster::getLowerOffsetX() const { return x_lower_offset_; }

/**
 * @brief Get the Lower Offset Y object
 *
 * @return int
 */
int ViaMaster::getLowerOffsetY() const { return y_lower_offset_; }

/**
 * @brief Get the Upper Offset X object
 *
 * @return int
 */
int ViaMaster::getUpperOffsetX() const { return x_upper_offset_; }

/**
 * @brief Get the Upper Offset Y object
 *
 * @return int
 */
int ViaMaster::getUpperOffsetY() const { return y_upper_offset_; }

/**
 * @brief judge is array
 *
 * @return true
 * @return false
 */
bool ViaMaster::isArray() { return is_array_; }

/**
 * @brief judge is from def
 *
 * @return true
 * @return false
 */
bool ViaMaster::isFromDEF() { return is_from_def_; }
/**
 * @brief judge is from ndr
 *
 * @return true
 * @return false
 */
bool ViaMaster::isFromNDR() { return is_from_ndr_; }

/**
 * @brief has origin
 *
 * @return true
 * @return false
 */
bool ViaMaster::hasOrigin() { return has_origin_; }

/**
 * @brief has offset
 *
 * @return true
 * @return false
 */
bool ViaMaster::hasOffset() { return has_offset_; }

/**
 * @brief has cut patten
 *
 * @return true
 * @return false
 */
bool ViaMaster::hasCutPatten() { return has_cut_pattern_; }

/**
 * @brief is generated
 *
 * @return true
 * @return false
 */
bool ViaMaster::hasGenerated() { return has_generated_; }

/**
 * @brief has resistance
 *
 * @return true
 * @return false
 */
bool ViaMaster::hasResistance() { return has_resistance_; }

/**
 * @brief Get the Via Layer Vector object
 *
 * @return ObjectId
 */
ObjectId ViaMaster::getViaLayerVector() { return via_layers_; }

/**
 * @brief Set the Cut Size X object
 *
 * @param x
 */
void ViaMaster::setCutSizeX(int x) { x_size_ = x; }

/**
 * @brief Set the Cut Size Y object
 *
 * @param y
 */
void ViaMaster::setCutSizeY(int y) { y_size_ = y; }

/**
 * @brief Set the Cut Spacing X object
 *
 * @param x
 */
void ViaMaster::setCutSpacingX(int x) { x_cut_spacing_ = x; }

/**
 * @brief Set the Cut Spacing Y object
 *
 * @param y
 */
void ViaMaster::setCutSpacingY(int y) { y_cut_spacing_ = y; }

/**
 * @brief Set the Lower Enc X object
 *
 * @param x
 */
void ViaMaster::setLowerEncX(int x) { x_lower_enclosure_ = x; }

/**
 * @brief Set the Lower Enc Y object
 *
 * @param y
 */
void ViaMaster::setLowerEncY(int y) { y_lower_enclosure_ = y; }

/**
 * @brief Set the Upper Enc X object
 *
 * @param x
 */
void ViaMaster::setUpperEncX(int x) { x_upper_enclosure_ = x; }

/**
 * @brief Set the Upper Enc Y object
 *
 * @param y
 */
void ViaMaster::setUpperEncY(int y) { y_upper_enclosure_ = y; }

/**
 * @brief Set the Row object
 *
 * @param rows
 */
void ViaMaster::setRow(int rows) { rows_ = rows; }

/**
 * @brief Set the Col object
 *
 * @param col
 */
void ViaMaster::setCol(int col) { columns_ = col; }

/**
 * @brief Set the Offset X object
 *
 * @param x
 */
void ViaMaster::setOffsetX(int x) { x_offset_ = x; }

/**
 * @brief Set the Offset Y object
 *
 * @param y
 */
void ViaMaster::setOffsetY(int y) { y_offset_ = y; }

/**
 * @brief Set the Lower Offset X object
 *
 * @param x
 */
void ViaMaster::setLowerOffsetX(int x) { x_lower_offset_ = x; }

/**
 * @brief Set the Lower Offset Y object
 *
 * @param y
 */
void ViaMaster::setLowerOffsetY(int y) { y_lower_offset_ = y; }

/**
 * @brief Set the Upper Offset X object
 *
 * @param x
 */
void ViaMaster::setUpperOffsetX(int x) { x_upper_offset_ = x; }

/**
 * @brief Set the Upper Offset Y object
 *
 * @param y
 */
void ViaMaster::setUpperOffsetY(int y) { y_upper_offset_ = y; }

/**
 * @brief Set the Is Array object
 *
 * @param is_array
 */
void ViaMaster::setIsArray(Bits is_array) { is_array_ = is_array; }

/**
 * @brief Set the Is From D E F object
 *
 * @param is_from_def
 */
void ViaMaster::setIsFromDEF(Bits is_from_def) { is_from_def_ = is_from_def; }

/**
 * @brief Set the Is From ndr object
 *
 * @param is_from_def
 */
void ViaMaster::setIsFromNDR(Bits is_from_ndr) { is_from_ndr_ = is_from_ndr; }

/**
 * @brief Set the Has Origin object
 *
 * @param has_origin
 */
void ViaMaster::setHasOrigin(int has_origin) {
    if (has_origin > 0) has_origin_ = 1;
}

/**
 * @brief Set the Has Offset object
 *
 * @param has_offset
 */
void ViaMaster::setHasOffset(int has_offset) {
    if (has_offset > 0) has_offset_ = 1;
}

/**
 * @brief set has Generated
 *
 * @param has_generated
 */
void ViaMaster::sethasGenerated(Bits has_generated) {
    has_generated_ = has_generated;
}

/**
 * @brief Set the Has Resistance object
 *
 * @param has_resistance
 */
void ViaMaster::setHasResistance(Bits has_resistance) {
    has_resistance_ = has_resistance;
}

/**
 * @brief Set the Has Cut Patten object
 *
 * @param has_cut_pattern
 */
void ViaMaster::setHasCutPatten(Bits has_cut_pattern) {
    has_cut_pattern_ = has_cut_pattern;
}

/**
 * @brief Get the Name object
 *
 * @return std::string const&
 */
std::string const& ViaMaster::getCutPatten() {
    return getTechLib()->getSymbolByIndex(cut_patten_index_);
}

/**
 * @brief Set the Name object
 *
 * @param name
 */
bool ViaMaster::setCutPatten(std::string const& cut_pattern) {
    SymbolIndex index = getTechLib()->getOrCreateSymbol(cut_pattern.c_str());
    if (index == kInvalidSymbolIndex) return false;

    cut_patten_index_ = index;
    getTechLib()->addSymbolReference(cut_patten_index_, this->getId());
    return true;
}

/**
 * @brief Get the Via Rule object
 *
 * @return const char*
 */
ViaRule* ViaMaster::getViaRule() const {
    ViaRule* via_rule = nullptr;
    via_rule = addr<ViaRule>(via_rule_);
    return via_rule;
}

/**
 * @brief Set the Via Rule object
 *
 * @param via_rule
 */
void ViaMaster::setViaRule(ViaRule* via_rule) {
    if (via_rule) via_rule_ = via_rule->getId();
}

Layer* ViaMaster::getLowerLayer() {
    Layer* layer = getTechLib()->getLayer(lower_layer_);
    return layer;
}
Layer* ViaMaster::getCutLayer() {
    Layer* layer = getTechLib()->getLayer(cut_layer_);
    return layer;
}
Layer* ViaMaster::getUpperLayer() {
    Layer* layer = getTechLib()->getLayer(upper_layer_);
    return layer;
}

/**
 * @brief Get the Lower Layer Index object
 *
 * @return ObjectIndex
 */
int ViaMaster::getLowerLayerIndex() {
    int lower_layer_num = 0;
    if (lower_layer_ != 0) {
        lower_layer_num = lower_layer_;
    } else {
        ArrayObject<ObjectId>* via_layers = getViaLayerArray();
        Tech* lib = getTechLib();

        for (auto iter = via_layers->begin();
             iter != via_layers->end();  // find the lower layer
             iter++) {
            ViaLayer* via_layer = addr<ViaLayer>(*iter);
            if (via_layer) {
                if (lib) {
                    int layer_num = lib->getLayerLEFIndexByName(
                        via_layer->getName().c_str());
                    if (lower_layer_num == 0 || lower_layer_num > layer_num) {
                        lower_layer_num = layer_num;
                    }
                }
            }
        }
    }

    return lower_layer_num;
}

/**
 * @brief Set the Lower Layer Index object
 *
 * @param index
 */
void ViaMaster::setLowerLayerIndex(int index) { lower_layer_ = index; }

/**
 * @brief Get the Cut Layer Index object
 *
 * @return ObjectIndex
 */
int ViaMaster::getCutLayerIndex() {
    int cut_layer_num = 0;
    if (cut_layer_ != 0) {
        cut_layer_num = cut_layer_;
    } else {
        ArrayObject<ObjectId>* via_layers = getViaLayerArray();

        Tech* lib             = getTechLib();
        int   upper_layer_num = getUpperLayerIndex();
        int   lower_layer_num = getLowerLayerIndex();
        for (auto iter = via_layers->begin();
             iter != via_layers->end(); // find the cut layer
             iter++) {
            ViaLayer* via_layer = addr<ViaLayer>(*iter);
            if (via_layer) {
                if (lib) {
                    int layer_num = lib->getLayerLEFIndexByName(
                      via_layer->getName().c_str());
                    if ((layer_num > lower_layer_num) && (layer_num < upper_layer_num)) {
                        cut_layer_num = layer_num;
                        return cut_layer_num;
                    }
                }
            }
        }
    }

    return cut_layer_num;
}

/**
 * @brief Set the Cut Layer Index object
 *
 * @param index
 */
void ViaMaster::setCutLayerIndex(int index) { cut_layer_ = index; }

/**
 * @brief Get the Uper Layer Index object
 *
 * @return ObjectIndex
 */
int ViaMaster::getUpperLayerIndex() {
    int upper_layer_num = 0;
    if (upper_layer_ != 0) {
        upper_layer_num = upper_layer_;
    } else {
        ArrayObject<ObjectId>* via_layers = getViaLayerArray();
        Tech* lib = getTechLib();

        for (auto iter = via_layers->begin();
             iter != via_layers->end();  // find the upper layer
             iter++) {
            ViaLayer* via_layer = addr<ViaLayer>(*iter);
            if (via_layer) {
                if (lib) {
                    int layer_num = lib->getLayerLEFIndexByName(
                        via_layer->getName().c_str());
                    if (upper_layer_num == 0 || upper_layer_num < layer_num) {
                        upper_layer_num = layer_num;
                    }
                }
            }
        }
    }

    return upper_layer_num;
}

/**
 * @brief Set the Uper Layer Index object
 *
 * @param index
 */
void ViaMaster::setUpperLayerIndex(int index) { upper_layer_ = index; }

/**
 * @brief get resistance
 *
 * @return double
 */
double ViaMaster::getResistance() const { return resistance_; }

Box ViaMaster::getBoundingBox() {
    Box&& cut_rect = getCutBox();
    Box&& bot_rect = getBotBox();
    Box&& top_rect = getTopBox();

    std::vector<Box> rects{cut_rect, bot_rect, top_rect};

    return __calculateBBox(rects);
}

Box ViaMaster::getBotBox() {
    // generated vias
    if (via_rule_) {
        int width  = x_lower_enclosure_ * 2 + x_size_ * columns_ + x_cut_spacing_ * (columns_ - 1);
        int height = y_lower_enclosure_ * 2 + y_size_ * rows_ + y_cut_spacing_ * (rows_ - 1);
        int llx    = -width / 2;
        int lly    = -height / 2;
        int urx    = width / 2;
        int ury    = height / 2;
        //add origin offset
        llx += x_offset_;
        lly += y_offset_;
        urx += x_offset_;
        ury += y_offset_;
        //add layer offset
        llx += x_lower_offset_;
        lly += y_lower_offset_;
        urx += x_lower_offset_;
        ury += y_lower_offset_;
        return Box(llx, lly, urx, ury);
    } else { // fixed vias
        ediAssert(via_layers_ != 0);

        IdArray* via_layer_vector = addr<IdArray>(via_layers_);
        Tech*    lib              = getTechLib();
        for (IdArray::iterator iter = via_layer_vector->begin();
             iter != via_layer_vector->end(); ++iter) {
            ObjectId  id        = (*iter);
            ViaLayer* via_layer = addr<ViaLayer>(id);
            if (via_layer) {
                if (lib) {
                    int layer_num = lib->getLayerLEFIndexByName(
                      via_layer->getName().c_str());
                    int lower_layer_num = getLowerLayerIndex();
                    if (layer_num == lower_layer_num) {
                        std::vector<Box> rects = via_layer->getRects();
                        return __calculateBBox(rects);
                    }
                }
            }
        }
        return Box(0, 0, 0, 0);
    }
}

Box ViaMaster::getTopBox()  {

    // generated vias
    if (via_rule_) {
        int width  = x_upper_enclosure_ * 2 + x_size_ * columns_ + x_cut_spacing_ * (columns_ - 1);
        int height = y_upper_enclosure_ * 2 + y_size_ * rows_ + y_cut_spacing_ * (rows_ - 1);
        int llx    = -width / 2;
        int lly    = -height / 2;
        int urx    = width / 2;
        int ury    = height / 2;
        //add origin offset
        llx += x_offset_;
        lly += y_offset_;
        urx += x_offset_;
        ury += y_offset_;
        //add layer offset
        llx += x_upper_offset_;
        lly += y_upper_offset_;
        urx += x_upper_offset_;
        ury += y_upper_offset_;
        return Box(llx, lly, urx, ury);
    } else { // fixed vias
        ediAssert(via_layers_ != 0);

        IdArray* via_layer_vector = addr<IdArray>(via_layers_);
        Tech*    lib              = getTechLib();
        for (IdArray::iterator iter = via_layer_vector->begin();
             iter != via_layer_vector->end(); ++iter) {
            ObjectId  id        = (*iter);
            ViaLayer* via_layer = addr<ViaLayer>(id);
            if (via_layer) {
                if (lib) {
                    int layer_num = lib->getLayerLEFIndexByName(
                      via_layer->getName().c_str());
                    int upper_layer_num = getUpperLayerIndex();
                    if (layer_num == upper_layer_num) {
                        std::vector<Box> rects = via_layer->getRects();
                        return __calculateBBox(rects);
                    }
                }
            }
        }
        return Box(0, 0, 0, 0);
    }
}

Box ViaMaster::getCutBox() {

    // generated vias
    if (via_rule_) {
        int width  = x_size_ * columns_ + x_cut_spacing_ * (columns_ - 1);
        int height = y_size_ * rows_ + y_cut_spacing_ * (rows_ - 1);
        int llx    = -width / 2;
        int lly    = -height / 2;
        int urx    = width / 2;
        int ury    = height / 2;
        //add origin offset
        llx += x_offset_;
        lly += y_offset_;
        urx += x_offset_;
        ury += y_offset_;
        return Box(llx, lly, urx, ury);
    } else { // fixed vias
        ediAssert(via_layers_ != 0);

        IdArray* via_layer_vector = addr<IdArray>(via_layers_);
        Tech*    lib              = getTechLib();
        for (IdArray::iterator iter = via_layer_vector->begin();
             iter != via_layer_vector->end(); ++iter) {
            ObjectId  id        = (*iter);
            ViaLayer* via_layer = addr<ViaLayer>(id);
            if (via_layer) {
                if (lib) {
                    int layer_num = lib->getLayerLEFIndexByName(
                      via_layer->getName().c_str());
                    int cut_layer_num = getCutLayerIndex();
                    if (layer_num == cut_layer_num) {
                        std::vector<Box> rects = via_layer->getRects();
                        return __calculateBBox(rects);
                    }
                }
            }
        }
        return Box(0, 0, 0, 0);
    }
}

/**
 * @brief set resistance
 *
 * @param resistance
 */
void ViaMaster::setResistance(double resistance) { resistance_ = resistance; }

/**
 * @brief Get the viaMaster default status
 *
 * @return true
 * @return false
 */
bool ViaMaster::isDefault() const { return is_default_; }

/**
 * @brief creatViaLayer
 *
 * @param name
 * @return ViaLayer*
 */
ViaLayer* ViaMaster::creatViaLayer(std::string& name) {
    ViaLayer* via_layer = Object::createObject<ViaLayer>(kObjectTypeViaMaster,
                                                         getTechLib()->getId());
    via_layer->setName(name);
    return via_layer;
}

int ViaMaster::addViaLayer(ViaLayer* via_layer) {
    if (via_layers_ == 0) {
        via_layers_ = __createObjectIdArray(8);
    }
    IdArray* via_layer_vector = addr<IdArray>(via_layers_);
    via_layer_vector->pushBack(via_layer->getId());
    return 0;
}

ArrayObject<ObjectId>* ViaMaster::getViaLayerArray() {
    if (via_layers_ == 0) return nullptr;

    ArrayObject<ObjectId>* layer_array =
        Object::addr<ArrayObject<ObjectId>>(via_layers_);

    return layer_array;
}

void ViaMaster::setArrayIndex(uint16_t array_index) {
    array_index_ = array_index;
}

uint16_t ViaMaster::getArrayIndex() const {
    return array_index_;
}

/**
 * @brief Set the viaMaster default status
 *
 * @param is_default
 */
void ViaMaster::setDefault(bool is_default) { is_default_ = is_default; }

void ViaMaster::printDEF(IOManager& io_manager) {
    io_manager.write(" - %s ", getName().c_str());

    if (isDefault()) {
        io_manager.write(" DEFAULT");
    }

    if (pattern_index_)
        io_manager.write("\n + PATTERNNAME %s", getPattern().c_str());
    ViaRule* via_rule = getViaRule();
    if (via_rule) {
        io_manager.write("\n    + VIARULE %s"
            "\n    + CUTSIZE %d %d"
            "\n    + LAYERS %s %s %s"
            "\n    + CUTSPACING %d %d"
            "\n    + ENCLOSURE %d %d %d %d",
            via_rule->getName().c_str(),
            getCutSizeX(), getCutSizeY(),
            getLowerLayer()->getName(), getCutLayer()->getName(), getUpperLayer()->getName(),
            getCutSpacingX(), getCutSpacingY(),
            getLowerEncX(), getLowerEncY(), getUpperEncX(), getUpperEncY());

        if (isArray()) {
            io_manager.write("\n    + ROWCOL %d %d", getRow(), getCol());
        }

        if (cut_patten_index_) {
            io_manager.write("\n    + PATTERN " + getCutPatten());
        }

        if (hasOrigin()) {
            io_manager.write("\n    + ORIGIN %d %d",
                getOffsetX(), getOffsetY());
        }

        if (hasOffset()) {
            io_manager.write("\n    + OFFSET %d %d %d %d",
                getLowerOffsetX(), getLowerOffsetY(),
                getUpperOffsetX(), getUpperOffsetY());
        }
    }

    if (getResistance())
        io_manager.write("\n  RESISTANCE %f", getResistance());
    if (via_layers_) {
        IdArray* via_layer_vector = addr<IdArray>(via_layers_);
        int i = 0;
        int size = via_layer_vector->getSize();
        for (IdArray::iterator iter = via_layer_vector->begin();
             iter != via_layer_vector->end(); ++iter) {
            ObjectId id = (*iter);
            ViaLayer* via_layer = addr<ViaLayer>(id);
            via_layer->printDEF(io_manager);
            ++i;
            if (i == size) {
                io_manager.write(" ;");
            }
        }
    } else {
        io_manager.write(" ;");
    }
    io_manager.write("\n\n");
}

Box ViaMaster::__calculateBBox(std::vector<Box>& rects) 
{
    ediAssert(rects.size() > 0);

    int offset_llx = rects[0].getLLX();
    int offset_urx = rects[0].getURX();
    int offset_lly = rects[0].getLLY();
    int offset_ury = rects[0].getURY();

    for (unsigned int i = 1; i < rects.size(); ++i) {
        offset_llx = std::min(rects[i].getLLX(), offset_llx);
        offset_lly = std::min(rects[i].getLLY(), offset_lly);
        offset_urx = std::max(rects[i].getURX(), offset_urx);
        offset_ury = std::max(rects[i].getURY(), offset_ury);
    }

    return Box(offset_llx, offset_lly, offset_urx, offset_ury);
}

void ViaMaster::printLEF(IOManager &io_manager, uint32_t num_spaces) {
    std::string space_str = getSpaceStr(num_spaces);
    Cell* top_cell = getTopCell();
    Tech* lib = top_cell->getTechLib();

    io_manager.write("%sVIA %s", space_str.c_str(), getName().c_str());

    if (isDefault()) io_manager.write(" DEFAULT");

    if (hasGenerated()) io_manager.write(" GENERATED");

    io_manager.write(" \n");

    ViaRule* via_rule = getViaRule();
    if (via_rule) {
        io_manager.write("%s  VIARULE %s ;\n", space_str.c_str(), getViaRule());
        io_manager.write("%s    CUTSIZE %d %d ;\n",
            space_str.c_str(), getCutSizeX(), getCutSizeY());
        io_manager.write("%s    LAYERS %s %s %s ;\n", space_str.c_str(),
            getLowerLayer()->getName(), getCutLayer()->getName(), getUpperLayer()->getName());
        io_manager.write("%s    CUTSPACING %d %d ;\n", space_str.c_str(),
            getCutSpacingX(), getCutSpacingY());
        io_manager.write("%s    ENCLOSURE %d %d %d %d ;\n", space_str.c_str(),
            getLowerEncX(), getLowerEncY(), getUpperEncX(), getUpperEncY());
        if (isArray()) {
            io_manager.write("%s    ROWCOL %d %d ;\n", space_str.c_str(),
                getRow(), getCol());
        }

        if (hasOrigin()) {
            io_manager.write("%s    ORIGIN %d %d ;\n", space_str.c_str(),
                getOffsetX(), getOffsetY());
        }

        if (hasOffset()) {
            io_manager.write("%s    OFFSET %d %d %d %d ;\n",
                space_str.c_str(), getLowerOffsetX(),
                getLowerOffsetY(), getUpperOffsetX(), getUpperOffsetY());
        }
        if (cut_patten_index_) {
            io_manager.write("%s    PATTERN %s ;\n",
                space_str.c_str(), getCutPatten().c_str());
        }
    }

    if (getResistance())
        io_manager.write("%s   RESISTANCE ", space_str.c_str());
        io_manager.write("%g ;\n", getResistance());

    if (via_layers_) {
        IdArray* via_layer_vector = addr<IdArray>(via_layers_);
        for (IdArray::iterator iter = via_layer_vector->begin();
             iter != via_layer_vector->end(); ++iter) {
            ObjectId id = (*iter);
            ViaLayer* via_layer = addr<ViaLayer>(id);
            via_layer->printLEF(io_manager, num_spaces);
        }
    }

    PropertyManager* table = top_cell->getPropertyManager(); 
    std::vector<std::string> properties;
    table->getSparsePropertyList<ViaMaster>(this, properties);
    if (properties.size() > 0) {
        for(int i = 0; i < properties.size(); i++) {
            std::string header = "    PROPERTY ";
            io_manager.write(header);
            PropertyDefinition* pd = getTechLib()->getPropertyDefinition(properties.at(i).c_str());
            switch (pd->getDataType())
            {
            case PropDataType::kInt:
                io_manager.write("%s %d ",
                                  properties.at(i).c_str(), 
                                  table->getPropertyInt<ViaMaster>
                                  (this, properties.at(i).c_str()));
                break;
            case PropDataType::kReal:
                io_manager.write("%s %g ",
                                  properties.at(i).c_str(),
                                   table->getPropertyDouble<ViaMaster>
                                   (this, properties.at(i).c_str()));
                break;
            default:
                {   
                    io_manager.write("%s \"%s\" ",
                                      properties.at(i).c_str(),
                                      table->getPropertyString<ViaMaster>
                                      (this, properties.at(i).c_str()).c_str());
                }
                break;
            }
            io_manager.write(";\n");
        }
    }

    io_manager.write("%sEND %s\n\n",
        space_str.c_str(), getName().c_str());
}

}  // namespace db
}  // namespace open_edi
