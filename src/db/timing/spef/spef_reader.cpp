/** 
 * @file spef_reader.cpp
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
#include "db/core/timing.h"
#include "util/stream.h"
#include "util/monitor.h"

#include <iostream>
#include <sys/time.h>
#include <time.h>

#include "db/timing/spef/spef_reader.h"

#ifndef NDEBUG
uint32_t total_net_count_ = 0;
uint32_t net_with_bid_ = 0;
uint32_t net_with_loop_ = 0;
uint32_t valid_net_ = 0;
uint32_t node_without_cap_ = 0;
uint32_t total_node_ = 0;
uint32_t res_number = 0;
uint32_t xcap_number = 0;
uint32_t node_res = 0;
bool     first = true;
#endif

namespace SpefReader {

std::string converToLegalName(const char *name) {
    std::string new_name = name;

    auto find = new_name.find('/', 0);
    while (find != std::string::npos) {
        if (find > 0 && (new_name[find-1] != '\\')) {
            new_name.insert(find, 1, '\\');
            ++find;
        }
        find = new_name.find('/', find + 1);
    }
    return new_name;
}

SpefReader::SpefReader(std::string fileName, DesignParasitics *designParasitics)
    : is_name_index_continuous_(true),
      name_index_shift_(UNINIT_UINT32),
      design_parasitics_(designParasitics),
      nets_parasitics_(nullptr),
      dnet_parasitics_(nullptr),
      rnet_parasitics_(nullptr),
      cell_(nullptr),
      net_(nullptr),
      scanner_(nullptr),
      spef_file_name_(fileName),
      record_str_(""),
      par_value_(0.0),
      line_no_(1),
      spef_field_(designParasitics->getSpefField()),
      stage_(ReaderStage::ReadingNameMap),
      is_extended_spef_(false),
      coordinate_x_(DUMMY_COORDINATE),
      coordinate_y_(DUMMY_COORDINATE)
      {}
    //addDesignNetsParasitics();

SpefReader::~SpefReader() {
    #ifndef NDEBUG
    std::cout << "SPEF valid net: " << valid_net_ << std::endl;
    std::cout << "SPEF net with bid: " << net_with_bid_ << std::endl;
    std::cout << "SPEF net with loop: " << net_with_loop_ << std::endl;

    std::cout << "total_node: " << total_node_ << std::endl;
    std::cout << "res_number: " << res_number << std::endl;
    std::cout << "xcap_number: " << xcap_number << std::endl;
    #endif
    if (nets_parasitics_) {
        if (is_name_index_continuous_) {
            // TODO(yemyu): can reduce sparse to save memory, maybe not necessary
            nets_parasitics_->moveNameMap(std::move(name_map_));
        } else {
            std::vector<SymbolIndex> name_map(name_hash_map_.size());
            // use map to maintain original index order
            std::map<uint32_t, SymbolIndex> temp_map(name_hash_map_.begin(), name_hash_map_.end());
            size_t index = 0;
            for (const auto &pair : temp_map) {
                name_map[index++] = pair.second;
            }
            nets_parasitics_->moveNameMap(std::move(name_map));
        }
    }
    design_parasitics_->clearNetMap();
}

void SpefReader::stringDelete(const char *str) const{
    if (str) {
        delete [] str;
    }
}

const char* SpefReader::stringCopy(const char *str) { 
    if (str) {
        char *copy = new char[strlen(str) + 1];
        strcpy(copy, str);
        return copy;
    } else 
        return nullptr;
}

bool SpefReader::isDigits(const char *str) const {
    for (const char *s = str; *s; s++) {
        if (!isdigit(*s))
            return false;
    }
    return true;
}

void SpefReader::setCell(const char *designName) {
    Cell* topCell = getTopCell();
    if (topCell != nullptr) {
	cell_ = topCell->getCell(std::string(designName));
        if (cell_ == nullptr) {
            cell_ == topCell;  //Use top cell for the time being
	    open_edi::util::message->issueMsg(open_edi::util::kError,
                                              "Failed to find SPEF cell in design.\n");
        }
        addDesignNetsParasitics();
    }
    stringDelete(designName);
}

void SpefReader::addDesignNetsParasitics() {
    Timing *timingdb = getTimingLib();
    if (timingdb != nullptr) {
        nets_parasitics_ = timingdb->createObject<NetsParasitics>(kObjectTypeNetsParasitics, timingdb->getId());
        if (nets_parasitics_ == nullptr) {
            open_edi::util::message->issueMsg(open_edi::util::kError,
                                              "Creating NetsParasitics failed.\n");
            return;
        }

        nets_parasitics_->setExtendedSpef(is_extended_spef_);

        if (cell_) {
            nets_parasitics_->setCellId(cell_->getId());
            SymbolIndex spefFileIdx = timingdb->getOrCreateSymbol(spef_file_name_.c_str());
	    if (design_parasitics_) {
                design_parasitics_->addCellId(cell_->getId());
                design_parasitics_->addSpef(spefFileIdx);
	        design_parasitics_->addNetsParasitics(nets_parasitics_->getId());
	    }	 
	}
    }
}

void SpefReader::setTimeScale(float digits, const char* unit) {
    float scale = 1.0;
    if (strcmp(unit, "NS") == 0)
        scale = digits*1.0e-9;
    else if (strcmp(unit, "PS") == 0)
        scale = digits*1.0e-12;
    else {
        open_edi::util::message->issueMsg(open_edi::util::kError,
                                              "Incorrect time unit defined in SPEF.\n");
    }
    nets_parasitics_->setTimeScale(scale);
    stringDelete(unit);
}

void SpefReader::setCapScale(float digits, const char* unit) {
    float scale = 1.0;
    if (strcmp(unit, "PF") == 0)
        scale = digits*1.0e-12;
    else if (strcmp(unit, "FF") == 0)
        scale = digits*1.0e-15;
    else {
        open_edi::util::message->issueMsg(open_edi::util::kError,
                                              "Incorrect capacitance unit defined in SPEF.\n");
    }
    nets_parasitics_->setCapScale(scale);
    stringDelete(unit);
}

void SpefReader::setResScale(float digits, const char* unit) {
    float scale = 1.0;
    if (strcmp(unit, "OHM") == 0)
        scale = digits*1.0;
    else if (strcmp(unit, "KOHM") == 0)
        scale = digits*1.0e3;
    else {
        open_edi::util::message->issueMsg(open_edi::util::kError,
                                              "Incorrect resistance unit defined in SPEF.\n");
    }
    nets_parasitics_->setResScale(scale);
    stringDelete(unit);
}
    
void SpefReader::setInductScale(float digits, const char* unit) {
    float scale = 1.0;
    if (strcmp(unit, "HENRY") == 0)
        scale = digits;
    else if (strcmp(unit, "MH") == 0)
        scale = digits*1.0e-3;
    else if (strcmp(unit, "UH") == 0)
	scale = digits*1.0e-6;
    else {
        open_edi::util::message->issueMsg(open_edi::util::kError,
                                              "Incorrect inductance unit defined in SPEF.\n");
    }
    nets_parasitics_->setInductScale(scale);
    stringDelete(unit);
}

void SpefReader::setDesignFlow(StringVec *dsgFlow) {
    for (auto str : *dsgFlow) {
        nets_parasitics_->addDesignFlow(std::string(str));
        stringDelete(str);
    }
}

/**
 * @brief Add name map for spef, stored as index to SymbolIndex to save memory. First stored as 
 * vector, will be reduced to unordered_map if index is not continuos.
 * 
 * @param index 
 * @param name 
 */
