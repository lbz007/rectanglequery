/** 
 * @file net_parasitics.cpp
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

#include "db/timing/spef/net_parasitics.h"
#include <iostream>
#include <map>
#include <unordered_set>

#include "db/core/db.h"
#include "db/core/timing.h"
#include "util/stream.h"
#include "util/util_mem.h"

#ifndef NDEBUG
extern uint32_t net_with_bid_;
extern uint32_t valid_net_;
extern uint32_t net_with_loop_;
extern uint32_t res_number;
extern uint32_t xcap_number;
#endif

namespace open_edi {
namespace db {

static const uint32_t RESERVED_SIZE = 32;

NetParasitics::NetParasitics()
    : NetParasitics::BaseType(),
      net_id_(UNINIT_OBJECT_ID),
      container_id_(UNINIT_OBJECT_ID),
      total_cap_(0.0) {
    setObjectType(ObjectType::kObjectTypeNetParasitics);
}

NetParasitics::~NetParasitics() {
}

DNetParasitics::DNetParasitics()
    : NetParasitics() {
    gcaps_.reserve(RESERVED_SIZE);
    xcaps_.reserve(RESERVED_SIZE);
    resistors_.reserve(2 * RESERVED_SIZE);
    node_coordinates_ = nullptr;
    setObjectType(ObjectType::kObjectTypeDNetParasitics);
}

void DNetParasitics::addCouplingCap(NodeID this_node, Net* that_net, NodeID that_node,
                                    float capValue) {
    Timing *timingdb = getTimingLib();
    if (timingdb) {
        xcaps_.push_back(ParasiticXCap(this_node, that_net, that_node, capValue));
    }
}

void DNetParasitics::addResistor(NodeID node1Id, NodeID node2Id, float resValue) {
    Timing *timingdb = getTimingLib();
    if (timingdb) {
        resistors_.push_back(ParasiticResistor(node1Id, node2Id, resValue));
    }
}

void DNetParasitics::postProcess() {
    gcaps_.shrink_to_fit();
    xcaps_.shrink_to_fit();
    resistors_.shrink_to_fit();
    if (node_coordinates_)
        node_coordinates_->shrink_to_fit();
}

DNetParasitics::~DNetParasitics() {
    destroy();
}

void DNetParasitics::setNodeCoordinate(NodeID id, float x, float y) {
    if (node_coordinates_ == nullptr)
        node_coordinates_ = new std::vector<NodeCoordinate>();

    int request_size = id + 1;
    if (request_size > node_coordinates_->capacity())
        node_coordinates_->reserve(request_size * 1.5);

    if (request_size > node_coordinates_->size())
        node_coordinates_->resize(request_size);

    (*node_coordinates_)[id].x = x;
    (*node_coordinates_)[id].y = y;
}

void DNetParasitics::scale(float cap_scale, float res_scal, float /*induct_scale*/) {
    for (auto& cap : gcaps_)
        cap *= cap_scale;

    for (auto& xcap : xcaps_)
        xcap.setCapacitance(xcap.getCapacitance() * cap_scale);

    for (auto& res : resistors_) {
        res.setResistance(res.getResistance() * res_scal);
    }
}

void DNetParasitics::setNetId(ObjectId netId, PinIdMap &pin_id_map) {
    NetParasitics::setNetId(netId);
    auto net = Object::addr<Net>(netId);
    auto pin_array = net->getPinArray();
    Timing *timingdb = getTimingLib();
    if (timingdb && pin_array) {
        for (auto pin_id : (*pin_array)) {
            auto pin_ptr = Object::addr<Pin>(pin_id);
            // DUMMY_CAP for now, will set cap val later
            auto id = addGroundCap(DUMMY_CAP);
            pin_id_map[pin_ptr] = id;
        }
    }
}

void DNetParasitics::destroy() {
    if (node_coordinates_) {
        node_coordinates_->clear();
        delete node_coordinates_;
        node_coordinates_ = nullptr;
    }
}

RNetParasitics::RNetParasitics()
    : NetParasitics(),
      c2_(0.0),
      r1_(0.0),
      c1_(0.0) {
    setObjectType(ObjectType::kObjectTypeRNetParasitics);
}

/**
 * @brief Construct a new DNetGraphProcessor::DNetGraphProcessor object
 * 
 * @param dnet 
 */
DNetGraphProcessor::DNetGraphProcessor(const DNetParasitics &dnet) {
    // build cpas vector
    auto gcaps = dnet.getGroundCaps();
    caps_.reserve(gcaps.size());
    for (auto cap : gcaps) {
        caps_.push_back(cap);
    }

    // build coordinates vector
    auto node_coordinates = dnet.getNodeCoordinates();
    if (node_coordinates) {
        coordinates_.reserve(node_coordinates->size());
        for (auto coordinate : *node_coordinates) {
            coordinates_.push_back(coordinate);
        }
    }

    // handle pin node and add out and bidir to roots
    auto net = Object::addr<Net>(dnet.getNetId());
    auto pin_array = net->getPinArray();
    NodeID node_id = 0;
    if (pin_array) {
        for (const auto &pin_id : (*pin_array)) {
            auto pin = Object::addr<Pin>(pin_id);
            if (pin->getDirection() == SignalDirection::kOutput ||
                pin->getDirection() == SignalDirection::kInout) {
                roots_.push_back(node_id);
            }
            pin_to_id_[pin] = node_id;
            ++node_id;
        }
    }

    // build adjacent_map_
    auto &&resistors = dnet.getResistors();
    for (const auto &r : resistors) {
        NodeID node1_id = r.getNode1Id();
        NodeID node2_id = r.getNode2Id();
        addAdjacentEdge(node1_id, &r);
        addAdjacentEdge(node2_id, &r);
    }
}

