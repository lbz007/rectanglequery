
#include "db/core/pin.h"

#include "db/core/db.h"
#include "db/core/inst.h"
#include "db/core/net.h"
#include "db/util/transform.h"

namespace open_edi {
namespace db {

std::map<ObjectId, bool> primary_map;
std::map<ObjectId, bool> special_map;

Pin::Pin() {
    is_connect_nets_ = false;
    term_index_ = -1;
    inst_ = 0;
    net_ = 0;
}

Pin::Pin(Pin const& rhs) { copy(rhs); }

Pin::Pin(Pin&& rhs) noexcept { move(std::move(std::move(rhs))); }

bool Pin::setName(std::string name) {
    // Only do it for primary pin, since no duplicate.
    if (getIsPrimary()) {
        StorageUtil *storage_util = getStorageUtil();
        ediAssert(storage_util != nullptr);
        SymbolTable *symtbl = storage_util->getSymbolTable();
        ediAssert(symtbl != nullptr);
        SymbolIndex symbol_index = 
            symtbl->getOrCreateSymbol(name.c_str());
        if (symbol_index == kInvalidSymbolIndex) return false;

        symtbl->addReference(symbol_index, getId());
    }
    return true;
}

std::string& Pin::getName() const {
    Term* master_term = getTerm();
    ediAssert(nullptr != master_term);
    return master_term->getName();
}

Pin& Pin::operator=(Pin const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

Pin& Pin::operator=(Pin&& rhs) noexcept {
    if (this != &rhs) {
        move(std::move(std::move(rhs)));
    }
    return *this;
}

SignalDirection Pin::getDirection() const {
    Term* term = nullptr;
    term = getTerm();
    if (nullptr != term) return term->getDirection();

    return SignalDirection::kUnknown;
}

Term* Pin::getTerm() const {
    if (term_index_ == -1) {
        return nullptr;
    }

    ObjectId term_ = 0;
    ArrayObject<ObjectId> *vct = nullptr;
    Cell* cell = nullptr;
    Inst* inst = nullptr;
    
    inst = getInst();
    if (!inst) {
	cell = getTopCell();
    } else {
	cell = inst->getMaster();
    }
    vct = cell->getTermArray();
    if (vct == nullptr) return nullptr;
    term_ = (*vct)[term_index_];
    return addr<Term>(term_);
}

void Pin::setTerm(Term* v) {
    if (v == nullptr) {
        term_index_ = -1;
    } else {
        ObjectId term_ = v->getId();
        ArrayObject<ObjectId> *vct = nullptr;
        Bits16 idx = -1;
        vct = v->getCell()->getTermArray();

        for (auto iter = vct->begin(); iter != vct->end(); iter++) {
            idx++;
            if (term_ == (*iter)) {
                term_index_ = idx;
                break;

            }
        }
    }
}

Inst* Pin::getInst() const {
    if (!inst_) {
        return nullptr;
    }
    return addr<Inst>(inst_); 
}

void Pin::setInst(Inst* v) {
    if (v == nullptr) {
        inst_ = 0;
    } else {
        inst_ = v->getId();
    }
}

void Pin::getBoxVector(std::vector <Box> & box_vector) const {
    Inst *inst = getInst();
    Term *term = getTerm();
    ediAssert(term != nullptr);
    Box box(0, 0, 0, 0);
    for (int index = 0; index < term->getPortNum(); ++index) {
        Port *port = term->getPort(index);
        for (int layergeom_index = 0;
            layergeom_index < port->getLayerGeometryNum(); ++layergeom_index) {
            LayerGeometry *layer_geom = port->getLayerGeometry(layergeom_index);
            for (int geom_i = 0; geom_i < layer_geom->getVecNum(); ++geom_i) {
            Geometry *geo = layer_geom->getGeometry(geom_i);
            if (geo == nullptr ||
                geo->getType() != GeometryType::kRect) continue;
            Box box = geo->getBox();
            Box originBox = geo->getBox();
            if (inst != nullptr) {
                transformByInst(inst, box);
                box_vector.push_back(box);
            } else {
                transformByIOPin(this, box);
                Box testedBox = box;
                reverseTransformByIOPin(this, testedBox);
#ifdef TEST                
                if (testedBox == originBox) {
                    std::cout << "tested passed.\n";
                } else {
                    std::cout << "failed to reverse.\n";
                }
#endif                
                box_vector.push_back(originBox);
            }
            }
        }
    }
}

Net* Pin::getNet() const {
    if (!net_) {
        return nullptr;
    }
    return addr<Net>(net_); 
}

void Pin::setNet(Net* net) {
    if (net == nullptr) {
        net_ = 0;
    } else {
        net_ = net->getId();
    }
}

void Pin::addNet(Net *net) {
    ArrayObject<ObjectId> *vct = nullptr;

    if (nets_ == 0) {
        nets_ = __createObjectIdArray(8);
    }

    if (nets_ != 0) {
        vct = addr< ArrayObject<ObjectId> >(nets_);
    }

    if (vct) {
        vct->pushBack(net->getId());
        is_connect_nets_ = true;
    }
}

ArrayObject<ObjectId> *Pin::getNetArray() const {
    if (is_connect_nets_) {
        return addr< ArrayObject<ObjectId> >(nets_);
    } else {
        return nullptr;
    }
}

void Pin::copy(Pin const& rhs) {
    setIsSpecial(rhs.getIsSpecial());
    // copy and move don't consider is_primary
    term_index_ = rhs.term_index_;
    inst_ = rhs.inst_;
    net_ = rhs.net_;
}

void Pin::move(Pin&& rhs) {
    setIsSpecial(rhs.getIsSpecial());
    auto it = special_map.find(rhs.getId());
    if (it != special_map.end()) {
        special_map.erase(it);
    }
    term_index_ = std::exchange(rhs.term_index_, 0);
    inst_ = std::exchange(rhs.inst_, 0);
    net_ = std::exchange(rhs.net_, 0);
}

OStreamBase& operator<<(OStreamBase& os, Pin const& rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");

    os << DataFieldName("inst_");
    if (rhs.inst_) {
        os << rhs.inst_;
    } else {
        os << UNINIT_OBJECT_ID;
    }
    os << DataDelimiter();

    os << DataFieldName("term_");
    if (rhs.term_index_) {
        os << rhs.term_index_;
    } else {
        os << UNINIT_OBJECT_ID;
    }

    os << DataDelimiter();
    os << DataFieldName("net_");
    if (rhs.net_) {
        os << rhs.net_;
    } else {
        os << UNINIT_OBJECT_ID;
    }
    os << DataDelimiter();

    os << DataEnd(")");
    return os;
}

IStreamBase& operator>>(IStreamBase& is, Pin& rhs) {
    is >> DataTypeName(className(rhs)) >> DataBegin("(");

    auto context = is.getContext();

    // add current context to Pin
    // is.addContext(&rhs);

    Pin::IndexType inst_id;
    is >> DataFieldName("inst_") >> inst_id >> DataDelimiter();
    if (inst_id == UNINIT_OBJECT_ID) {
        rhs.inst_ = 0;
    } 
    Pin::IndexType net_id;
    is >> DataFieldName("net_") >> net_id >> DataDelimiter();
    if (net_id == UNINIT_OBJECT_ID) {
        rhs.net_ = 0;
    }

    is >> DataEnd(")");
    return is;
}

bool Pin::getIsSpecial() const { 
    auto is_special = special_map.find(getId());
    if (is_special != special_map.end()) {
        return true;
    }
    return false; 
}

void Pin::setIsSpecial(bool flag) {
    // only set true usage now
    if (flag == false) {
        return;
    }
    
    special_map.insert(std::pair<ObjectId, bool>(getId(), 1)); 
}

bool Pin::getIsConnectNets() const { return is_connect_nets_; }

/// @brief getIsPrimary 
///
/// @return 
bool Pin::getIsPrimary() const
{    
    auto is_primary = primary_map.find(getId());
    if (is_primary != primary_map.end()) {
        return true;
    }
    return false; 
}

/// @brief setIsPrimary 
///
/// @param p
void Pin::setIsPrimary(bool flag)
{
    // only set true usage now
    if (flag == false) {
        return;
    }
    
    primary_map.insert(std::pair<ObjectId, bool>(getId(), 1));     
}

bool Pin::getIsIOPin() const
{
    if (!inst_) {
        return true;
    }
    return false; 
}

}  // namespace db
}  // namespace open_edi