void SpefReader::addNameMap(const char *index, const char *name) {
    if (cell_ && nets_parasitics_) {
        #if SPEF_READER_DEBUG
        auto new_name = converToLegalName(name);
        SymbolIndex nameIdx = cell_->getSymbol(new_name.c_str());
        #else 
        SymbolIndex nameIdx = cell_->getSymbol(name);
        #endif
        if (index[0] == '*') {
            uint32_t idx = strtoul(index+1, NULL, 0);
            if (name_index_shift_ == UNINIT_UINT32) {
                name_index_shift_ = idx;
            }
            size_t i = idx - name_index_shift_;
            if (is_name_index_continuous_ &&
                (i >= name_map_.size() + INDEX_TOLERANCE || idx < name_index_shift_)) {
            // if (is_name_index_continuous_) {
                // reduce to unordered_map instead of vector due to index discontinuity
                is_name_index_continuous_ = false;
                name_hash_map_.reserve(name_map_.size());
                for (size_t j = 0; j < name_map_.size(); ++j) {
                    if (name_map_[j] != kInvalidSymbolIndex)
                        name_hash_map_[j + name_index_shift_] = name_map_[j];
                }
                std::vector<SymbolIndex>().swap(name_map_);
                name_hash_map_[idx] = nameIdx;
            } else if (!is_name_index_continuous_) {
                name_hash_map_[idx] = nameIdx;
            } else {
                // index is still continuos within tolerance
                while (i >= name_map_.size()) {
                    name_map_.push_back(kInvalidSymbolIndex);
                }
                name_map_[i] = nameIdx;
            }
        }
    }
    stringDelete(index);
    stringDelete(name);
}

