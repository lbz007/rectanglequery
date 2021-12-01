/** 
 * @file design_parasitics.cpp
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

#include <map>

#include "db/timing/spef/design_parasitics.h"
#include "db/timing/spef/nets_parasitics.h"
#include "db/core/db.h"
#include "util/stream.h"

namespace open_edi {
namespace db {

DesignParasitics::DesignParasitics()
    : DesignParasitics::BaseType(),
      cell_vec_id_(UNINIT_OBJECT_ID),
      spef_vec_id_(UNINIT_OBJECT_ID),
      netsparasitics_vec_id_(UNINIT_OBJECT_ID),
      container_id_(UNINIT_OBJECT_ID),
      spef_field_(1),
      net_detailed_para_map_(nullptr) {
    setObjectType(ObjectType::kObjectTypeDesignParasitics);
}

DesignParasitics::~DesignParasitics() {
    delete net_detailed_para_map_;
}

DesignParasitics::DesignParasitics(Object* owner, DesignParasitics::IndexType id)
    : DesignParasitics::BaseType(owner, id),
      cell_vec_id_(UNINIT_OBJECT_ID),
      spef_vec_id_(UNINIT_OBJECT_ID),
      netsparasitics_vec_id_(UNINIT_OBJECT_ID),
      container_id_(UNINIT_OBJECT_ID),
      spef_field_(1),
      net_detailed_para_map_(nullptr) {
    setObjectType(ObjectType::kObjectTypeDesignParasitics);
}

DesignParasitics::DesignParasitics(DesignParasitics const& rhs) { copy(rhs); }

DesignParasitics::DesignParasitics(DesignParasitics&& rhs) noexcept { move(std::move(rhs)); }

DesignParasitics& DesignParasitics::operator=(DesignParasitics const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

DesignParasitics& DesignParasitics::operator=(DesignParasitics&& rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

void DesignParasitics::addCellId(ObjectId cellId) {
    ArrayObject<ObjectId> *vct = nullptr;
    Timing *timingdb = getTimingLib();
    if (timingdb) {
        if (cell_vec_id_ == UNINIT_OBJECT_ID) {
            vct = timingdb->createObject< ArrayObject<ObjectId> >(kObjectTypeArray, timingdb->getId());
            if (vct != nullptr) {
                vct->setPool(timingdb->getPool());
                vct->reserve(10);
                cell_vec_id_ = vct->getId();
            }
        } else
            vct = addr< ArrayObject<ObjectId> >(cell_vec_id_);

        if (vct != nullptr)
            vct->pushBack(cellId);
    }

}

void DesignParasitics::addSpef(SymbolIndex index) {
    ArrayObject<SymbolIndex> *vct = nullptr;
    Timing *timingdb = getTimingLib();
    if (timingdb) {
        if (spef_vec_id_ == UNINIT_OBJECT_ID) {
            vct = timingdb->createObject< ArrayObject<SymbolIndex> >(kObjectTypeArray, timingdb->getId());
            if (vct != nullptr) {
                vct->setPool(timingdb->getPool());
                vct->reserve(16);
                spef_vec_id_ = vct->getId();
            }
        } else
            vct = addr< ArrayObject<SymbolIndex> >(spef_vec_id_);

        if (vct != nullptr)
            vct->pushBack(index);
    }

}

void DesignParasitics::addNetsParasitics(ObjectId netsPara) {
    ArrayObject<ObjectId> *vct = nullptr;
    Timing *timingdb = getTimingLib();
    if (timingdb) {
        if (netsparasitics_vec_id_ == UNINIT_OBJECT_ID) {
            vct = timingdb->createObject< ArrayObject<ObjectId> >(kObjectTypeArray, timingdb->getId());
            if (vct != nullptr) {
                vct->setPool(timingdb->getPool());
                vct->reserve(16);
                netsparasitics_vec_id_ = vct->getId();
            }
        } else
            vct = addr< ArrayObject<ObjectId> >(netsparasitics_vec_id_);

        if (vct != nullptr)
            vct->pushBack(netsPara);
    }

}

void DesignParasitics::copy(DesignParasitics const& rhs) {
    this->BaseType::copy(rhs);

    cell_vec_id_ = rhs.cell_vec_id_;
    spef_vec_id_  = rhs.spef_vec_id_;
    netsparasitics_vec_id_ = rhs.netsparasitics_vec_id_;
    spef_field_ = rhs.spef_field_;
    container_id_ = rhs.container_id_;
    net_detailed_para_map_ = new CellNetParaMap(*rhs.net_detailed_para_map_);
}

void DesignParasitics::move(DesignParasitics&& rhs) {
    this->BaseType::move(std::move(rhs));
    cell_vec_id_ = std::move(rhs.cell_vec_id_);
    spef_vec_id_ = std::move(rhs.spef_vec_id_);
    netsparasitics_vec_id_ = std::move(rhs.netsparasitics_vec_id_);
    container_id_ = std::move(rhs.container_id_);
    spef_field_ = std::move(rhs.spef_field_);
    net_detailed_para_map_ = rhs.net_detailed_para_map_;
    rhs.net_detailed_para_map_ = nullptr;
}

std::ofstream &operator<<(std::ofstream &os, DesignParasitics const &rhs) {
    if (rhs.netsparasitics_vec_id_ != UNINIT_OBJECT_ID) {
        ArrayObject<ObjectId> *objVector =  Object::addr< ArrayObject<ObjectId> >(rhs.netsparasitics_vec_id_);
	for (auto obj : *objVector) {
            NetsParasitics *netsParasitics = Object::addr<NetsParasitics>(obj);
	    if (netsParasitics)
		os << *netsParasitics;
            break;         ///Currently only support dump out first spef file	
	}
    }
    return os;
}

std::unordered_map<const Net *, ObjectId>* 
DesignParasitics::getOrCreateNetParaMap() const {
    if (!net_detailed_para_map_) {
        net_detailed_para_map_ = new CellNetParaMap();
        auto cell_vct = addr<ArrayObject<ObjectId>>(cell_vec_id_);
        // TODO: doesn't support hier for now
        assert(cell_vct->getSize() == 1);
        auto nets_parasitic_vct = addr<ArrayObject<ObjectId>>(netsparasitics_vec_id_);
        auto nets_parasitics = addr<NetsParasitics>((*nets_parasitic_vct)[0]);
        auto para_vct = addr<ArrayObject<ObjectId>>(nets_parasitics->getNetParasitics());
        for (auto &&para_id : *para_vct) {
            //TODO-yemyu: need to handle DNET and RNET seperately
            auto para = Object::addr<DNetParasitics>(para_id);
            (*net_detailed_para_map_)[addr<Net>(para->getNetId())] = para_id;
        }
    }
    return net_detailed_para_map_;
}

std::vector<std::vector<OptParaNode>> 
DesignParasitics::getOptNetParasiticNodes(Net* net) {
    if (cell_vec_id_ == UNINIT_OBJECT_ID) {
        return {};
    }
    getOrCreateNetParaMap();
    auto iter = net_detailed_para_map_->find(net);
    if (iter != net_detailed_para_map_->end()) {
        DNetGraphProcessor processor(*Object::addr<DNetParasitics>(iter->second));
        return processor.getForest(net);
    } else {
        return {};
    }
}

/**
 * @brief get DNet from net pointer
 * 
 * @param net 
 * @return ObjectId 
 */
ObjectId DesignParasitics::getDNet(const Net *net) const {
    getOrCreateNetParaMap();
    if (!net_detailed_para_map_) return UNINIT_OBJECT_ID;
    auto iter = net_detailed_para_map_->find(net);
    if (iter == net_detailed_para_map_->end()) return UNINIT_OBJECT_ID;
    return iter->second;
}


bool DesignParasitics::setDNet(const Net *net, DNetParasitics &dnet) {
    if (!net_detailed_para_map_) return false;
    (*net_detailed_para_map_)[net] = dnet.getId();
    return true;
}

}  // namespace db
}  // namespace open_edi