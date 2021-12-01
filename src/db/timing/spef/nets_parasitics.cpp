/** 
 * @file nets_parasitics.cpp
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

#include "db/timing/spef/nets_parasitics.h"

#include <stdio.h>
#include <time.h>
#include "stdlib.h"
#include "db/core/db.h"
#include "db/core/timing.h"
#include "util/stream.h"

#ifndef NDEBUG
extern uint32_t net_with_loop_;
extern uint32_t net_with_bid_;
extern uint32_t valid_net_;
#endif

namespace open_edi {
namespace db {

NetsParasitics::NetsParasitics()
    : NetsParasitics::BaseType(),
      revert_name_map_(),
      revert_ports_map_(),
      netparasitics_vec_id_(UNINIT_OBJECT_ID),
      design_flow_vec_id_(UNINIT_OBJECT_ID),
      ports_vec_id_(UNINIT_OBJECT_ID),
      cell_id_(UNINIT_OBJECT_ID),
      container_id_(UNINIT_OBJECT_ID),
      time_scale_(1.0),
      res_scale_(1.0),
      cap_scale_(1.0),
      induct_scale_(1.0),
      divider_('\0'),
      delimiter_('\0'),
      pre_bus_del_('\0'),
      suf_bus_del_('\0'),
      is_extended_spef_(false) {
    setObjectType(ObjectType::kObjectTypeNetsParasitics);
}

NetsParasitics::~NetsParasitics() {
}

NetsParasitics::NetsParasitics(Object* owner, NetsParasitics::IndexType id)
    : NetsParasitics::BaseType(owner, id),
      revert_name_map_(),
      revert_ports_map_(),
      netparasitics_vec_id_(UNINIT_OBJECT_ID),
      design_flow_vec_id_(UNINIT_OBJECT_ID),
      ports_vec_id_(UNINIT_OBJECT_ID),
      cell_id_(UNINIT_OBJECT_ID),
      container_id_(UNINIT_OBJECT_ID),
      time_scale_(1.0),
      res_scale_(1.0),
      cap_scale_(1.0),
      induct_scale_(1.0),
      divider_('\0'),
      delimiter_('\0'),
      pre_bus_del_('\0'),
      suf_bus_del_('\0') {
    setObjectType(ObjectType::kObjectTypeNetsParasitics);
}

NetsParasitics::NetsParasitics(NetsParasitics const& rhs) { copy(rhs); }

NetsParasitics::NetsParasitics(NetsParasitics&& rhs) noexcept { move(std::move(rhs)); }

NetsParasitics& NetsParasitics::operator=(NetsParasitics const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

NetsParasitics& NetsParasitics::operator=(NetsParasitics&& rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

void NetsParasitics::copy(NetsParasitics const& rhs) {
    this->BaseType::copy(rhs);

    name_map_ = rhs.name_map_;
    revert_name_map_ = rhs.revert_name_map_;
    revert_ports_map_ = rhs.revert_ports_map_;
    netparasitics_vec_id_ = rhs.netparasitics_vec_id_;
    design_flow_vec_id_ = rhs.design_flow_vec_id_;
    ports_vec_id_ = rhs.ports_vec_id_;
    cell_id_ = rhs.cell_id_;
    container_id_ = rhs.container_id_;
    time_scale_ = rhs.time_scale_;
    res_scale_ = rhs.res_scale_;
    cap_scale_ = rhs.cap_scale_;
    induct_scale_ = rhs.induct_scale_;
    divider_ = rhs.divider_;
    delimiter_ = rhs.delimiter_;
    pre_bus_del_ = rhs.pre_bus_del_;
    suf_bus_del_ = rhs.suf_bus_del_;
}

void NetsParasitics::move(NetsParasitics&& rhs) {
    this->BaseType::move(std::move(rhs));
    name_map_ = std::move(rhs.name_map_);
    revert_name_map_ = std::move(rhs.revert_name_map_);
    revert_ports_map_ = std::move(rhs.revert_ports_map_);
    netparasitics_vec_id_ = std::move(rhs.netparasitics_vec_id_);
    design_flow_vec_id_ = std::move(rhs.design_flow_vec_id_);
    ports_vec_id_ = std::move(rhs.ports_vec_id_);
    cell_id_ = std::move(rhs.cell_id_);
    container_id_ = std::move(rhs.container_id_);
    time_scale_ = std::move(rhs.time_scale_);
    res_scale_ = std::move(rhs.res_scale_);
    cap_scale_ = std::move(rhs.cap_scale_);
    induct_scale_ = std::move(rhs.induct_scale_);
    divider_ = std::move(rhs.divider_);
    delimiter_ = std::move(rhs.delimiter_);
    pre_bus_del_ = std::move(rhs.pre_bus_del_);
    suf_bus_del_ = std::move(rhs.suf_bus_del_);
    rhs.name_map_.clear();
    rhs.revert_name_map_.clear();
    rhs.revert_ports_map_.clear();
    
}

void NetsParasitics::addDesignFlow(std::string flowStr) {
    Timing *timingdb = getTimingLib();
    if (timingdb) {
        ArrayObject<SymbolIndex> *vctSym = nullptr;
        if (design_flow_vec_id_ == UNINIT_OBJECT_ID) {
            vctSym = timingdb->createObject< ArrayObject<SymbolIndex> >(kObjectTypeArray, timingdb->getId());
            if (vctSym == nullptr)
                return;
            vctSym->setPool(timingdb->getPool());
	    vctSym->reserve(5);
            design_flow_vec_id_ = vctSym->getId();
        } else {
            vctSym = addr< ArrayObject<SymbolIndex> >(design_flow_vec_id_);
        }
        if (vctSym != nullptr) {
	    SymbolIndex symIdx = timingdb->getOrCreateSymbol(flowStr.c_str());
            vctSym->pushBack(symIdx);
	}
    }

}

/**
 * @brief We should reuse name_map from SpefReader
 * 
 * @param name_map 
 */