/**
 * @brief Get SymboleIndex from name map.
 * 
 * @param idx 
 * @return SymbolIndex 
 */
SymbolIndex SpefReader::getNameMap(uint32_t idx) const {
    if (is_name_index_continuous_) {
        return name_map_[idx - name_index_shift_];
    } else {
        auto iter = name_hash_map_.find(idx);
        if (iter != name_hash_map_.end()) {
            return iter->second;
        } else {
            return kInvalidSymbolIndex;
        }
    }
}

void SpefReader::addPort(const char *name) {
    Pin *pin = findPin(name);
    if (nets_parasitics_ && pin)
        nets_parasitics_->addPort(pin->getId());
    stringDelete(name);
}

/*void SpefReader::addPowerNet(char* name) {
    Cell* topCell = getTopCell();
    if (topCell) {
	SpecialNet* snet = topCell->getSpecialNet(name);
	if (snet) 
            nets_parasitics_->addPowerNet(snet->getId());
    }
    stringDelete(name);
}*/

float SpefReader::addParValue(float value1, float value2, float value3) {
    if (spef_field_ == 1) 
        par_value_ = value1;
    else if (spef_field_ == 2)
        par_value_ = value2;
    else if (spef_field_ == 3)
        par_value_ = value3;
    else
        par_value_ = value1;
    return par_value_;
}

Net* SpefReader::findNet(const char *name) const{
    Net *net = nullptr;
    std::string netStr = name;

    if (nets_parasitics_ && cell_ && name) {
        if (name[0] == '*') {
            uint32_t idx = strtoul(name+1, NULL, 0);
            auto symbol_index = getNameMap(idx);
            if (symbol_index != kInvalidSymbolIndex) {
                net = nets_parasitics_->getNetBySymbol(symbol_index);
            }
        } else {
            net = cell_->getNet(netStr);
        }
    }
    stringDelete(name);
    return net;
}

Net* SpefReader::findNet(const std::string &name) const{
    Net *net = nullptr;

    if (nets_parasitics_ && cell_) {
        if (name[0] == '*') {
            const char *c_str = name.c_str();
            uint32_t idx = strtoul(c_str+1, NULL, 0);
            auto symbol_index = getNameMap(idx);
            if (symbol_index != kInvalidSymbolIndex) {
                net = nets_parasitics_->getNetBySymbol(symbol_index);
            }
        } else {
            net = cell_->getNet(name);
        }
    }
    return net;
}

