/* @file  via_master.h
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#ifndef EDI_DB_PHYSICAL_VIA_MASTER_H_
#define EDI_DB_PHYSICAL_VIA_MASTER_H_

#include <string>

#include "db/core/object.h"
#include "db/tech/property.h"
#include "db/util/box.h"
#include "db/tech/via_rule.h"
#include "db/tech/layer.h"
#include "util/io_manager.h"

namespace open_edi {
namespace db {
class ViaRule;

class ViaLayer : public Object {
  public:
    ViaLayer();
    ~ViaLayer();

    SymbolIndex getNameIndex() const;
    bool setName(std::string const& name);
    std::string const& getName();

    void addMask(int num);
    void addRect(Box rect);

    Box getRect(int num);
    int getMaskNum(int num);
    std::vector<Box> getRects();

    void print(int is_def);
    void printLEF(IOManager &io_manager, uint32_t num_spaces = 0);
    void printDEF(IOManager &io_manager);

  private:
    SymbolIndex name_index_;
    std::vector<int> masks_;
    std::vector<Box> rects_;
};

class ViaMaster : public Object {
  public:
    ViaMaster();
    ~ViaMaster();

    SymbolIndex getNameIndex() const;
    std::string const& getName();
    bool setName(std::string const& name);

    std::string const& getPattern();
    std::string const& getCutPatten();

    bool setPattern(std::string const& name);
    bool setCutPatten(std::string const& name);

    ViaRule* getViaRule() const;
    void setViaRule(ViaRule* via_rule);

    Layer* getLowerLayer();
    Layer* getCutLayer();
    Layer* getUpperLayer();

    int getLowerLayerIndex();
    void setLowerLayerIndex(int index);

    int getCutLayerIndex();
    void setCutLayerIndex(int index);

    int getUpperLayerIndex();
    void setUpperLayerIndex(int index);

    int getCutSizeX() const;
    int getCutSizeY() const;
    int getCutSpacingX() const;
    int getCutSpacingY() const;
    int getLowerEncX() const;
    int getLowerEncY() const;
    int getUpperEncX() const;
    int getUpperEncY() const;
    int getRow() const;
    int getCol() const;
    int getOffsetX() const;
    int getOffsetY() const;
    int getLowerOffsetX() const;
    int getLowerOffsetY() const;
    int getUpperOffsetX() const;
    int getUpperOffsetY() const;
    double getResistance() const;
    Box    getBoundingBox();
    Box    getBotBox();
    Box    getTopBox();
    Box    getCutBox();

    bool isDefault() const;
    bool isArray();
    bool isFromDEF();
    bool isFromNDR();
    bool hasOrigin();
    bool hasOffset();
    bool hasCutPatten();
    bool hasGenerated();
    bool hasResistance();
    ObjectId getViaLayerVector();
    uint64_t numProperties() const;
    uint16_t getArrayIndex() const;

    void setCutSizeX(int x);
    void setCutSizeY(int y);
    void setCutSpacingX(int x);
    void setCutSpacingY(int y);
    void setLowerEncX(int x);
    void setLowerEncY(int y);
    void setUpperEncX(int x);
    void setUpperEncY(int y);
    void setRow(int rows);
    void setCol(int col);
    void setOffsetX(int x);
    void setOffsetY(int y);
    void setLowerOffsetX(int x);
    void setLowerOffsetY(int y);
    void setUpperOffsetX(int x);
    void setUpperOffsetY(int y);
    void setResistance(double resistance);
    void setDefault(bool is_default);
    void setIsArray(Bits is_array);
    void setIsFromDEF(Bits is_from_def);
    void setIsFromNDR(Bits is_from_ndr);
    void setHasOrigin(int has_origin);
    void setHasOffset(int has_offset);
    void setArrayIndex(uint16_t array_index);

    void setHasCutPatten(Bits has_cut_pattern);
    void setHasResistance(Bits has_resistance);
    void sethasGenerated(Bits has_generated);

    ViaLayer* creatViaLayer(std::string& name);
    int addViaLayer(ViaLayer* via_layer);
    ArrayObject<ObjectId>* getViaLayerArray();

    void print();
    void printLEF(IOManager &io_manager, uint32_t num_spaces = 0);
    void printDEF(IOManager &io_manager);

  private:
    SymbolIndex name_index_; /**< name */
    SymbolIndex pattern_index_;
    SymbolIndex cut_patten_index_;

    Bits is_default_      :  1;
    Bits has_via_rule_    :  1;
    Bits is_array_        :  1;
    Bits is_from_def_     :  1;
    Bits is_from_ndr_     :  1;
    Bits has_origin_      :  1;
    Bits has_offset_      :  1;
    Bits has_generated_   :  1;
    Bits has_cut_pattern_ :  1;
    Bits has_resistance_  :  1;
    uint16_t array_index_ : 16;

    int x_size_;
    int y_size_;

    ObjectId via_rule_;
    int lower_layer_;
    int cut_layer_;
    int upper_layer_;

    int x_cut_spacing_;
    int y_cut_spacing_;

    int x_lower_enclosure_;
    int y_lower_enclosure_;
    int x_upper_enclosure_;
    int y_upper_enclosure_;

    int rows_;    /**< rows of via array */
    int columns_; /**< colunms of via array*/

    int x_offset_;
    int y_offset_;

    int x_lower_offset_;
    int y_lower_offset_;
    int x_upper_offset_;
    int y_upper_offset_;

    double resistance_; /**< resistance value */
    ObjectId via_layers_;

    Box __calculateBBox(std::vector<Box>& rects);
};

}  // namespace db
}  // namespace open_edi

#endif