void NetsParasitics::moveNameMap(std::vector<SymbolIndex> &&name_map) {
    name_map_ = std::move(name_map);
    name_map_.shrink_to_fit();
}

void NetsParasitics::addPort(ObjectId portId) { 
    Timing *timingdb = getTimingLib();
    if (timingdb) {
        ArrayObject<ObjectId> *vctPort = nullptr;
        if (ports_vec_id_ == UNINIT_OBJECT_ID) {
            vctPort = timingdb->createObject< ArrayObject<ObjectId> >(kObjectTypeArray, timingdb->getId());
            if (vctPort == nullptr)
                return;
            vctPort->setPool(timingdb->getPool());
            vctPort->reserve(64);
            ports_vec_id_ = vctPort->getId();
        } else {
            vctPort = addr< ArrayObject<ObjectId> >(ports_vec_id_);
        }
        if (vctPort != nullptr)
            vctPort->pushBack(portId);
    }

}

void NetsParasitics::addNetParasitics(ObjectId netParaId) {
    Timing *timingdb = getTimingLib();
    if (timingdb) {
        ArrayObject<ObjectId> *vctNetPara = nullptr;
        if (netparasitics_vec_id_ == UNINIT_OBJECT_ID) {
            vctNetPara = timingdb->createObject< ArrayObject<ObjectId> >(kObjectTypeArray, timingdb->getId());
            if (vctNetPara == nullptr)
                return;
            vctNetPara->setPool(timingdb->getPool());
            vctNetPara->reserve(64);
            netparasitics_vec_id_ = vctNetPara->getId();
        } else {
            vctNetPara = addr< ArrayObject<ObjectId> >(netparasitics_vec_id_);
        }
        if (vctNetPara != nullptr)
            vctNetPara->pushBack(netParaId);
    }

}

bool NetsParasitics::isDigits(const char *str)
{
  for (const char *s = str; *s; s++) {
    if (!isdigit(*s))
      return false;
  }
  return true;
}

Net* NetsParasitics::getNetBySymbol(SymbolIndex index) {
    Cell *cell = Object::addr<Cell>(cell_id_);
    if (cell) {
        std::vector<ObjectId> objectVec = cell->getSymbolTable()->getReferences(index);
        for (auto obj : objectVec) {
            Net *net = Object::addr<Net>(obj);
            if (net && net->getObjectType() == kObjectTypeNet) {
                return net;
            }
        }
    }
    return nullptr;
}   


Pin* NetsParasitics::getPinBySymbol(SymbolIndex index, const std::string& pinName) {
    Cell *cell = Object::addr<Cell>(cell_id_);
    if (cell) {
        std::vector<ObjectId> objectVec = cell->getSymbolTable()->getReferences(index);
	for (auto obj : objectVec) {
	    Inst *inst = Object::addr<Inst>(obj);
            if (inst && inst->getObjectType() == kObjectTypeInst) {
		return inst->getPin(pinName);	
	    }
	}
    }
    return nullptr;
}