void DNetGraphProcessor::addAdjacentEdge(NodeID from, const ParasiticResistor *resistance) {
    auto iter = adjacent_map_.find(from);
    if (iter == adjacent_map_.end()) {
        adjacent_map_[from] = {resistance};
    } else {
        iter->second.push_back(resistance);
    }
}

/**
 * @brief get RC tree
 * 
 * @param pin 
 * @return std::vector<OptParaNode> 
 */
std::vector<OptParaNode> DNetGraphProcessor::getRcTree(Pin *pin) const {
    bool has_loop = false;
    auto node_size = caps_.size();
    auto coordinates_size = coordinates_.size();
    auto it = pin_to_id_.find(pin);
    auto net = pin->getNet();
    auto pins = net->getPinArray();
    auto pin_size = pins->getSize();

    auto getCoordinateX = [&](NodeID id)->float {
        return (id < coordinates_.size() ? coordinates_[id].x : DUMMY_COORDINATE);
    };

    auto getCoordinateY = [&](NodeID id)->float {
        return (id < coordinates_.size() ? coordinates_[id].y : DUMMY_COORDINATE);
    };

    if (it == pin_to_id_.end()) {
        assert(false);
        return {};
    }
    auto root = it->second;

    std::vector<OptParaNode> tree;
    std::unordered_set<const ParasiticResistor*> visited_r;
    std::vector<size_t> node_to_index(node_size, SIZE_MAX);
    std::vector<NodeID> index_to_node(node_size, INVALID_NODE_ID);

    tree.reserve(node_size);
    node_to_index[root] = 0;
    index_to_node[0] = root;
    tree.push_back({0, 0, caps_[root], getCoordinateX(root), getCoordinateY(root), pin});
    for (size_t i = 0; i < tree.size(); ++i) {
        // To reduce runtime, using pre-reversed vector to do queueing.
        NodeID node = index_to_node[i];
        auto res_it = adjacent_map_.find(node);
        if (res_it == adjacent_map_.end()) {
            continue;
        } else {
            auto resistances = res_it->second;
            for (auto res : resistances) {
                if (visited_r.count(res)) {
                    continue;
                }
                visited_r.insert(res);
                auto next_node = node == res->getNode1Id()?res->getNode2Id():res->getNode1Id();

                #ifndef NDEBUG
                if (node_to_index[next_node] != SIZE_MAX) {
                    // already visted
                    has_loop = true;
                    continue;
                }
                if (has_loop) {
                    std::cout << "HAS LOOP!!!" << std::endl;
                    std::cout << net->getName() << std:: endl;
                    ++net_with_loop_;
                }
                #endif

                if (next_node < pin_size) {
                    pin = Object::addr<Pin>((*pins)[next_node]);
                } else {
                    pin = nullptr;
                }
                tree.push_back({i,                          /* parent_node */
                                res->getResistance(),       /* resistance */
                                caps_[next_node],
                                getCoordinateX(next_node),
                                getCoordinateY(next_node),
                                pin
                                });
                node_to_index[next_node] = tree.size() - 1;
                index_to_node[tree.size() - 1] = next_node;
            }
        }
    }
    return tree;
}

/**
 * @brief get RC forest
 * 
 * @param net 
 * @return std::vector<std::vector<OptParaNode>> 
 */
std::vector<std::vector<OptParaNode>> DNetGraphProcessor::getForest(const Net* net) const {
    std::vector<std::vector<OptParaNode>> parasitic_forest;
    auto node_size = caps_.size();
    auto pins = net->getPinArray();
    if (!pins) {
        return {};
    }
    auto pin_size = pins->getSize();
    Pin *pin = nullptr;

    #ifndef NDEBUG
    bool has_loop = false;
    ++valid_net_;
    #endif

    if (roots_.empty()) {
        return {};
    }
    for (auto root : roots_) {
        assert(root < pin_size);
        auto tree = getRcTree(Object::addr<Pin>((*pins)[root]));
        assert(selfCheck(net, tree));
        parasitic_forest.push_back(std::move(tree));
    }
    return parasitic_forest;
}

bool DNetGraphProcessor::selfCheck(const Net *net, const std::vector<OptParaNode> &tree) const {
    bool ok = true;
    auto size = tree.size();
    auto pins = net->getPinArray();
    auto pin_size = pins->getSize();
    size_t tree_pin_size = 0;

    for (int index = 0; index < tree.size(); ++index) {
        if (tree[index].parent >= size) {
            std::cout << "Node index out of boundary!" << std::endl
            << " index: " << index << " parent: " << tree[index].parent << std::endl;
            ok = false;
        }
        if (tree[index].pin) {
            ++tree_pin_size;
        }
    }
    if (tree_pin_size != pin_size) {
        std::cout << "Pin size mismatch! net pins:" << pin_size << " tree pin: " << tree_pin_size
        << std::endl;
        assert(false);
        ok = false;
    }
    // if (tree.size() < 2) {
    //     std::cout << "Pin size less than 2!" << std::endl;
    //     ok = false;
    // }

    if (!ok) {
        std::cout << " Net name: " << net->getName() << std::endl;
    }
    return ok;
}

}  // namespace db
}  // namespace open_edi
