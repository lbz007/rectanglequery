/* @file  db_init.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "db/core/db_init.h"

#include "db/core/db.h"
#include "db/tech/layer.h"
#include "db/tech/tech.h"
namespace open_edi {
namespace db {

static LayerArray* layer_arr = new LayerArray();  // layer hv-tree class
static HVTree<Object> inst_tree;  // instance has no layer, so it is indepent.
static HVTree<Object> pin_tree;

int fetchDB(Box area, std::vector<Object*>* result) {
    inst_tree.search(area, result);  // for inst tree

    std::map<Layer*, HVTree<Object>*>* map = layer_arr->getLayerMap();
    for (auto iter = map->begin(); iter != map->end(); iter++) {
        iter->second->search(area, result);  // for wire and via tree.
    }

    return 0;
}

void insertInst() {
    ArrayObject<ObjectId>* inst_array = nullptr;
    Cell* top_cell = getTopCell();
    if (top_cell) {
        inst_array = top_cell->getInstanceArray();
    }
    if (inst_array) {
        for (auto iter = inst_array->begin(); iter != inst_array->end();
             iter++) {
            Inst* inst = Object::addr<Inst>(*iter);
            inst_tree.insert(inst);
        }
    }
}

void insertWireAndVia() {
    ArrayObject<ObjectId>* net_array = nullptr;

    Cell* top_cell = getTopCell();
    if (top_cell) {
        net_array = top_cell->getNetArray();
    }
    if (net_array) {
        for (auto iter = net_array->begin(); iter != net_array->end(); iter++) {
            Net* net = Object::addr<Net>(*iter);
            if (net) {
                // insert wire box to hvtree
                ArrayObject<ObjectId>* wire_array = nullptr;
                wire_array = net->getWireArray();
                if (wire_array) {
                    for (auto wire_iter = wire_array->begin();
                         wire_iter != wire_array->end(); wire_iter++) {
                        Wire* wire = Object::addr<Wire>(*wire_iter);
                        if (wire) {
                            HVTree<Object>* hv_tree =
                                layer_arr->getHVtree(wire->getLayer());
                            if (hv_tree)
                                hv_tree->insert(static_cast<Object*>(wire));
                        }
                    }
                }

                // insert via box to hvtree
                ArrayObject<ObjectId>* via_array = nullptr;
                via_array = net->getWireArray();
                if (via_array) {
                    for (auto via_iter = via_array->begin();
                         via_iter != via_array->end(); ++via_iter) {
                        Via* via = Object::addr<Via>(*via_iter);

                        if (via) {
                            ViaMaster* via_master = via->getMaster();
                            if (via_master) {
                                HVTree<Object>* hv_tree = layer_arr->getHVtree(
                                    via->getMaster()->getUpperLayer());
                                if (hv_tree)
                                    hv_tree->insert(static_cast<Object*>(via));
                            }
                        }
                    }
                }
            }
        }
    }
}

Layer* getLayerByIndex(int index) {
    Layer* layer = layer_arr->getLayer(index);
    return layer;
}

void HVtreeInit() {
    layer_arr->layerInit();
    insertInst();
    insertWireAndVia();
}

// run time layer class section
void LayerArray::layerInit() {
    ArrayObject<ObjectId>* layer_array = nullptr;
    Tech* tech = nullptr;
    tech = getTopCell()->getTechLib();
    if (tech) layer_array = tech->getLayerArray();
    if (layer_array) {
        for (auto iter = layer_array->begin(); iter != layer_array->end();
             iter++) {
            Layer* layer = Object::addr<Layer>(*iter);
            if (layer) {
                HVTree<Object>* hv_tree = new HVTree<Object>;
                layer_tree_map_.insert(
                    std::pair<Layer*, HVTree<Object>*>(layer, hv_tree));
                // record first metal layer
                if (!is_first_metal_set_ && layer->isRouting() &&
                    !layer->isBackside()) {
                    first_metal_layer_num_ = num_;
                    is_first_metal_set_ = true;
                }
            }
        }
    }
}

HVTree<Object>* LayerArray::getHVtree(Layer* layer) {
    if (!layer) return nullptr;
    auto search = layer_tree_map_.find(layer);
    if (search != layer_tree_map_.end()) {
        return search->second;
    }

    return nullptr;
}

HVTree<Object>* LayerArray::getHVtree(int layer_num) {
    Tech* lib = getTopCell()->getTechLib();
    Layer* layer = nullptr;
    layer = lib->getLayer(layer_num);
    return getHVtree(layer);
}

HVTree<Object>* LayerArray::getHVtree(std::string layer_name) {
    Tech* lib = getTopCell()->getTechLib();
    Layer* layer = nullptr;
    layer = lib->getLayerByName(layer_name.c_str());
    return getHVtree(layer);
}

LayerArray::LayerArray() {
    first_metal_layer_num_ = 0;
    is_first_metal_set_ = false;
}

LayerArray::~LayerArray() {
    for (auto iter = layer_tree_map_.begin(); iter != layer_tree_map_.end();
         iter++) {
        if (iter->second) delete iter->second;
    }
}

Layer* LayerArray::getLayer(int z) {
    if (z > layer_tree_map_.size()) return nullptr;
    auto iter = layer_tree_map_.begin();
    for (int i = 0; i < z; i++) {
        ++iter;
    }

    return iter->first;
}

Layer* LayerArray::getMetalLayer(int metal_z) {
    if (metal_z <= 0) return nullptr;
    int z = 2 * (metal_z - 1) + first_metal_layer_num_;
    return getLayer(z);
}

std::map<Layer*, HVTree<Object>*>* LayerArray::getLayerMap() {
    return &layer_tree_map_;
}

}  // namespace db
}  // namespace open_edi