Pin* SpefReader::findPin(const char *name) const {
    if (nets_parasitics_ && cell_ && name) {
        std::string pinStr = name;
        std::size_t found = pinStr.find_last_of(delimiter_);
        if (found != std::string::npos) {
            if (name[0] == '*') {
                uint32_t idx = strtoul(pinStr.substr(1, found).c_str(), NULL, 0);
                auto symbol_index = getNameMap(idx);
                if (symbol_index != kInvalidSymbolIndex)
                    return nets_parasitics_->getPinBySymbol(symbol_index, pinStr.substr(found+1));
            } else {  // Name map doesn't exist
                Inst *inst = cell_->getInstance(pinStr.substr(0, found));
                if (inst)
                    return inst->getPin(pinStr.substr(found+1));
            }
        } else {  // Handle IO pin
            if (name[0] == '*') {
                uint32_t idx = strtoul(name+1, NULL, 0);
                auto symbol_index = getNameMap(idx);
                if (symbol_index != kInvalidSymbolIndex) {
                    return nets_parasitics_->getPortBySymbol(symbol_index);
                }
            } else {
                return cell_->getIOPin(pinStr);
            }
        }
    }
    return nullptr;
}

void SpefReader::addDNetBegin(Net *net) { 
    if (stage_ == ReaderStage::ReadingNameMap) {
        #ifndef NDEBUG
        monitor_.print(ResourceType::kPhysicalMem | ResourceType::kVirtualMem,
                               "Begin read DNET");
        #endif
        stage_ = ReaderStage::ReadingNet;
        // wrap up name mapping data
        if (is_name_index_continuous_) {
            name_map_.shrink_to_fit();
        }
    }
    if (nets_parasitics_ && net) { 
        net_ = net;
        dnet_parasitics_ = nets_parasitics_->addDNetParasitics(net->getId(), par_value_, pin_id_map_); 
        if (net_ && dnet_parasitics_) {
            net_para_map_[net_] = dnet_parasitics_;
        }
        #ifndef NDEBUG
        if (net_set_.find(net) != net_set_.end()) {
            std::cout << "Added duplicate net:" << net->getName() << std::endl;
            assert(false);
        } else {
            net_set_.insert(net);
        }
        #endif
    }
}

void SpefReader::addDNetEnd() {
    if (dnet_parasitics_) {
        // printf("Net %s\n", net_->getName().c_str());
        // dnet_parasitics_->checkLoop();
        #ifndef NDEBUG
        #if SPEF_READER_DEBUG
        if (node_res <= total_node_) {
            std::string meg = "Total node num: ";
            meg += std::to_string(total_node_);
            node_res = total_node_ + 50000;
            monitor_.print(ResourceType::kPhysicalMem | ResourceType::kVirtualMem,
                                   meg.c_str());
        }
        #endif
        ++total_net_count_;
        auto caps = dnet_parasitics_->getGroundCaps();
        auto res = dnet_parasitics_->getResistors();
        auto xcaps = dnet_parasitics_->getCouplingCaps();
        total_node_ += caps.size();
        res_number += res.size();
        xcap_number += 2 * xcaps.size();
        #endif

        #if SPEF_READER_DEBUG
        DNetGraphProcessor processor(*dnet_parasitics_);
        auto forest = processor.getForest(net_);
        #endif
        dnet_parasitics_->postProcess();
        dnet_parasitics_->scale(nets_parasitics_->getCapScale(),
                                nets_parasitics_->getResScale(),
                                nets_parasitics_->getInductScale());
    }
    dnet_parasitics_ = nullptr;

    NodeStrIdMap().swap(node_id_map_);
    PinIdMap().swap(pin_id_map_);
}

void SpefReader::addParasiticNodeCoordinate(const char *pin_name) {
    if (pin_name) {
        if (dnet_parasitics_) {
            std::string pin_str = pin_name;
            getOrCreateParasiticNode(pin_str, DUMMY_CAP, coordinate_x_, coordinate_y_);
        }
        stringDelete(pin_name);
    }
}

