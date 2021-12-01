/* @file  term.h
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef SRC_DB_CORE_TERM_H_
#define SRC_DB_CORE_TERM_H_

#include <algorithm>
#include <string>
#include <utility>

#include "db/core/object.h"
#include "db/util/geometrys.h"
#include "util/io_manager.h"

namespace open_edi {
namespace db {

class Term;

class AntennaArea : public Object {
  public:
    AntennaArea() { layer_idx_ = -1; }
    AntennaArea(int64_t a) : area_(a), layer_idx_(-1) {}
    ~AntennaArea() {}
    void setArea(int64_t a) { area_ = a; }
    int64_t getArea() const { return area_; }
    void setLayerByName(const char* v);
    Layer* getLayer() const;

  private:
    int64_t area_;
    ObjectIndex layer_idx_;
};

class AntennaModelTerm : public Object {
  public:
    AntennaModelTerm();
    ~AntennaModelTerm() {}

    void addAntennaGateArea(ObjectId aa);
    AntennaArea* getAntennaGateArea(int index) const;
    int getAntennaGateAreaNum() const;
    void addAntennaMaxAreaCar(ObjectId aa);
    AntennaArea* getAntennaMaxAreaCar(int index) const;
    int getAntennaMaxAreaCarNum() const;
    void addAntennaMaxSideAreaCar(ObjectId aa);
    AntennaArea* getAntennaMaxSideAreaCar(int index) const;
    int getAntennaMaxSideAreaCarNum() const;
    void addAntennaMaxCutCar(ObjectId aa);
    AntennaArea* getAntennaMaxCutCar(int index) const;
    int getAntennaMaxCutCarNum() const;
    void setValid(bool v) { valid_ = v; }
    bool getValid() const { return valid_; }
    void printLEF(IOManager &io_manager) const;

  private:
    bool valid_;
    ObjectId antenna_gate_areas_;
    ObjectId antenna_max_area_cars_;
    ObjectId antenna_max_side_area_cars_;
    ObjectId antenna_max_cut_cars_;
};

class Port : public Object {
  public:
    Port();
    ~Port();

    void setTermId(ObjectId term_id);
    Term *getTerm() const;

    void setClass(const char* v);
    std::string& getClass() const;
    void addLayerGeometry(ObjectId v);
    LayerGeometry* getLayerGeometry(int index) const;
    int getLayerGeometryNum() const;
    bool getIsReal() const { return is_real_; }
    void setIsReal(bool flag) { is_real_ = flag; }
    bool getHasPlacement() const;
    void setHasPlacement(bool flag);
    PlaceStatus getStatus() const;
    void setStatus(PlaceStatus s);
    Point getLocation() const;
    void setLocation(Point& p);
    Orient getOrient() const;
    void setOrient(Orient o);
    class shapeIter {
        public:
          shapeIter(Port *port) :
              cur_port_(port),
              layer_geom_index_(0),
              geom_index_(-1) {
              cur_layer_geom_ = cur_port_->getLayerGeometry(layer_geom_index_);
          }

          Geometry *getNext() {
              if (layer_geom_index_ == cur_port_->getLayerGeometryNum()) {
                  return nullptr;
              }
              ++geom_index_;
              if (geom_index_ == cur_layer_geom_->getGeometryNum()) {
                  ++layer_geom_index_;
                  if (layer_geom_index_ == cur_port_->getLayerGeometryNum()) {
                      return nullptr;
                  }
                  cur_layer_geom_ = cur_port_->getLayerGeometry(layer_geom_index_);
                  geom_index_ = 0;
              }
              return cur_layer_geom_->getGeometry(geom_index_);
          }
        private:
          Port *cur_port_;
          LayerGeometry *cur_layer_geom_;
          int32_t layer_geom_index_;
          int32_t geom_index_;
    };
  private:
    ObjectId term_id_;
    // lef information
    SymbolIndex class_index_;
    ObjectId layer_geometries_;
    // def information
    PlaceStatus status_;
    Point location_;
    bool is_real_       :1;
    bool has_placement_ :1;
    Orient orient_      :8;
    int reserved_       :22;
};

enum AccessType {
    kOnAccessGrid = 1,
    kOnPinGrid    = 2,
    kInBound      = 4,
    kInEolBound   = 8,
    kSubsumed     = 16
};

enum AccessStyle {
    kWirePinAccess = 1,
    kViaPinAccess  = 2
};

constexpr int kMaxOrientationNum = 8;
constexpr int kMaxSetNum         = 16;


/// @brief stores pin access info on cell terms
class PinAccess : public Object {
  public:
    PinAccess();
    ~PinAccess();

    // get APIs
    Point         getPosition();
    Box           getBoundingBox(); // valid for via-access only, need to transfer using pos_ & orient_
    Box           getBotBox();      // valid for via-access only, need to transfer using pos_ & orient_
    Box           getTopBox();      // valid for via-access only, need to transfer using pos_ & orient_
    Box           getPatchBox();    // already transferred, just return patch box directly
    Cell*         getCell();        // implicitly get cell pointer via term
    Term*         getTerm();        // return term pointer
    Layer*        getLayer();       // return layer pointer from layer_no_
    ViaMaster*    getViaMaster();   // return via master pointer
    AccessStyle   getAccessStyle(); // AccStyle should be defined enum type
    AccessType    getAccessType();  // AccType should be defined enum type
    unsigned char getSet();
    Orient        getOrient();
    bool          getIsOnAccessGrid();
    bool          getIsOnPinGrid();
    bool          getIsNearBoundary();
    bool          getIsEolBoundary();
    bool          getIsSubsumed();
    bool          getIsViaPinAccess();
    bool          getIsWirePinAccess();
    bool          getIsPreferred();

    // set APIs
    void setIsOnAccessGrid(bool v);
    void setIsOnPinGrid(bool v);
    void setIsNearBoundary(bool v);
    void setIsEolBoundary(bool v);
    void setIsSubsumed(bool v);
    void setIsViaPinAccess(bool v);
    void setIsWirePinAccess(bool v);
    void setIsPreferred(bool v);

    void setOrient(Orient o);
    void setAccessStyle(AccessStyle s);
    void setAccessType(AccessType t);
    void setTerm(Term* term);
    void setPatchBox(Box& box);
    void setPosition(Point& pos);
    void setViaMaster(ViaMaster* vm);
    void setLayer(Layer* l);
    void setSet(unsigned char s);

  private:
    Point pos_; // via boundary box center, for via-access only

    ObjectId via_master_; // via master, via-access only
    Box      patch_;      // patch box for min-area rule for via-access or metal for wire-access

    ObjectId term_; // the term it blongs to

    uint8_t    layer_no_;
    AccessType access_type_ : 8; // access-grid, pin-grid, bound, eol-bound & subsumed

    Bits8       orient_ : 4;       // instance orient
    AccessStyle access_style_ : 2; // via-pin-acess or wire-pin-access
    Bits8       null_ : 2;

    Bits8 set_ : 5;
    Bits8 is_preferred_ : 1;
    Bits8 null1_ : 2;

    Box   __calculateBBox(Box& rect);
    Point __tansformPoint(Point& placed_pt);
    Box   __tansformBox(Box& rect);
};

/// Yibo: I need this for InstTerm, so this is a temporary implementation.
/// I do not think Term needs to be an AttrObject, as it is frequently accessed.
/// We can directly put all the data members in its data field.

/// @brief Terminal of a cell
class Term : public Object {
  public:
    using BaseType = Object;

    /// @brief default constructor
    Term();

    /// @brief constructor
    Term(Object* owner, IndexType id);

    /// @brief copy constructor
    Term(Term const& rhs);

    /// @brief move constructor
    Term(Term&& rhs) noexcept;

    ~Term();

    /// @brief copy assignment
    Term& operator=(Term const& rhs);

    /// @brief move assignment
    Term& operator=(Term&& rhs) noexcept;

    void setName(std::string name);
    std::string& getName() const;

    SymbolIndex getNameIndex() { return name_index_; }

    /// @brief summarize memory usage of the object in bytes
    IndexType memory() const;

    void setHasRange(bool has_range) { has_range_ = has_range; }
    bool getHasRange() { return has_range_; }

    void setIsWriteVerilog(bool is_write_verilog) {
        is_write_verilog_ = is_write_verilog;
    }
    bool getIsWriteVerilog() { return is_write_verilog_; }

    void setIsWriteDef(bool is_write_def) {
        is_write_def_ = is_write_def;
    }
    bool getIsWriteDef() { return is_write_def_; }

    void setRangeLow(int32_t range_low) { range_low_ = range_low; }
    int32_t getRangeLow() { return range_low_; }

    void setRangeHigh(int32_t range_high) { range_high_ = range_high; }
    int32_t getRangeHigh() { return range_high_; }

    std::string const& getTaperRule() const;
    void setTaperRule(const char* v);

    //LEF: PIN DIRECTION
    bool hasDirection() const;
    bool isInput(); // DIRECTION INPUT
    bool isOutput(); // DIRECTION OUTPUT
    bool isInOut(); // DIRECTION INOUT
    bool isFeedthru(); // DIRECTION FEEDTHRU
    void setDirection(const char* v);
    void setDirection(SignalDirection v);

    //LEF: PIN USE
    bool hasUse() const;
    void setUse(const char* v);

    SignalDirection getDirection() const;
    std::string getDirectionStr() const;
    void setDirectionStr(const char* v);
    
    bool isPGType() const;
    SignalType getType() const;
    std::string getTypeStr() const;
    void setTypeStr(const char* v);
    void setType(SignalType v);

    //SymbolIndex getNetExprIndex() const;
    std::string getNetExpr() const;
    void setNetExpr(const char* v);
    SymbolIndex getSupplySensitivityIndex() const;
    std::string const& getSupplySensitivity() const;
    void setSupplySensitivity(const char* v);
    SymbolIndex getGroundSensitivityIndex() const;
    std::string const& getGroundSensitivity() const;
    void setGroundSensitivity(const char* v);
    std::string const& getMustjoin() const;
    void setMustjoin(const char* v);
    std::string const& getShape() const;
    void setShape(const char* v);
    void addAntennaPartialMetalArea(ObjectId aa);
    AntennaArea* getAntennaPartialMetalArea(int index) const;
    int getAntennaPartialMetalAreaNum() const;
    void addAntennaPartialMetalSideArea(ObjectId aa);
    AntennaArea* getAntennaPartialMetalSideArea(int index) const;
    int getAntennaPartialMetalSideAreaNum() const;
    void addAntennaPartialCutArea(ObjectId aa);
    AntennaArea* getAntennaPartialCutArea(int index) const;
    int getAntennaPartialCutAreaNum() const;
    void addAntennaDiffArea(ObjectId aa);
    AntennaArea* getAntennaDiffArea(int index) const;
    int getAntennaDiffAreaNum() const;
    void addAntennaModelTerm(int index, AntennaModelTerm* am);
    const AntennaModelTerm* GetAntennaModelTerm(int index) const {
        return &antenna_models_[index];
    }
    void addPort(ObjectId p);
    Port* getPort(int index) const;
    int getPortNum() const;

    void setCellId(ObjectId cell_id);
    ObjectId getCellId();
    Cell* getCell() const;

    void printLEF(IOManager &io_manager) const;

    std::vector<PinAccess*>* getAccesses(const Orient ori, const unsigned char set);

  protected:
    /// @brief copy object
    void copy(Term const& rhs);
    /// @brief move object
    void move(Term&& rhs);
    /// @brief overload output stream
    friend OStreamBase& operator<<(OStreamBase& os, Term const& rhs);
    /// @brief overload input stream
    friend IStreamBase& operator>>(IStreamBase& is, Term& rhs);

  private:
    SymbolIndex name_index_;
    
    SignalDirection direction_ :8;
    SignalType type_           :8;
    bool has_range_            :1;
    bool is_write_verilog_     :1;
    bool is_write_def_         :1;
    int32_t reserved_          :13;
    int32_t range_low_;
    int32_t range_high_;

    //  SymbolIndex name_index_; ///< terminal name
    SymbolIndex taper_rule_index_;
    SymbolIndex supply_sensitivity_index_;
    SymbolIndex ground_sensitivity_index_;
    SymbolIndex mustjoin_index_;
    SymbolIndex shape_index_;
    ObjectId antenna_partial_metal_areas_;
    ObjectId antenna_partial_metal_side_areas_;
    ObjectId antenna_partial_cut_areas_;
    ObjectId antenna_diff_areas_;
    AntennaModelTerm antenna_models_[6];
    ObjectId ports_;
    ObjectId cell_id_; // the macro or module which has the term.
    std::vector<PinAccess*> accesses_[kMaxOrientationNum][kMaxSetNum];
};


}  // namespace db
}  // namespace open_edi

#endif  //  SRC_DB_CORE_TERM_H_
