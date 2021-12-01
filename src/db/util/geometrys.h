/* @file  geometry.h
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#ifndef SRC_UTIL_GEOMETRYS_H_
#define SRC_UTIL_GEOMETRYS_H_
#include  <string>
#include "util/util.h"
#include "util/point.h"
#include "db/util/box.h"
#include "util/polygon_table.h"
#include "db/util/symbol_table.h"
#include "db/core/object.h"
#include "db/tech/via_master.h"
#include "db/tech/layer.h"
#include "util/io_manager.h"

namespace open_edi {
namespace db {
const int32_t kInvalidLengthValue = -1;
class Transform;

enum GeometryType : int8_t{
    kNoGeomType = 0,
    kPath = 1,
    kRect = 2,
    kPolygon = 3,
    kPoint = 4,
};

class MaskNumber {
  public:
    MaskNumber() : top_(0), cut_(0), bottom_(0) {}
    MaskNumber(int value) : top_(0), cut_(0) { setBottomMaskNum(value); }
    MaskNumber(int top, int cut, int bottom);
    ~MaskNumber() {}

    // for via layer, three mask numbers:
    void setTopMaskNum(int value);
    int getTopMaskNum() const;
    void setCutMaskNum(int value);
    int getCutMaskNum() const;
    void setBottomMaskNum(int value);
    int getBottomMaskNum() const;
    // for one layer, only one mask number.
    void setMaskNum(int value) { setBottomMaskNum(value); }
    int getMaskNum() const { return getBottomMaskNum(); }
  private:
    Bits top_: 4; //using 4bits to indicate one mask_number
    Bits cut_: 4; //using 4bits to indicate one mask_number
    Bits bottom_: 4; //using 4bits to indicate one mask_number
};

class Geometry: public Object{
 public:
    Geometry();
    ~Geometry();
    GeometryType getType() const { return type_; }
    bool isPath() const { return getType() == kPath; }
    bool isRect() const { return getType() == kRect; }
    bool isPolygon() const { return getType() == kPolygon; }
    bool isPoint() const { return getType() == kPoint; }

    // for via layer, three mask numbers:
    void setTopMaskNum(int value);
    int getTopMaskNum() const;
    void setCutMaskNum(int value);
    int getCutMaskNum() const;
    void setBottomMaskNum(int value);
    int getBottomMaskNum() const;
    // for one layer, only one mask number.
    void setMaskNum(int value) { setBottomMaskNum(value); }
    int getMaskNum() const { return getBottomMaskNum(); }

    void setPath(Polygon *polygon);
    ObjectIndex getPathId() const { return polygon_id_; }
    Polygon* getPath() const;
    void setPolygon(Polygon *polygon);
    ObjectIndex getPolygonId() const { return polygon_id_; }
    Polygon* getPolygon() const;
    void setBox(Box* b);
    const Box& getBox() const;
    void setPoint(Point* value);
    const Point& getPoint() const;

 private:
    PolygonTable* getPolygonTable() const;

    void __setType(GeometryType t) { type_ = t; }
    ObjectIndex __generatePolygonIndex(Polygon* p);
    union {
        Point pt_;
        Box box_;
        ObjectIndex polygon_id_; //either polygon or path points.
    };
    GeometryType type_;
    MaskNumber mask_numbers_;
};

#if 0
class GeometryVia: public Object{
 public:
    GeometryVia() {}
    ~GeometryVia() {}
    SymbolTable* getSymbolTable();
    SymbolIndex getOrCreateSymbol(const char *name);
    void setViaMaster(const char *value);
    ViaMaster* getViaMaster();
    void setPoint(Point value) { p_ = value; }
    Point getPoint() const { return p_; }
    void setTopMaskNum(int value) { top_mask_num_ = value; }
    int getTopMaskNum() const { return top_mask_num_; }
    void setCutMaskNum(int value) { cut_mask_num_ = value; }
    int getCutMaskNum() const { return cut_mask_num_; }
    void setBottomMaskNum(int value) { bottom_mask_num_ = value; }
    int getBottomMaskNum() const { return bottom_mask_num_; }

 private:
    ObjectId via_master_index_;
    Point p_;
    int top_mask_num_;
    int cut_mask_num_;
    int bottom_mask_num_;
};
#endif

class LayerGeometry: public Object{
 public:
    LayerGeometry();
    ~LayerGeometry();
    //GeometryType getType() const { return type_; }
    //void setType(GeometryType t) { type_ = t; }
    void addGeometry(ObjectId id);
    int getGeometryNum() const;
    int getVecNum() const;
    Geometry* getGeometry(int i) const;

    void setLayer(const char *value);
    Layer* getLayer();
    void setViaMaster(const char *value);
    ViaMaster* getViaMaster();
    bool isViaLayer() const {
        return (is_via_ && layer_via_id_ != kInvalidObjectId);
    }
    bool isLayer() const {
        return (!is_via_ && layer_via_id_ != kInvalidLayerIndex);
    }

    // for via layer, three mask numbers:
    void setTopMaskNum(int value);
    int getTopMaskNum() const;
    void setCutMaskNum(int value);
    int getCutMaskNum() const;
    void setBottomMaskNum(int value);
    int getBottomMaskNum() const;
    // for one layer, only one mask number.
    void setMaskNum(int value) { setBottomMaskNum(value); }
    int getMaskNum() const { return getBottomMaskNum(); }

    void setExceptPgNet(bool value = true) { is_except_pgnet_ = value; }
    bool isExceptPgNet() const { return is_except_pgnet_; }

    void setMinSpacing(int value);
    int getMinSpacing() const;
    void setDesignRuleWidth(int value);
    int getDesignRuleWidth() const;
    void setWidth(int value);
    int getWidth() const;
    bool hasSpacing() const { return has_spacing_; }
    bool hasDRW() const { return has_drw_; }
    bool hasWidth() const { return has_width_; }

    void printLEF(IOManager &io_manager, bool from_port = false);
    void printDEF(IOManager &io_manager, uint32_t space_count);
    void printDEF(IOManager &io_manager, uint32_t space_count, Transform &transform);

    class BoxIter {
        public:
          BoxIter(LayerGeometry *layer_geom, int index) : 
              cur_layer_geom_(layer_geom),
              geom_index_(index - 1) {
          }

          Box *getNext() {
              if (cur_layer_geom_ == nullptr) return nullptr;
              while (++geom_index_ < cur_layer_geom_->getGeometryNum()) {
                  Geometry *geom = cur_layer_geom_->getGeometry(geom_index_);
                  ediAssert(geom != nullptr);
                  if (geom->isRect()) {
                      return const_cast<Box *>(&(geom->getBox()));
                  }
              }
              return nullptr;
          }
        private:
          LayerGeometry *cur_layer_geom_;
          int32_t geom_index_;
    };

    BoxIter getBoxIter() { return BoxIter(this, 0); }

    class PolygonIter {
        public:
          PolygonIter(LayerGeometry *layer_geom, int index) :
              cur_layer_geom_(layer_geom),
              geom_index_(index - 1) {
          }

          Polygon *getNext() {
              if (cur_layer_geom_ == nullptr) return nullptr;
              while (++geom_index_ < cur_layer_geom_->getGeometryNum()) {
                  Geometry *geom = cur_layer_geom_->getGeometry(geom_index_);
                  ediAssert(geom != nullptr);
                  if (geom->isPolygon()) {
                      return geom->getPolygon();
                  }
              }
              return nullptr;
          }
        private:
          LayerGeometry *cur_layer_geom_;
          int32_t geom_index_;
    };

    PolygonIter getPolygonIter() { return PolygonIter(this, 0); }
 private:
    ObjectId geometries_;
    int64_t layer_via_id_; // either layer index or via master id
    int32_t spacing_; // either min_spacing or design rule width.
    int32_t width_; // width.
    MaskNumber mask_numbers_;
    bool has_spacing_ : 1;
    bool has_drw_ : 1;
    bool has_width_ : 1;
    bool is_except_pgnet_ : 1;
    bool is_via_ : 1;
    bool is_geometry_array_ : 1;
    // Bits from_def_ : 1;
    // Bits from_lef_ : 1;
};

}  // namespace db
}  // namespace open_edi

#endif  // SRC_UTIL_GEOMETRYS_H_
