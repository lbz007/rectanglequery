/** 
 * @file parasitic_device.cpp
 * @date 2020-11-02
 * @brief
 *
 * Copyright (C) 2020 NIIC EDA
 *
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 *
 * of the BSD license.  See the LICENSE file for details.
 */

#include "db/core/db.h"
#include "util/stream.h"

#include "db/timing/spef/parasitic_device.h"


namespace open_edi {
namespace db {

ParasiticDevice::ParasiticDevice()
    : node1_id_(INVALID_NODE_ID) {
}

ParasiticDevice::ParasiticDevice(NodeID id)
    : node1_id_(id) {
}

ParasiticResistor::ParasiticResistor()
    : ParasiticDevice(),
      node2_id_(INVALID_NODE_ID),
      resistance_(-1.0) {
}

ParasiticResistor::ParasiticResistor(NodeID node1Id, NodeID node2Id, float resValue)
    : ParasiticDevice(node1Id),
      node2_id_(node2Id),
      resistance_(resValue){
      }

ParasiticXCap::ParasiticXCap()
    : ParasiticDevice(),   
      that_net_(nullptr),
      that_node_id_(INVALID_NODE_ID),
      capacitance_(-1.0) {
}

ParasiticXCap::ParasiticXCap(NodeID this_node, Net* that_net, NodeID that_node, float caps)
    : ParasiticDevice(this_node),
      that_net_(that_net),
      that_node_id_(that_node),
      capacitance_(caps) {}

}  // namespace db
}  // namespace open_edi
