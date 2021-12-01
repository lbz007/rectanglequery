/* @file  data_model.h
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#ifndef SRC_DB_DATA_MODEL_H_
#define SRC_DB_DATA_MODEL_H_

#include "infra/command_manager.h"
#include "db/core/db.h"

namespace open_edi {
namespace db {

using namespace open_edi::infra;
using namespace open_edi::db;

struct LRect {
    Box rect_;
    int layer_id_;
};

class DataModel {

  public:
    void importAllGeometries();
    std::vector<LRect> &getGeometries();
  protected:
    void _importRoutingBlockages();
    void _importIOPins();
    void _importInstances();
    void _importLayerGeometry(LayerGeometry *lg, Inst *inst);
    void _importRNets();
    void _importSNets();
    void _importWire(Wire *wire);
    void _importVia(Via *via);
    void _importPatch(WirePatch *patch);
  private:
    std::vector<LRect> geometries_;
};

}  // namespace db
}  // namespace open_edi

#endif  // SRC_DB_DATA_MODEL_H_