Pin* NetsParasitics::getPortBySymbol(SymbolIndex index) {
    Cell *cell = Object::addr<Cell>(cell_id_);
    if (cell) {
        std::vector<ObjectId> objectVec = cell->getSymbolTable()->getReferences(index);
        for (auto obj : objectVec) {
            Pin *pin = Object::addr<Pin>(obj);
            if (pin && pin->getObjectType() == kObjectTypePin) {
                return pin;
            }
        }
    }
    return nullptr;
}


#if 0
NodeID  NetsParasitics::createParaNode(DNetParasitics *netParasitics, const char *nodeName, const PinIdMap &pin_id_map) {
    Cell *cell = Object::addr<Cell>(cell_id_);
    if (cell && nodeName) {
        std::string nodeStr = nodeName;
        std::size_t found = nodeStr.find_last_of(delimiter_);
        Pin *pin = nullptr;
        Net *net = nullptr;
        if (found != std::string::npos) {
	    if (isDigits(nodeStr.substr(found+1).c_str())) {  //Never see use all number for term name
                net = findNet(nodeStr.substr(0, found).c_str());
                if (net == nullptr)
                    return UNINIT_OBJECT_ID;
		else if (net->getId() == netParasitics->getNetId()) { //Internal node
		    uint32_t intNodeId = strtoul(nodeStr.substr(found+1).c_str(), NULL, 0);
                    return netParasitics->createIntNode(intNodeId);
		} else {  //External node
		    uint32_t extNodeId = strtoul(nodeStr.substr(found+1).c_str(), NULL, 0);
		    return netParasitics->createExtNode(net->getId(), extNodeId);
		}
            } else { //Pin node
		pin = findPin(nodeName);
		if (pin != nullptr) {
                    if (pin->getDirection() == SignalDirection::kOutput || pin->getDirection() == SignalDirection::kInout) {
                        auto pin_addr = Object::addr<ParasiticPinNode>(pinNodeId);
                        netParasitics->addRoot(pin_addr);
                    }
                    auto iter = pin_id_map.find(pin);
                    if (iter == pin_id_map.end()) {
                        return INVALID_NODE_ID;
                    } else {
                        return iter->second;
                    }
		}
	    }
        } else { //To handle IO pin
	    pin = findPin(nodeName);
            if (pin != nullptr) {
		ObjectId pinNodeId = netParasitics->createPinNode(pin->getId());
                if (pin->getDirection() == SignalDirection::kOutput || pin->getDirection() == SignalDirection::kInout) {
                    auto pin_addr = Object::addr<ParasiticPinNode>(pinNodeId);
                    netParasitics->addRoot(pin_addr);
                }
                /*if (pin->getNet()->getId() == netParasitics->getNetId()) { //Add internal pin node
                    netParasitics->addPinNode(pinNodeId);
                }*/
		return pinNodeId;
	    } 
	}
    }
    return UNINIT_OBJECT_ID;
}
#endif

DNetParasitics* NetsParasitics::addDNetParasitics(ObjectId netId, float totCap, PinIdMap &pin_id_map) {
    Timing *timingdb = getTimingLib();
    if (timingdb) {
        auto netPara = timingdb->createObject<DNetParasitics>(kObjectTypeDNetParasitics, timingdb->getId());
        if (netPara) {
            netPara->setContainerId(this->getId());
            netPara->setNetId(netId, pin_id_map);
            netPara->setNetTotalCap(totCap);
            ObjectId netParaId = netPara->getId();
            //addNet(netId);
            addNetParasitics(netParaId);
            return netPara;
        }
    }
    return nullptr;
}

RNetParasitics* NetsParasitics::addRNetParasitics(ObjectId netId, float totCap) {
    Timing *timingdb = getTimingLib();
    if (timingdb) {
        auto netPara = timingdb->createObject<RNetParasitics>(kObjectTypeRNetParasitics, timingdb->getId());
        if (netPara) {
            netPara->setContainerId(this->getId());
            netPara->setNetId(netId);
            netPara->setNetTotalCap(totCap);
            ObjectId netParaId = netPara->getId();
            //addNet(netId);
	        addNetParasitics(netParaId);
            return netPara;
        }
    }
    return nullptr;
}

