/* @file  via.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#include "db/core/via.h"

#include "db/core/db.h"
namespace open_edi {
namespace db {
/**
 * @brief Construct a new Via:: Via object
 *
 * @param via_master
 */
Via::Via(ViaMaster* via_master) {
    setMaster(via_master);
    status_ = 0;
    orientation_ = 0;
}
/**
 * @brief Construct a new Via:: Via object
 *
 */
Via::Via() {
    master_index_ = 0;
    status_ = 0;
    orientation_ = 0;
}

/**
 * @brief Get the Location object
 *
 * @return Point
 */
Point Via::getLoc() const { return loc_; }

/**
 * @brief Set the Location object
 *
 * @param loc
 */
void Via::setLoc(Point loc) { loc_ = loc; }

/**
 * @brief Set the Location object
 *
 * @param loc
 */
void Via::setLoc(int x, int y) {
    loc_.setX(x);
    loc_.setY(y);
}

/**
 * @brief Get the ViaMaster* object
 *
 * @return ObjectId
 */
ViaMaster* Via::getMaster() const {
    ViaMaster* via_master = nullptr;
    Tech* tech = getTechLib();
    via_master = tech->getViaMasterByIndex(master_index_);
    return via_master;
}

/**
 * @brief Set the Master object
 *
 * @param ViaMaster*
 */
void Via::setMaster(ViaMaster* via_master) {
    Tech* tech = getTechLib();
    if (tech) {
        master_index_ = tech->getViaMasterIndex(via_master);
    } else {
        master_index_ = 0;
    }
}

/**
 * @brief get Via box
 *
 * @return Box
 */
Box Via::getBox() {
    Box box;
    ViaMaster* master = getMaster();
    if (master) {
        int loc_x = loc_.getX();
        int loc_y = loc_.getY();
        int cut_x = master->getCutSizeX();
        int cut_y = master->getCutSizeY();
        if (cut_x && cut_y) {  // cut via type
            box.setBox(loc_x, loc_y, loc_x + cut_x, loc_y + cut_y);
        } else {  // layer via type
            ArrayObject<ObjectId>* layer_arr = master->getViaLayerArray();
            if (layer_arr) {
                ViaLayer* via_layer = Object::addr<ViaLayer>(
                    (*layer_arr)[layer_arr->getSize() - 1]);

                if (via_layer) {
                    Box via_master_box = via_layer->getRect(0);
                    box.setBox(via_master_box.getLLX() + loc_x,
                               via_master_box.getLLY() + loc_y,
                               via_master_box.getURX() + loc_x,
                               via_master_box.getURY() + loc_y);
                }
            }
        }
    }

    return box;
}

/**
 * @brief Get the Orien object
 *
 * @return Bits
 */
Bits Via::getOrient() const { return orientation_; }

/**
 * @brief Set the Orient object
 *
 * @param orient
 */
void Via::setOrient(Bits orient) { orientation_ = orient; }

/**
 * @brief judge Via orientation status
 *
 * @return true
 * @return false
 */
bool Via::isR0() const { return getOrient() == kR0; }

/**
 * @brief judge Via orientation status
 *
 * @return true
 * @return false
 */
bool Via::isR90() const { return getOrient() == kR90; }

/**
 * @brief judge Via orientation status
 *
 * @return true
 * @return false
 */
bool Via::isR180() const { return getOrient() == kR180; }

/**
 * @brief judge Via orientation status
 *
 * @return true
 * @return false
 */
bool Via::isR270() const { return getOrient() == kR270; }

/**
 * @brief judge Via orientation status
 *
 * @return true
 * @return false
 */
bool Via::isMX() const { return getOrient() == kMX; }

/**
 * @brief judge Via orientation status
 *
 * @return true
 * @return falsel
 */
bool Via::isMXR90() const { return getOrient() == kMXR90; }

/**
 * @brief judge Via orientation status
 *
 * @return true
 * @return false
 */
bool Via::isMY() const { return getOrient() == kMY; }

/**
 * @brief judge Via orientation status
 *
 * @return true
 * @return false
 */
bool Via::isMYR90() const { return getOrient() == kMYR90; }

/**
 * @brief Get the Route Status object
 *
 * @return Bits
 */
Bits Via::getRouteStatus() const { return status_; }

/**
 * @brief Set the Route Status object
 *
 * @param status
 */
void Via::setRouteStatus(Bits status) { status_ = status; }

void Via::printDEF(IOManager& io_manager) {
    Tech* lib = getTopCell()->getTechLib();
    Layer* layer = nullptr;
    ViaMaster* via_master = getMaster();

    int layer_index = via_master->getUpperLayerIndex();
    std::string layer_name = "";

    if (layer_index == 0) {
        ObjectId layer_v = via_master->getViaLayerVector();
        ArrayObject<ObjectId>* layer_array =
            Object::addr<ArrayObject<ObjectId>>(layer_v);
        ViaLayer* via_layer =
            Object::addr<ViaLayer>((*layer_array)[layer_array->getSize() - 1]);
        if (via_layer) layer_name = via_layer->getName();
    } else {
        layer = lib->getLayer(layer_index);
        if (layer) layer_name = layer->getName();
    }

    if (via_master) {
        io_manager.write("    NEW %s ( %d %d ) %s\n",
            layer_name.c_str(), loc_.getX(), loc_.getY(),
            via_master->getName().c_str());
    }
}

}  // namespace db
}  // namespace open_edi
