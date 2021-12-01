/* @file  data_model.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "db/rq/data_model.h"
#include "db/util/transform.h"

namespace open_edi {
namespace db {

void DataModel::importAllGeometries()
{
    _importRoutingBlockages();
    _importIOPins();
    _importInstances();
    _importRNets();
    _importSNets();
}

std::vector<LRect> &DataModel::getGeometries()
{
    return geometries_;
}

void DataModel::_importRoutingBlockages()
{
    Cell* top_cell = getTopCell();
    ObjectId route_blockages = top_cell->getFloorplan()->getRouteBlockages();
    if (route_blockages > 0) {
        IdArray *route_vector = Object::addr< IdArray >(route_blockages);
        if (!route_vector) {
            return;
        }
        for (int i = 0; i < route_vector->getSize(); ++i) {
            Constraint *route_blockage =
                Object::addr<Constraint>((*route_vector)[i]);
            if (!route_blockage) {
                continue;
            }
            if (!route_blockage->hasLayer()) {
                continue;
            }
            LayerGeometry *lg = route_blockage->getLayerGeometry();
            if (!lg) {
                continue;
            }
            _importLayerGeometry(lg, 0);
        }
    }
}

void DataModel::_importIOPins()
{
    Cell* top_cell = getTopCell();
    for (int i = 0; i < top_cell->getNumOfIOPins(); i++) {
        Pin* pin = top_cell->getIOPin(i);
        if (!pin) {
            continue;
        }
        Term *term = pin->getTerm();
        for (int i = 0; i < term->getPortNum(); i++) {
            Port *p = term->getPort(i);
            if (p->getLayerGeometryNum() > 0) {
                for (int j = 0; j < p->getLayerGeometryNum(); j++) {
                    LayerGeometry *lg = p->getLayerGeometry(j);
                    _importLayerGeometry(lg, 0);
                }
            }
        }
    }
}

void DataModel::_importInstances()
{
    Cell* top_cell = getTopCell();
    uint64_t num_components = top_cell->getNumOfInsts();
    ObjectId components = top_cell->getInstances();
    ArrayObject<ObjectId> *component_vector =
        Object::addr< ArrayObject<ObjectId> >(components);
    if (!component_vector) {
        return;
    }
    for (auto iter = component_vector->begin(); iter != component_vector->end();
         ++iter) {
        Inst *instance = Object::addr<Inst>(*iter);
        if (!instance) {
            continue;
        }
        Cell *cell = instance->getMaster();
        if (!cell) {
            continue;
        }
        for (int i = 0; i < cell->getNumOfTerms(); i++) {
            Term *term = cell->getTerm(i);
            for (int i = 0; i < term->getPortNum(); i++) {
                Port *p = term->getPort(i);
                if (p->getLayerGeometryNum() > 0) {
                    for (int j = 0; j < p->getLayerGeometryNum(); j++) {
                        LayerGeometry *lg = p->getLayerGeometry(j);
                        _importLayerGeometry(lg, instance);
                    }
                }
            }
        }
        for (int i = 0; i < cell->getOBSSize(); i++) {
            LayerGeometry *lg = cell->getOBS(i);
            _importLayerGeometry(lg, instance);
        }
    }
}

void DataModel::_importRNets()
{
    Cell* top_cell = getTopCell();
    int nets_num = top_cell->getNumOfNets();
    if (nets_num == 0) return;
    ObjectId nets = top_cell->getNets();
    ArrayObject<ObjectId> *net_vector =
                                   Object::addr< ArrayObject<ObjectId> >(nets);
    for (auto iter = net_vector->begin(); iter != net_vector->end(); ++iter) {
        Net *net = Object::addr<Net>(*iter);
        if (!net) {
            continue;
        }
        if (net->getIsBusNet()) {
            continue;
        }
        ArrayObject<ObjectId>* wire_vector = net->getWireArray();
        if (wire_vector) {
            for (ArrayObject<ObjectId>::iterator iter = wire_vector->begin();
                 iter != wire_vector->end(); ++iter) {
                Wire* wire = nullptr;
                ObjectId id = (*iter);
                if (id) {
                    wire = Object::addr<Wire>(id);
                }
                if (wire) {
                    _importWire(wire);
                }
            }
        }
        ArrayObject<ObjectId>* via_vector = net->getViaArray();
        if (via_vector) {
            for (ArrayObject<ObjectId>::iterator iter = via_vector->begin();
                 iter != via_vector->end(); ++iter) {
                Via* via = nullptr;
                ObjectId id = (*iter);
                if (id) {
                    via = Object::addr<Via>(id);
                }
                if (via) {
                    _importVia(via);
                }
            }
        }
        // patch
        ObjectId patches = 0;
        auto search = getPatchMap().find(net->getId());
        if (search != getPatchMap().end()) {
            patches = search->second;
        }
        if (patches) {
            ArrayObject<ObjectId>* patch_vector =
                Object::addr<ArrayObject<ObjectId>>(patches);
            for (ArrayObject<ObjectId>::iterator iter = patch_vector->begin();
                 iter != patch_vector->end(); ++iter) {
                WirePatch* patch = nullptr;
                ObjectId id = (*iter);
                if (id) {
                    patch = Object::addr<WirePatch>(id);
                }
                if (patch) {
                    _importPatch(patch);
                }
            }
        }
    }
}

void DataModel::_importSNets()
{
    Cell* top_cell = getTopCell();
    int special_nets_num = top_cell->getNumOfSpecialNets();
    if (special_nets_num == 0) return;
    ObjectId special_nets = top_cell->getSpecialNets();
    ArrayObject<ObjectId> *special_net_vector =
        Object::addr< ArrayObject<ObjectId> >(special_nets);
    for (int i = 0; i < special_net_vector->getSize(); ++i) {
        SpecialNet *special_net =
            Object::addr<SpecialNet>((*special_net_vector)[i]);
        if (!special_net) {
            continue;
        }
        ArrayObject<ObjectId>* wire_vector = special_net->getWireArray();
        if (wire_vector) {
            for (ArrayObject<ObjectId>::iterator iter = wire_vector->begin();
                 iter != wire_vector->end(); ++iter) {
                Wire* wire = nullptr;
                ObjectId id = (*iter);
                if (id) {
                    wire = Object::addr<Wire>(id);
                }
                if (wire) {
                    _importWire(wire);
                }
            }
        }
        ArrayObject<ObjectId>* via_vector = special_net->getWireArray();
        if (via_vector) {
            for (ArrayObject<ObjectId>::iterator iter = via_vector->begin();
                 iter != via_vector->end(); ++iter) {
                Via* via = nullptr;
                ObjectId id = (*iter);
                if (id) {
                    via = Object::addr<Via>(id);
                }
                if (via) {
                    _importVia(via);
                }
            }
        }
    }
}

void DataModel::_importWire(Wire *wire)
{
    int ext = getTopCell()->getTechLib()->getLayer(wire->getLayerNum())->getWidth()/2;
    int x = wire->getX();
    int y = wire->getY();
    int height = wire->getHeight();
    int length = wire->getLength();
    Box wire_rect;
    if (wire->isVertical()) {
        wire_rect.setBox(x, y - ext, x + length, y + height + ext);
    } else {
        wire_rect.setBox(x - ext, y, x + length + ext, y + height);
    }
    LRect rect;
    rect.rect_ = wire_rect;
    rect.layer_id_ = wire->getLayerNum();
    geometries_.push_back(rect);
}

void DataModel::_importVia(Via *via)
{
    Point p = via->getLoc();
    int x = p.getX();
    int y = p.getY();
    ViaMaster *via_master = via->getMaster();
    if (via_master->getViaLayerVector()) {
        IdArray* via_layer_vector = Object::addr<IdArray>(via_master->getViaLayerVector());
        for (IdArray::iterator iter = via_layer_vector->begin();
             iter != via_layer_vector->end(); ++iter) {
            ObjectId id = (*iter);
            ViaLayer* via_layer = Object::addr<ViaLayer>(id);
            if (via_layer) {
                int layer_id = getTopCell()->getTechLib()->getLayerByName(via_layer->getName().c_str())->getIndexInLef();
                std::vector<Box> via_rects = via_layer->getRects();
                for (int i = 0; i < via_rects.size(); i++) {
                    Box via_rect = via_layer->getRect(i);
                    via_rect.setBox(via_rect.getLLX() + x,
                                    via_rect.getLLY() + y,
                                    via_rect.getURX() + x,
                                    via_rect.getURY() + y);
                    LRect rect;
                    rect.rect_ = via_rect;
                    rect.layer_id_ = layer_id;
                    geometries_.push_back(rect);
                }
            }
        }
    }
}

void DataModel::_importPatch(WirePatch *patch)
{
    LRect patch_rect;
    patch_rect.rect_.setBox(patch->getX1() + patch->getLocX(),
                            patch->getY1() + patch->getLocY(),
                            patch->getX2() + patch->getLocX(),
                            patch->getY2() + patch->getLocY());
    patch_rect.layer_id_ = patch->getLayerNum();
    geometries_.push_back(patch_rect);
}

// routing blockage needs no transform
// IO pin has been transformed
// Instance pin/obs needs to be transformed from cell
void DataModel::_importLayerGeometry(LayerGeometry *lg, Inst *inst)
{
    Layer *layer = lg->getLayer();
    auto iter_box = lg->getBoxIter();
    for (Box *box = iter_box.getNext(); box != nullptr;
        box = iter_box.getNext()) {
        LRect rect;
        rect.rect_ = *box;
        if (inst) {
            transformByInst(inst, rect.rect_);
        }
        rect.layer_id_ = layer->getIndexInLef();
        geometries_.push_back(rect);
    }
}

}  // namespace db
}  // namespace open_edi