///Functions for SPEF dumpping
void NetsParasitics::clearTempMapContent() {
    revert_name_map_.clear();
    revert_ports_map_.clear();
}


std::string NetsParasitics::getNetDumpName(Net *net) {
    std::string netName = net->getName();
    if (revert_name_map_.find(netName) != revert_name_map_.end())
        netName = "*" + std::to_string(revert_name_map_[netName]);   
    return netName;
}

std::string NetsParasitics::getCellDumpName(Cell *cell) { //for RNet
    std::string cellName = cell->getName();
    if (revert_name_map_.find(cellName) != revert_name_map_.end())
        cellName = "*" + std::to_string(revert_name_map_[cellName]);
    return cellName;
}

std::string NetsParasitics::getNodeDumpName(Net *net, NodeID id, ArrayObject<ObjectId> *pins) {
    if (pins && id < pins->getSize()) {
        // For first pin array size of cpas, they are pin node. Convert to pin name.
        auto pin = Object::addr<Pin>((*pins)[id]);
        return getPinDumpName(pin);
    } else {
        std::string intNodeName = getNetDumpName(net);
        intNodeName += std::string(1,getDelimiter());
        intNodeName += std::to_string(id);
        return intNodeName; 
    }
}

std::string NetsParasitics::getPinDumpName(Pin *pin) {
    std::string pinName = pin->getName();
    if (revert_ports_map_.find(pinName) != revert_ports_map_.end())
        pinName = "*" + std::to_string(revert_ports_map_[pinName]);
    else {
	Inst *inst = pin->getInst();
        if (inst) {
	    std::string instName = inst->getName();
            if (revert_name_map_.find(instName) != revert_name_map_.end()) {
                pinName = "*" + std::to_string(revert_name_map_[instName]);
                pinName += std::string(1,getDelimiter());
                pinName += pin->getTerm()->getName();
            }
	}
    }
    return pinName;
}

std::string NetsParasitics::getTermDirDumpName(Pin *pin) {
    std::string termDirStr = "B";
    Term *term = pin->getTerm();
    if (term) {
        if (term->getDirectionStr() == "Input")
            termDirStr = "I";
        else if (term->getDirectionStr() == "Output")
            termDirStr = "O";
    }
    return termDirStr;
}

void NetsParasitics::dumpSpefHeader(std::ofstream& os) {
    Timing *timingdb = getTimingLib();

    if (is_extended_spef_)
        os << ("// SPEF OUTPUT FLAVOR : extended\n");

    os << ("*SPEF \"IEEE 1481-2009\"\n");

    Cell *cell = Object::addr<Cell>(cell_id_);
    if (cell) {
        std::string cellName = cell->getName();  //Waiting for DB team to fix this issue
        os << ("*DESIGN \"") << (cellName) << ("\"\n");
    }

    time_t rawtime;
    struct tm *timeinfo;
    char timeStr[100];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeStr, 100, "%a %h %d %T %Y", timeinfo);
    os << ("*DATE \"") << (timeStr) << ("\"\n");
    os << ("*VENDOR \"NIIC EDA\"\n");
    os << ("*PROGRAM \"openEDA\"\n");
    os << ("*VERSION \"1.0\"\n");
    os << ("*DESIGN_FLOW");
    if (design_flow_vec_id_ != UNINIT_OBJECT_ID) {
	ArrayObject<SymbolIndex> *vct =  Object::addr< ArrayObject<SymbolIndex> >(design_flow_vec_id_);
	if (vct) {
            for (auto obj : *vct) {
		if (timingdb) {
		    std::string flwStr = timingdb->getSymbolByIndex(obj);
                    os << (" ") << ("\"") << (flwStr) << ("\"");
		}
	    }
	}
    }
    os << ("\n");

    os << ("*DIVIDER ") << (std::string(1,getDivider())) << ("\n");
    os << ("*DELIMITER ") << (std::string(1,getDelimiter())) << ("\n");
    os << ("*BUS_DELIMITER ") << (std::string(1,getPreBusDel()));
    if (getSufBusDel() != '\0')
        os << (std::string(1,getSufBusDel()));
    os << ("\n");
    os << ("*T_UNIT ") << (std::to_string(getTimeScale()*1e12)) << (" PS\n");
    os << ("*C_UNIT ") << (std::to_string(getCapScale()*1e12)) << (" PF\n");
    os << ("*R_UNIT ") << (std::to_string(getResScale())) << (" OHM\n");
    os << ("*L_UNIT ") << (std::to_string(getInductScale())) << (" HENRY\n\n");
}

