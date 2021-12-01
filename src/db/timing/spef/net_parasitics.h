/** 
 * @file net_parasitics.h
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

#ifndef EDI_DB_TIMING_SPEF_NET_PARASITICS_H_
#define EDI_DB_TIMING_SPEF_NET_PARASITICS_H_

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "db/core/object.h"
#include "db/util/array.h"
#include "util/data_traits.h"
#include "parasitic_device.h"

namespace open_edi {
namespace db {


class NetParasitics : public Object {
  public:
    using BaseType = Object;

    /// @brief default constructor
    NetParasitics();
    /// @brief destructor
    ~NetParasitics();
    ObjectId getNetId() const { return net_id_; }
    void setNetId(ObjectId netId) { net_id_ = netId; } 
    float getNetTotalCap() const { return total_cap_; }
    void setNetTotalCap(float totCap) { total_cap_ = totCap; }

    void setContainerId(ObjectId id) { container_id_ = id; }
    ObjectId getContainerId(void) const { return container_id_; }

  private:
    ObjectId net_id_;
    ObjectId container_id_;
    float total_cap_;
};

typedef struct _NodeCoordinate {
    _NodeCoordinate() { x = DUMMY_COORDINATE, y = DUMMY_COORDINATE; }
    float x;
    float y;
} NodeCoordinate;

class DNetParasitics : public NetParasitics {
  public:
    DNetParasitics();
    ~DNetParasitics();
    //ObjectId getPinNodeVecId() const { return pin_node_vec_id_; }
    const std::vector<ParasiticXCap>& getCouplingCaps() const { return xcaps_; }
    const std::vector<ParasiticResistor>& getResistors() const { return resistors_; }
    const std::vector<float>& getGroundCaps() const { return gcaps_; }

    // ObjectId createPinNode(ObjectId pinId);
    // ObjectId createIntNode(uint32_t intNodeId);
    // ObjectId createExtNode(ObjectId netId, uint32_t extNodeId);
    //void addPinNode(ObjectId pinNodeId);
    NodeID addGroundCap(float cap_val) { gcaps_.push_back(cap_val); return gcaps_.size() - 1; };
    void setGroundCap(float cap_val, NodeID id) { if (id < gcaps_.size()) gcaps_[id] = cap_val; };
    void setNetId(ObjectId netId, PinIdMap &pin_id_map);
    void addCouplingCap(NodeID this_node, Net *that_net, NodeID that_node, float capValue);
    void addResistor(NodeID node1Id, NodeID node2Id, float resValue);
    void postProcess();
    void scale(float cap_scale, float res_scal, float induct_scale);
    void setNodeCoordinate(NodeID id, float x, float y);

    std::vector<NodeCoordinate>* getNodeCoordinates() { return node_coordinates_; }
    const std::vector<NodeCoordinate>* getNodeCoordinates() const { return node_coordinates_; }
    void destroy();

  private:
    /// The vector to store Pin Node pointer belongs to this net
    /// Don't need this any more and to use net to get connecting pins instead
    //ObjectId pin_node_vec_id_;
    /// The vector to store Grounded Cap pointer belongs to this net
    /// ArrayObject<ParasiticCap>
    // ObjectId gcap_vec_id_;
    std::vector<float> gcaps_;
    std::vector<ParasiticXCap> xcaps_;
    std::vector<ParasiticResistor> resistors_;
    std::vector<NodeCoordinate>* node_coordinates_;
};

class RNetParasitics : public NetParasitics {
  public:
    RNetParasitics();
    ~RNetParasitics();
     void setDriverPinId(ObjectId drvrPinId) { drvr_pin_id_ = drvrPinId; }
     ObjectId getDriverPinId() const { return drvr_pin_id_; }
     void setC2(float c2) { c2_ = c2; }
     void setR1(float r1) { r1_ = r1; }
     void setC1(float c1) { c1_ = c1; }
     float getC2() const { return c2_; }
     float getR1() const { return r1_; }
     float getC1() const { return c1_; }

  private:
    /// Driver pin ObjectId
    ObjectId drvr_pin_id_;
    float c2_;
    float r1_;
    float c1_;
};

class DNetGraphProcessor {
  public:
    explicit DNetGraphProcessor(const DNetParasitics &dnet);
    std::vector<OptParaNode> getRcTree(Pin *pin) const;
    std::vector<std::vector<OptParaNode>> getForest(const Net *net) const;

  private:
    void prepareGraphData(const DNetParasitics &dnet);
    void addAdjacentEdge(NodeID from, const ParasiticResistor *resistance);
    bool selfCheck(const Net *net, const std::vector<OptParaNode> &tree) const;

    std::vector<NodeID> roots_;
    std::vector<float> caps_;
    std::vector<NodeCoordinate> coordinates_;
    std::unordered_map<NodeID, std::list<const ParasiticResistor*>> adjacent_map_;
    std::unordered_map<Pin*, NodeID> pin_to_id_;
};

}  // namespace db
}  // namespace open_edi

#endif  // EDI_DB_TIMING_SPEF_NETS_PARASITICS_H_
