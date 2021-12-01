/* @file  db_init.h
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#ifndef EDI_DB_CORE_DB_INIT_H_
#define EDI_DB_CORE_DB_INIT_H_
#include "db/tech/layer.h"
#include "db/util/hv_tree.h"
namespace open_edi {
namespace db {
// run time clase section

class LayerArray {
  public:
    LayerArray();
    ~LayerArray();

    HVTree<Object>* getHVtree(std::string layer_name);
    HVTree<Object>* getHVtree(Layer* layer);
    HVTree<Object>* getHVtree(int z);

    void layerInit();
    void insert(std::pair<Layer*, HVTree<Object>> pair);

    Layer* getLayer(int z);
    Layer* getMetalLayer(int z);
    int getLayerNum();
    int getFirstMetalLayerNum();

    std::map<Layer*, HVTree<Object>*>* getLayerMap();

  private:
    std::map<Layer*, HVTree<Object>*> layer_tree_map_;
    int num_;
    bool is_first_metal_set_;
    int first_metal_layer_num_;
};

Layer* getLayerByZ(int z);
void HVtreeInit();
int fetchDB(Box area, std::vector<Object*>* result);

}  // namespace db
}  // namespace open_edi
#endif