/**
 * @brief This is core function of building SPEF data structure. By giving a node name, it 
 * returns existing node id or create a new one with cap. For pin node only, if the cap value is not 
 * DUMMY_CAP, it also overrides the original cap.
 * 
 * @param node_name 
 * @param cap default = DUMMY_CAP, and don't override pin cap
 * @param coordinate_x default = DUMMY_COORDINATE
 * @param coordinate_y default = DUMMY_COORDINATE
 * @return NodeID 
 */
NodeID SpefReader::getOrCreateParasiticNode(const std::string &node_name, float cap,
                                            float coordinate_x, float coordinate_y) {
    auto iter = node_id_map_.find(node_name);
    if (iter == node_id_map_.end()) {
        if (cell_) {
            std::size_t found = node_name.find_last_of(delimiter_);
            if (found != std::string::npos) {
                if (isDigits(node_name.substr(found+1).c_str())) {
                    // handle internal node
                    auto net = findNet(node_name.substr(0, found));
                    if (!net || net != net_) {
                        return INVALID_NODE_ID;
                    }

                    NodeID id = dnet_parasitics_->addGroundCap(cap);
                    node_id_map_[node_name] = id;

                    if (coordinate_x != DUMMY_COORDINATE
                        && coordinate_y != DUMMY_COORDINATE) {
                        dnet_parasitics_->setNodeCoordinate(id, coordinate_x, coordinate_y);
                    }

                    return id;
                } else {
                    // handle Pin node
                    auto pin = findPin(node_name.c_str());
                    if (pin != nullptr) {
                        auto iter = pin_id_map_.find(pin);
                        if (iter == pin_id_map_.end()) {
                            return INVALID_NODE_ID;
                        } else {
                            // !For pin node we may override previous dummy cap
                            if (cap != DUMMY_CAP)
                                dnet_parasitics_->setGroundCap(cap, iter->second);

                            if (coordinate_x != DUMMY_COORDINATE
                                && coordinate_y != DUMMY_COORDINATE) {
                                dnet_parasitics_->setNodeCoordinate(iter->second, coordinate_x, coordinate_y);
                            }

                            return iter->second;
                        }
                    }
                }
            } else {
                // handle IO pin
                auto pin = findPin(node_name.c_str());
                if (pin != nullptr) {
                    auto iter = pin_id_map_.find(pin);
                    if (iter == pin_id_map_.end()) {
                        return INVALID_NODE_ID;
                    } else {
                        // !For IO node we may override previous dummy cap
                        if (cap != DUMMY_CAP)
                            dnet_parasitics_->setGroundCap(cap, iter->second);

                        if (coordinate_x != DUMMY_COORDINATE
                            && coordinate_y != DUMMY_COORDINATE) {
                            dnet_parasitics_->setNodeCoordinate(iter->second, coordinate_x, coordinate_y);
                        }

                        return iter->second;
                    }
                }
            }
        }
        return INVALID_NODE_ID;
    } else {
        if (cap != DUMMY_CAP) {
            dnet_parasitics_->setGroundCap(cap, iter->second);
        }
        return iter->second;
    }
}

void SpefReader::addGroundCap(const char *nodeName) {
    //For gound cap only need to handle internal node as pin node handled in CONN section
    if (nodeName) {
        if (dnet_parasitics_) {
            std::string nodeStr = nodeName;
            getOrCreateParasiticNode(nodeStr, par_value_);
        }
    }
    stringDelete(nodeName);
}

std::tuple<Net*, NodeID> SpefReader::getParasiticExtNode(const std::string &node_name) {
    auto iter = ext_node_id_map_.find(node_name);
    NodeID node_id = INVALID_NODE_ID;
    if (iter == ext_node_id_map_.end()) {
        if (cell_ && dnet_parasitics_) {
            node_id = getOrCreateParasiticNode(node_name);
        }
        if (node_id == INVALID_NODE_ID) {
            return {nullptr, INVALID_NODE_ID};
        } else {
            ext_node_id_map_[node_name] = {net_, node_id};
            return {net_, node_id};
        }
    } else {
        return iter->second;
    }
}

