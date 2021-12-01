/** 
 * @file parasitic_device.h
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

#ifndef EDI_DB_TIMING_SPEF_PARASITIC_DEVICE_H_
#define EDI_DB_TIMING_SPEF_PARASITIC_DEVICE_H_

#define SPEF_READER_DEBUG 0

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <tuple>
#include <float.h>

#include "db/core/object.h"
#include "db/core/db.h"
#include "db/util/array.h"
#include "util/data_traits.h"

namespace open_edi {
namespace db {

class DNetParasitics;

using NodeID          = uint32_t;
using PinIdMap        = std::unordered_map<Pin*, NodeID>;
using NodeStrIdMap    = std::unordered_map<std::string, NodeID>;
using NetParaMap      = std::unordered_map<Net*, DNetParasitics*>;
using ExtNodeStrIdMap = std::unordered_map<std::string, std::tuple<Net*, NodeID>>;

static const float DUMMY_CAP = 0;
static const float DUMMY_COORDINATE = FLT_MAX;
static const uint32_t INVALID_NODE_ID = UINT32_MAX;
static const uint32_t UNINIT_UINT32 = UINT32_MAX;
static const uint32_t INDEX_TOLERANCE = 10;
/* SPEF defs END*/

class ParasiticDevice {
 public:
    ParasiticDevice();
    explicit ParasiticDevice(NodeID id);
    ~ParasiticDevice() {}

    void setNode1Id(NodeID node1Id) { node1_id_ = node1Id; }
    NodeID getNode1Id() const { return node1_id_; }

 private:
    NodeID node1_id_;
}__attribute__((packed));

class ParasiticResistor : public ParasiticDevice {
 public:
    ParasiticResistor();
    ParasiticResistor(NodeID node1Id, NodeID node2Id, float resValue);
    ~ParasiticResistor() {}

    void setResistance(float resValue) { resistance_ = resValue; }
    float getResistance() const { return resistance_; }
    void setNode2Id(ObjectId node2Id) { node2_id_ = node2Id; }
    NodeID getNode2Id() const { return node2_id_; }

 private:
    NodeID node2_id_;
    float resistance_;
}__attribute__((packed));

class ParasiticXCap : public ParasiticDevice {
 public:
    ParasiticXCap();
    ParasiticXCap(NodeID this_node, Net* that_net, NodeID that_node, float caps);
    ~ParasiticXCap() {}

    void setCapacitance(float capValue) { capacitance_ = capValue; }
    void setOtherNodeID(NodeID id) { that_node_id_ = id; }
    void setOtherNet(Net *net) { that_net_ = net; }

    float getCapacitance() const { return capacitance_; }
    NodeID getOtherNodeID() const { return that_node_id_; }
    Net* getOtherNet() const { return that_net_; }

 private:
    Net *that_net_;
    NodeID that_node_id_;
    float capacitance_;
}__attribute__((packed));

struct OptParaNode {
    /// ParasiticNode
    size_t      parent;
    float       resistance;
    float       capacitance;
    float       coordinate_x;
    float       coordinate_y;
    Pin*        pin;
};

}  // namespace db
}  // namespace open_edi

#endif  // EDI_DB_TIMING_SPEF_PARASITICS_DEVICE_H_
