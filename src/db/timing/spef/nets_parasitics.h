/** 
 * @file nets_parasitics.h
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

#ifndef EDI_DB_TIMING_SPEF_NETS_PARASITICS_H_
#define EDI_DB_TIMING_SPEF_NETS_PARASITICS_H_

#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

#include "db/timing/spef/net_parasitics.h"
#include "db/core/object.h"
#include "db/core/net.h"
#include "db/core/term.h"
#include "db/util/symbol_page.h"


namespace open_edi {
namespace db {

class NetParasitics;

class NetsParasitics : public Object {
  public:
    using BaseType = Object;

    /// @brief default constructor
    NetsParasitics();

    /// @brief destructor
    ~NetsParasitics();

    /// @brief constructor
    NetsParasitics(Object *owner, IndexType id);

    /// @brief copy constructor
    NetsParasitics(NetsParasitics const &rhs);

    /// @brief move constructor
    NetsParasitics(NetsParasitics &&rhs) noexcept;

    /// @brief copy assignment
    NetsParasitics &operator=(NetsParasitics const &rhs);

    /// @brief move assignment
    NetsParasitics &operator=(NetsParasitics &&rhs) noexcept;

    void setDivider(char divider) { divider_ = divider; }
    char getDivider() const { return divider_; }
    void setDelimiter(char delimiter) { delimiter_ = delimiter; }
    char getDelimiter() const { return delimiter_; }
    void setPreBusDel(char prebusdel) { pre_bus_del_ = prebusdel; }
    void setSufBusDel(char sufbusdel) { suf_bus_del_ = sufbusdel; }
    char getPreBusDel() const { return pre_bus_del_; }
    char getSufBusDel() const { return suf_bus_del_; }
    void setCellId(ObjectId cellId) { cell_id_ = cellId; }
    ObjectId getCellId() const { return cell_id_; }
    void addDesignFlow(std::string flowStr);
    ObjectId getDesignFlows() const { return design_flow_vec_id_; }
    void setTimeScale(float scale) { time_scale_ = scale; }
    float getTimeScale() const { return time_scale_; }
    void setCapScale(float scale) { cap_scale_ = scale; }
    float getCapScale() const { return cap_scale_; }
    void setResScale(float scale) { res_scale_ = scale; }
    float getResScale() const { return res_scale_; }
    void setInductScale(float scale) { induct_scale_ = scale; }
    float getInductScale() const { return induct_scale_; }
    void setContainerId(ObjectId id) { container_id_ = id; }
    ObjectId getContainerId() const { return container_id_; }
    void setExtendedSpef(bool extended_spef) { is_extended_spef_ = extended_spef; }
    //Get name map SymbolIndex vector id;
    void addNameMap(uint32_t index, SymbolIndex symIdx);
    void moveNameMap(std::vector<SymbolIndex> &&name_map);
    void addPort(ObjectId portId);
    //Get ports vector Id;
    ObjectId getPorts() const { return ports_vec_id_; }
    //void addNet(ObjectId netId);
    //Get nets vector Id;
    //ObjectId getNets() const { return net_vec_id_; }
    void addNetParasitics(ObjectId netParaId);
    //Get net parasitics vector Id;
    ObjectId getNetParasitics() const { return netparasitics_vec_id_; }
    bool isDigits(const char *str);
    Net* getNetBySymbol(SymbolIndex index);
    Pin* getPinBySymbol(SymbolIndex index, const std::string& pinName);
    Pin* getPortBySymbol(SymbolIndex index);
    NodeID createParaNode(DNetParasitics *netParasitics, const char *nodeName, const PinIdMap &pin_id_map);
    DNetParasitics* addDNetParasitics(ObjectId netId, float totCap, PinIdMap &pin_id_map);
    void addGroundCap(ObjectId netId, char *nodeName, float capValue);
    void addCouplingCap(ObjectId netId, char *nodeName1, char *nodeName2, float xCapValue);
    void addResistor(ObjectId netId, char *nodeName1, char *nodeName2, float resValue);
    RNetParasitics* addRNetParasitics(ObjectId netId, float totCap);
    ///functions for spef dumpping
    void clearTempMapContent();
    std::string getNetDumpName(Net *net);
    std::string getCellDumpName(Cell *cell);
    std::string getIntNodeDumpName(Net *net, NodeID id);
    std::string getPinDumpName(Pin *pin);
    std::string getTermDirDumpName(Pin *pin);
    std::string getNodeDumpName(Net *net, NodeID id, ArrayObject<ObjectId> *pins);
    void dumpCap(std::ofstream& os, uint32_t cap_no, const std::string &node_name, float cap);
    void dumpSpefHeader(std::ofstream& os);
    void dumpNameMap(std::ofstream& os);
    void dumpPorts(std::ofstream& os);
    void dumpDNetConn(std::ofstream& os, DNetParasitics *dNetPara);
    void dumpDNetConnAttr(std::ofstream& os, DNetParasitics *dNetPara, NodeID node_id);
    void dumpDNetCap(std::ofstream& os, DNetParasitics *dNetPara);
    void dumpDNetRes(std::ofstream& os, DNetParasitics *dNetPara);
    void dumpDNet(std::ofstream& os, DNetParasitics *dNetPara);
    void dumpRNet(std::ofstream& os, RNetParasitics *rNetPara);
    void dumpNets(std::ofstream& os);

  protected:
    /// @brief copy object
    void copy(NetsParasitics const &rhs);
    /// @brief move object
    void move(NetsParasitics &&rhs);
    /// @brief overload output stream
    friend std::ofstream &operator<<(std::ofstream &os, NetsParasitics &rhs);

  private:
    // The vector to save name mapping defined in SPEF, only use it on the fly
    std::vector<SymbolIndex> name_map_;
    //  Name map used for spef dumpping, keep it empty after spef dumpping done
    std::unordered_map<std::string, uint32_t> revert_name_map_;
    //  Ports name map used for spef dumpping, keep it empty after spef dumpping done
    std::unordered_map<std::string, uint32_t> revert_ports_map_;
    ObjectId netparasitics_vec_id_;
    ObjectId design_flow_vec_id_;
    ObjectId ports_vec_id_;
    ObjectId cell_id_;
    ObjectId container_id_;
    float time_scale_;
    float res_scale_;
    float cap_scale_;
    float induct_scale_;
    char divider_;
    char delimiter_;
    char pre_bus_del_;
    char suf_bus_del_;
    NodeID node_id_;
    bool is_extended_spef_;
};

}  // namespace db
}  // namespace open_edi

#endif  // EDI_DB_TIMING_SPEF_NETS_PARASITICS_H_