void SpefReader::addCouplingCap(const char *nodeName1, const char *nodeName2) {
    if (nodeName1 && nodeName2) {
        if (dnet_parasitics_) {
            std::string node1Str = nodeName1;
            std::string node2Str = nodeName2;
            NodeID para_node1_id = INVALID_NODE_ID, para_node2_id = INVALID_NODE_ID;
            Net *net1 = nullptr, *net2 = nullptr;
            std::tie(net1, para_node1_id) = getParasiticExtNode(node1Str);
            std::tie(net2, para_node2_id) = getParasiticExtNode(node2Str);
            if (para_node1_id != INVALID_NODE_ID && para_node2_id != INVALID_NODE_ID) {
                net_para_map_[net1]->addCouplingCap(para_node1_id, net2, para_node2_id, par_value_);
                net_para_map_[net2]->addCouplingCap(para_node2_id, net1, para_node1_id, par_value_);
            }
        }
    }
    stringDelete(nodeName1);
    stringDelete(nodeName2);
}

void SpefReader::addResistor(const char *nodeName1, const char *nodeName2) {
    if (nodeName1 && nodeName2 && dnet_parasitics_) {
        if (dnet_parasitics_) {
            std::string node1Str = nodeName1;
            std::string node2Str = nodeName2;
            auto paraNode1Id = getOrCreateParasiticNode(node1Str);
            auto paraNode2Id = getOrCreateParasiticNode(node2Str);
            if (paraNode1Id != INVALID_NODE_ID && paraNode2Id != INVALID_NODE_ID)
                dnet_parasitics_->addResistor(paraNode1Id, paraNode2Id, par_value_);
        }
    }
	stringDelete(nodeName1);
	stringDelete(nodeName2);
}

void SpefReader::addRNetBegin(Net *net) {
    if (stage_ == ReaderStage::ReadingNameMap) {
        #ifndef NDEBUG
        monitor_.print(ResourceType::kPhysicalMem | ResourceType::kVirtualMem,
                               "Begin read RNET");
        #endif
        stage_ = ReaderStage::ReadingNet;
        // wrap up name mapping data
        if (is_name_index_continuous_) {
            name_map_.shrink_to_fit();
        }
    }
    if (nets_parasitics_) {
	net_ = net;
        rnet_parasitics_ = nets_parasitics_->addRNetParasitics(net->getId(), par_value_);
    } 
}

void SpefReader::addRNetDrvr(const char *pinName) {
    if (pinName && rnet_parasitics_) {
        if (rnet_parasitics_) { 
            Pin *pin = findPin(pinName);
            if (pin)
                rnet_parasitics_->setDriverPinId(pin->getId());
	}
	stringDelete(pinName);
    }
}

void SpefReader::addPiModel(float c2, float r1, float c1) {
    if (rnet_parasitics_) {
        rnet_parasitics_->setC2(c2);
        rnet_parasitics_->setR1(r1);
	rnet_parasitics_->setC1(c1);
    }
}

bool SpefReader::parseSpefFile() {

    std::string errMsg = "Failed to open SPEF file: " + spef_file_name_;

    FILE *fspef = fopen(spef_file_name_.c_str(), "r");
    if (fspef == NULL) {
        open_edi::util::message->issueMsg(
                        open_edi::util::kError, errMsg.c_str());
        return false;
    }

    __spef_parse_begin(fspef);
    if (__spef_parse() != 0) {
        errMsg = "Failed to parse SPEF file: " + spef_file_name_;
	open_edi::util::message->issueMsg(
                        open_edi::util::kError, errMsg.c_str());

        __spef_parse_end(fspef);
        fclose(fspef);
        return false;
    }
    __spef_parse_end(fspef); 
    fclose(fspef);
    
    return true;
}

void SpefReader::setCoordinate(float x, float y) {
    coordinate_x_ = x;
    coordinate_y_ = y;
}

}  // namespace SpefReader