void NetsParasitics::dumpNameMap(std::ofstream& os) {
    Cell *cell = Object::addr<Cell>(cell_id_);
    os << ("*NAME_MAP\n\n");
    size_t idx = 1;
    for (const auto &symbol : name_map_) {
        if (symbol != kInvalidSymbolIndex) {
            std::string name = cell->getSymbolByIndex(symbol);
            os << ("*") << (std::to_string(idx));
            os << (" ") << (name) << ("\n");
            revert_name_map_[name] = idx;
            idx++;
        }
    }
    os << ("\n\n");
}

void NetsParasitics::dumpPorts(std::ofstream& os) {
    if (ports_vec_id_ != UNINIT_OBJECT_ID) {
        os << ("*PORTS\n\n");
	ArrayObject<ObjectId> *vctObj =  Object::addr< ArrayObject<ObjectId> >(ports_vec_id_);
	if (vctObj) {
            for (auto obj : *vctObj) {
                Pin *pin = Object::addr<Pin>(obj);
                std::string pinName = pin->getName();
                if (revert_name_map_.find(pinName) != revert_name_map_.end()) {  //To use index instead of port name directly
		    revert_ports_map_[pinName] = revert_name_map_[pinName];
                    pinName = std::to_string(revert_name_map_[pinName]);
                }

                os << ("*") << (pinName) << (" ");
                os << (getTermDirDumpName(pin)) << ("\n");
            }
	}
        os << ("\n");
    }
}

void NetsParasitics::dumpDNetConn(std::ofstream& os, DNetParasitics *dNetPara) {
    os << ("*CONN\n");
    ObjectId netId = dNetPara->getNetId(); 
    if (netId != UNINIT_OBJECT_ID) {
        Net *net = Object::addr<Net>(netId);
        if (net) {
	        ArrayObject<ObjectId> *objVector = net->getPinArray();
            if (objVector != nullptr) {
                NodeID cur_id = 0;
                for (auto obj : *objVector) {
                    Pin *pin = Object::addr<Pin>(obj);
                    if (pin != nullptr) {
                        std::string pinName = pin->getName();
                        if (revert_ports_map_.find(pinName) != revert_ports_map_.end())
                            os << ("*P ");
                        else
                            os << ("*I ");

                        os << (getPinDumpName(pin)) << (" ");
                        os << (getTermDirDumpName(pin)) << (" ");
                        dumpDNetConnAttr(os, dNetPara, cur_id);
                        os << ("\n");
                    }
                    cur_id++;
                }
            }
	    }
    }
    
    os << ("\n");
}

 void NetsParasitics::dumpDNetConnAttr(std::ofstream& os, DNetParasitics *dNetPara, NodeID node_id) {
     auto coordinates = dNetPara->getNodeCoordinates();
     if (coordinates == nullptr)
        return;

     if (node_id < coordinates->size()) {
         os << ("*C ") << coordinates->at(node_id).x << (" ");
         os << coordinates->at(node_id).y;
     }
 }

void NetsParasitics::dumpCap(std::ofstream& os, uint32_t cap_no, const std::string &node_name, float cap) {
    os << (std::to_string(cap_no)) << (" ");
    os << node_name;
    os << (" ") << (std::to_string(cap)) << ("\n");
}


