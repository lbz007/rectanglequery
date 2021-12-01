/** 
 * @file spef_reader.h
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

#ifndef EDI_DB_TIMING_SPEF_SPEF_READER_H_
#define EDI_DB_TIMING_SPEF_SPEF_READER_H_

#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <utility>
#include <vector>
#include <ctype.h>
#include <iostream>

#include "stdlib.h"
#include "db/core/object.h"
#include "db/util/array.h"
#include "util/data_traits.h"
#include "db/timing/spef/design_parasitics.h"
#include "db/timing/spef/nets_parasitics.h"

namespace open_edi {
namespace db {
  class DesignParasitics;
  class NetsParasitics;
}
}
using namespace open_edi::db;

namespace SpefReader {

enum class ReaderStage : uint8_t {
    ReadingNameMap = 0x1,
    ReadingNet
};

typedef std::vector<const char*> StringVec;


class SpefReader {
  public:
    SpefReader(std::string fileName, DesignParasitics *designParasitics);
    ~SpefReader();

    void stringDelete(const char *str) const;
    void recordChar(const char ch) { record_str_ += ch; }
    const char* getRecordStr() { return stringCopy(record_str_.c_str()); }
    void clearRecordStr() { record_str_.clear(); }
    const char* stringCopy(const char *str);
    bool isDigits(const char *str) const;
    Cell* getCell() const { return cell_; }
    void setCell(const char *designName);
    void addDesignNetsParasitics();
    void setDivider(const char divider) { if (nets_parasitics_) nets_parasitics_->setDivider(divider); }
    void setDelimiter(const char delimiter) { delimiter_ = delimiter; if (nets_parasitics_) nets_parasitics_->setDelimiter(delimiter); }
    void setPreBusDel(const char prebusdel) { if (nets_parasitics_) nets_parasitics_->setPreBusDel(prebusdel); }
    void setSufBusDel(const char sufbusdel) { if (nets_parasitics_) nets_parasitics_->setSufBusDel(sufbusdel); }
    void setTimeScale(float digits, const char *unit);
    void setCapScale(float digits, const char *unit);
    void setResScale(float digits, const char *unit);
    void setInductScale(float digits, const char *unit);
    void setDesignFlow(StringVec *dsgFlow);
    void addNameMap(const char *index, const char *name);
    SymbolIndex getNameMap(uint32_t idx) const;
    void addPort(const char *name);
    float addParValue(float value1) { par_value_ = value1; return par_value_; }
    float addParValue(float value1, float value2, float value3);
    Net* findNet(const char *name) const;
    Net* findNet(const std::string &name) const;
    Pin* findPin(const char *name) const;
    void addDNetBegin(Net *net);
    void addDNetEnd();
    NodeID getOrCreateParasiticNode(const std::string &node_name, float cap = DUMMY_CAP,
                                    float coordinate_x = DUMMY_COORDINATE,
                                    float coordinate_y = DUMMY_COORDINATE);
    std::tuple<Net*, NodeID> getParasiticExtNode(const std::string &node_name);
    void addGroundCap(const char *nodeName);
    void addCouplingCap(const char *nodeName1, const char *nodeName2);
    void addResistor(const char *nodeName1, const char *nodeName2);
    void addRNetBegin(Net *net);
    void addRNetDrvr(const char *pinName);
    void addPiModel(float c2, float r1, float c1); 
    void addRNetEnd() { net_ = nullptr; rnet_parasitics_ = nullptr; }
    void setSpefField(uint8_t spefFiled) { spef_field_ = spefFiled; }
    uint8_t getSpefField() const { return spef_field_; }
    void incrLineNo() { line_no_++; }
    uint32_t getLineNo() const { return line_no_; }
    std::string getSpefFile() const { return spef_file_name_; }
    bool parseSpefFile();

    bool isExtendedSpef() const { return is_extended_spef_; }
    void setExtendedSpef(bool is_extended) { is_extended_spef_ = is_extended;}
    void setCoordinate(float x, float y);
    void addParasiticNodeCoordinate(const char *pin_name);

  private:
    void __spef_parse_begin(FILE *fp);
    void __spef_parse_end(FILE *fp);
    int  __spef_parse();

    NodeStrIdMap node_id_map_; //use to check if node created for current net
    // std::unordered_map<std::string, NodeID> node_index_map_; //use to check if node created for net
    ExtNodeStrIdMap ext_node_id_map_;
    PinIdMap pin_id_map_;
    NetParaMap net_para_map_;
    bool is_name_index_continuous_;
    uint32_t name_index_shift_;
    // !Because NAME_MAP is not guaranteed to be continuous, we have to use unordered_map to handle
    std::unordered_map<uint32_t, SymbolIndex> name_hash_map_;
    // !Assume name id is continuous first. If not, convert to unordered_map
    std::vector<SymbolIndex> name_map_;
    DesignParasitics *design_parasitics_;
    NetsParasitics *nets_parasitics_;
    DNetParasitics *dnet_parasitics_;
    RNetParasitics *rnet_parasitics_;
    Cell *cell_;
    Net *net_;
    void *scanner_;
    std::string spef_file_name_;
    std::string record_str_;
    float par_value_;
    uint32_t line_no_;
    uint8_t spef_field_;
    char delimiter_;
    Monitor monitor_;
    std::unordered_set<Net*> net_set_;
    ReaderStage stage_;
    bool is_extended_spef_;
    float coordinate_x_;
    float coordinate_y_;
};

}  // namespace SpefReader

#endif  // EDI_DB_TIMING_SPEF_SPEF_READER_H_