void NetsParasitics::dumpDNetCap(std::ofstream& os, DNetParasitics *dNetPara) {
    Net *net = Object::addr<Net>(dNetPara->getNetId());
    NodeID cur_id = 0;
    uint32_t cap_no = 0;

    auto gcaps = dNetPara->getGroundCaps();
    if (gcaps.empty()) return;

    os << ("*CAP\n\n");
    auto pin_arr = net->getPinArray();
    // 1. Handle pin node.
    if (pin_arr) {
        for (auto pin_id : *pin_arr) {
            auto pin = addr<Pin>(pin_id);
            if (gcaps[cur_id] != DUMMY_CAP) {
                dumpCap(os, ++cap_no, getPinDumpName(pin), gcaps[cur_id]);
            }
            ++cur_id;
        }
    }
    // 2. Handle internal node.
    for (; cur_id < gcaps.size(); ++cur_id) {
        if (gcaps[cur_id] != DUMMY_CAP) {
            dumpCap(os, ++cap_no, getNodeDumpName(net, cur_id, pin_arr), gcaps[cur_id]);
        }
    }
    // 3. Handle xcap
    auto &xcaps = dNetPara->getCouplingCaps();
    for (auto &&xcap : xcaps) {
        auto other_net = xcap.getOtherNet();
        auto other_pins = other_net->getPinArray();
        os << (std::to_string(++cap_no)) << (" ");
        os << getNodeDumpName(net, xcap.getNode1Id(), pin_arr) << (" ");
        os << getNodeDumpName(other_net, xcap.getOtherNodeID(), other_pins);
        os << (" ") << (std::to_string(xcap.getCapacitance())) << ("\n");
    }
    os << ("\n"); 
}

void NetsParasitics::dumpDNetRes(std::ofstream& os, DNetParasitics *dNetPara) {
    Net *net = Object::addr<Net>(dNetPara->getNetId());
    auto pins = net->getPinArray();
    os << ("*RES\n\n");
    uint32_t resNo = 0;
    auto &resistors = dNetPara->getResistors();
    for (auto &&res : resistors) {
        resNo++;
        os << (std::to_string(resNo)) << (" ");
        os << (getNodeDumpName(net, res.getNode1Id(), pins)) << (" ");
        os << (getNodeDumpName(net, res.getNode2Id(), pins));
        os << (" ") << (std::to_string(res.getResistance())) << ("\n");
    }
    os << ("\n");
}

void NetsParasitics::dumpDNet(std::ofstream& os, DNetParasitics *dNetPara) {
    Net *net = Object::addr<Net>(dNetPara->getNetId());
    std::string netName = getNetDumpName(net);
    os << ("*D_NET ") << (netName) << (" ");
    os << (std::to_string(dNetPara->getNetTotalCap())) << ("\n\n"); 

    dumpDNetConn(os, dNetPara);
    dumpDNetCap(os, dNetPara);
    dumpDNetRes(os, dNetPara);   
    os << ("*END\n\n");
}

void NetsParasitics::dumpRNet(std::ofstream& os, RNetParasitics *rNetPara) {
    Net *net = Object::addr<Net>(rNetPara->getNetId());
    std::string netName = getNetDumpName(net);
    os << ("*R_NET ") << (netName);
    os << (std::to_string(rNetPara->getNetTotalCap())) << ("\n\n");

    Pin *pin = Object::addr<Pin>(rNetPara->getDriverPinId());   
    Cell *cell = pin->getInst()->getMaster(); 
    os << ("*DRIVER ") << (getPinDumpName(pin)) << ("\n");
    os << ("*CELL ") << (getCellDumpName(cell)) << ("\n");
    os << ("*C2_R1_C1 ") << (std::to_string(rNetPara->getC2()));
    os << (" ") << (std::to_string(rNetPara->getR1()));
    os << (" ") << (std::to_string(rNetPara->getC1()));
    os << ("\n");
    
    os << ("*END\n\n");
}

void NetsParasitics::dumpNets(std::ofstream& os) {
    if (netparasitics_vec_id_ != UNINIT_OBJECT_ID) {
        ArrayObject<ObjectId> *objVector = addr< ArrayObject<ObjectId> >(netparasitics_vec_id_);
        for (auto obj : *objVector) {
            NetParasitics *unObj = Object::addr<NetParasitics>(obj);  //Need to check further
            if (unObj->getObjectType() == ObjectType::kObjectTypeDNetParasitics) {
                DNetParasitics *dNetPara = Object::addr<DNetParasitics>(obj);
                dumpDNet(os, dNetPara);
            } else if (unObj->getObjectType() == ObjectType::kObjectTypeRNetParasitics) {
                RNetParasitics *rNetPara = Object::addr<RNetParasitics>(obj);
                dumpRNet(os, rNetPara);
            }
        }
    }
}

std::ofstream& operator<<(std::ofstream& os, NetsParasitics &rhs) {

    rhs.dumpSpefHeader(os);

    rhs.dumpNameMap(os);

    rhs.dumpPorts(os);

    rhs.dumpNets(os);

    rhs.clearTempMapContent();

    return os;
}

}  // namespace db
}  // namespace open_edi
