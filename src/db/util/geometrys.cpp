/* @file  geometry.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "db/util/geometrys.h"
#include "db/util/array.h"
#include "db/core/db.h"
#include "db/core/root.h"
#include "db/util/transform.h"

namespace open_edi {
namespace db {
using IdArray = ArrayObject<ObjectId>;

MaskNumber::MaskNumber(int top, int cut, int bottom) {
    setTopMaskNum(top);
    setCutMaskNum(cut);
    setBottomMaskNum(bottom);
}

void MaskNumber::setTopMaskNum(int value) {
    top_ = value & 0xf;
}

int MaskNumber::getTopMaskNum() const {
    int value = (top_ & 0xf);
    return value;  
}

void MaskNumber::setCutMaskNum(int value) {
    cut_ = value & 0xf;
}

int MaskNumber::getCutMaskNum() const {
    int value = (cut_ & 0xf);
    return value;  
}

void MaskNumber::setBottomMaskNum(int value) {
    bottom_ = value & 0xf;
}

int MaskNumber::getBottomMaskNum() const {
    int value = (bottom_ & 0xf);
    return value;
}

Geometry::Geometry() : box_(0, 0, 0, 0),
    type_(GeometryType::kNoGeomType), mask_numbers_(0) {
}

Geometry::~Geometry() {
}

PolygonTable* Geometry::getPolygonTable() const {
    StorageUtil *owner_util = Object::getStorageUtilById(this->getOwnerId());
    ediAssert(owner_util != nullptr);
    return owner_util->getPolygonTable();
}

ObjectIndex Geometry::__generatePolygonIndex(Polygon* p) {
    PolygonTable* t = getPolygonTable();
    if (t) {
        return t->addPolygon(p);
    }
    return kInvalidPolygonTableIndex;
}

void Geometry::setPolygon(Polygon *polygon) {
    ediAssert(polygon != nullptr);
    ObjectIndex index = __generatePolygonIndex(polygon);
    if (index != kInvalidPolygonTableIndex) {
        polygon_id_ = index;
        __setType(GeometryType::kPolygon);
    }
}

Polygon* Geometry::getPolygon() const {
    if (isPolygon() && polygon_id_ != kInvalidPolygonTableIndex) {
        Polygon *p = getPolygonTable()->getPolygonByIndex(polygon_id_);
        return p;
    }
    return nullptr;
}

void Geometry::setPath(Polygon *polygon) {
    ediAssert(polygon != nullptr);
    ObjectIndex index = __generatePolygonIndex(polygon);
    if (index != kInvalidPolygonTableIndex) {
        polygon_id_ = index;
        __setType(GeometryType::kPath);
    }
}

Polygon* Geometry::getPath() const {
    if (isPath() && polygon_id_ != kInvalidPolygonTableIndex) {
        Polygon *p = getPolygonTable()->getPolygonByIndex(polygon_id_);
        return p;
    }
    return nullptr;
}

void Geometry::setBox(Box* box) {
    ediAssert(box != nullptr);
    box_.setBox(box);
    __setType(GeometryType::kRect);
}

const Box& Geometry::getBox() const {
    return box_;
}

void Geometry::setPoint(Point* pt) {
    ediAssert(pt != nullptr);
    pt_.setX(pt->getX());
    pt_.setY(pt->getY());
    __setType(GeometryType::kPoint);
}

const Point& Geometry::getPoint() const {
    return pt_; 
}

void Geometry::setTopMaskNum(int value) {
    mask_numbers_.setTopMaskNum(value);
}

int Geometry::getTopMaskNum() const {
    return mask_numbers_.getTopMaskNum();
}

void Geometry::setCutMaskNum(int value) {
    mask_numbers_.setCutMaskNum(value);

}

int Geometry::getCutMaskNum() const {
    return mask_numbers_.getCutMaskNum();
}

void Geometry::setBottomMaskNum(int value) {
    mask_numbers_.setBottomMaskNum(value);
}

int Geometry::getBottomMaskNum() const {
    return mask_numbers_.getBottomMaskNum();
}

LayerGeometry::LayerGeometry() : geometries_(0), 
    layer_via_id_(kInvalidLayerIndex),
    spacing_(kInvalidLengthValue), width_(kInvalidLengthValue),
    has_spacing_(0), has_drw_(0), has_width_(0),
    is_except_pgnet_(0), is_via_(0), is_geometry_array_(0) {
}

LayerGeometry::~LayerGeometry() {
}

void LayerGeometry::setLayer(const char *value) {
    Tech *lib = getTopCell()->getTechLib();
    ediAssert(lib != nullptr);
    ObjectIndex index = lib->getLayerLEFIndexByName(value);
    if (index == kInvalidLayerIndex) {
        message->issueMsg("DB", 28, kError, value);
        return;
    }
    // TODO: to check this:
    if (index > 255) {
        message->issueMsg("DB", 29, kError, index);
        return;
    }
    layer_via_id_ = index;
    is_via_ = 0;
}

Layer* LayerGeometry::getLayer() {
    if (!isLayer()) {
        return nullptr;
    }
    Tech *lib = getTopCell()->getTechLib();
    ediAssert(lib != nullptr);
    return lib->getLayer(layer_via_id_);
}

void LayerGeometry::setViaMaster(const char *value) {
    Tech *lib = getTopCell()->getTechLib();
    ViaMaster* vm = lib->getViaMaster(value);
    if (vm == nullptr || vm->getId() == 0) {
        message->issueMsg("DB", 30, kError, value);
        return;
    }
    layer_via_id_ = vm->getId();  
    is_via_ = 1;
}

ViaMaster* LayerGeometry::getViaMaster() {
    if (!is_via_ || !layer_via_id_) {
        return nullptr;
    }
    return addr<ViaMaster>(layer_via_id_);
}

void LayerGeometry::addGeometry(ObjectId id) {
    ediAssert(id != 0);
    
    if (geometries_ == 0) {
        // if the inserted Geometry is the first one:
        geometries_ = id;
        is_geometry_array_ = 0;
        return;
    }
    IdArray *vct = nullptr;
    if (is_geometry_array_ == 0) {
        // if there is only one Geometry in the LayerGeometry,
        // and we're inserting another one Geometry:
        ObjectId orig_geomery_id = geometries_;
        geometries_ = __createObjectIdArray(16);
        ediAssert(geometries_ != 0);
        vct = addr<IdArray>(geometries_);
        ediAssert(vct != nullptr);
        is_geometry_array_ = 1;
        vct->pushBack(orig_geomery_id);
    } else {
        vct = addr<IdArray>(geometries_);
        ediAssert(vct != nullptr);
    }
    vct->pushBack(id);
}

Geometry* LayerGeometry::getGeometry(int i) const {
    if (geometries_ == 0) {
        return nullptr;
    } else if (!is_geometry_array_) {
        if (i > 0) {
            return nullptr;
        }
        return addr<Geometry>(geometries_);
    }
    
    IdArray *vct = addr<IdArray>(geometries_);
    ediAssert(vct != nullptr);
    if (i >= vct->getSize()) {
        return nullptr;
    }
    return addr<Geometry>((*vct)[i]);
}

int LayerGeometry::getVecNum() const {
    return getGeometryNum();
}

int LayerGeometry::getGeometryNum() const {
    if (geometries_ == 0) {
        return 0;
    }
    if (!is_geometry_array_) {
        return 1;
    }
    IdArray *vct = addr<IdArray>(geometries_);
    ediAssert(vct != nullptr);
    return vct->getSize();
}

void LayerGeometry::setMinSpacing(int value) {
    ediAssert(has_drw_ == 0);
    spacing_ = value;
    has_spacing_ = 1;
}

int LayerGeometry::getMinSpacing() const {
    if (has_spacing_) {
        return spacing_;
    }
    return kInvalidLengthValue;
}

void LayerGeometry::setDesignRuleWidth(int value) {
    ediAssert(has_spacing_ == 0);
    spacing_ = value;
    has_drw_ = 1;
}

int LayerGeometry::getDesignRuleWidth() const {
    if (has_drw_) {
        return spacing_;
    }
    return kInvalidLengthValue;
}

void LayerGeometry::setWidth(int value) {
    width_ = value;
    has_width_ = 1;
}

int LayerGeometry::getWidth() const {
    if (has_width_) {
        return width_;
    }
    return kInvalidLengthValue;
}

void LayerGeometry::setTopMaskNum(int value) {
    mask_numbers_.setTopMaskNum(value);
}

int LayerGeometry::getTopMaskNum() const {
    return mask_numbers_.getTopMaskNum();
}

void LayerGeometry::setCutMaskNum(int value) {
    mask_numbers_.setCutMaskNum(value);
}

int LayerGeometry::getCutMaskNum() const {
    return mask_numbers_.getCutMaskNum();
}

void LayerGeometry::setBottomMaskNum(int value) {
    mask_numbers_.setBottomMaskNum(value);
}

int LayerGeometry::getBottomMaskNum() const {
    return mask_numbers_.getBottomMaskNum();
}

void LayerGeometry::printDEF(IOManager &io_manager, uint32_t space_count) {
    Transform transform;
    printDEF(io_manager, space_count, transform);
}

void LayerGeometry::printDEF(IOManager &io_manager, 
    uint32_t space_count, Transform &transform
) {
    if (getGeometryNum() <= 0) {
        return;
    }
    Tech *lib = getTechLib();
    ediAssert(lib != nullptr);
    std::string space_str(space_count, ' ');
    for (int k = 0; k < getGeometryNum(); k++) {
        Geometry *geo = getGeometry(k);
        if (isViaLayer()) {
            io_manager.write("%s+ VIA %s", space_str.c_str(),
                getViaMaster()->getName().c_str());
            if (geo->getTopMaskNum() ||
              geo->getCutMaskNum() ||
              geo->getBottomMaskNum()) {
              if (geo->getTopMaskNum()) {
                io_manager.write(" MASK %d%d%d",
                  geo->getTopMaskNum(),
                  geo->getCutMaskNum(),
                  geo->getBottomMaskNum());
              } else if (geo->getCutMaskNum()) {
                io_manager.write(" MASK %d%d",
                  geo->getCutMaskNum(),
                  geo->getBottomMaskNum());
              } else {
                io_manager.write(" MASK %d",
                  geo->getBottomMaskNum());
              }
          }
          Point pt = geo->getPoint();
          transform.reverseTransform(pt);
          io_manager.write(" ( %d %d )\n",
            pt.getX(), pt.getY());
        } else {
            if (!getLayer()) {
                continue;
            }
            std::string prefix_str(" LAYER ");
            if (geo->getType() == GeometryType::kPolygon) {
                prefix_str = " POLYGON ";
            }
            std::string str_Layer_Name = getLayer()->getName();
            io_manager.write("%s+%s%s", space_str.c_str(),
                prefix_str.c_str(), str_Layer_Name.c_str());
            if (geo->getMaskNum() > 0) {
                io_manager.write(" MASK %d", geo->getMaskNum());
            }
            if (getMinSpacing() >= 0) {
                io_manager.write(" SPACING %d", getMinSpacing());
            }
            if (getDesignRuleWidth() >= 0) {
                io_manager.write(" DESIGNRULEWIDTH %d",
                    getDesignRuleWidth());
            }
            Box box;
            Polygon *po;
            switch (geo->getType()) {
            case GeometryType::kRect:
                box = geo->getBox();
                transform.reverseTransform(box);
                io_manager.write(" ( %d %d ) ( %d %d )\n",
                    box.getLLX(), box.getLLY(),
                    box.getURX(), box.getURY());
                break;
            case GeometryType::kPolygon:
                po = geo->getPolygon();
                for (int x = 0; x < po->getNumPoints(); ++x) {
                    Point pt = po->getPoint(x);
                    transform.reverseTransform(pt);
                    io_manager.write(" ( %d %d )",
                        pt.getX(), pt.getY());
                }
                io_manager.write("\n");
                break;
            }
        }
    }
}

void LayerGeometry::printLEF(IOManager &io_manager, bool from_port) {
    Tech *lib = getTechLib();

	  if (!isViaLayer()) {
        if (getLayer() == nullptr)
            return;
        if (from_port)
            io_manager.write("   ");
        io_manager.write("      LAYER %s", getLayer()->getName());
        if (this->isExceptPgNet())
            io_manager.write(" EXCEPTPGNET");
        if (this->hasSpacing())
            io_manager.write(" SPACING %g",
                lib->dbuToMicrons(this->getMinSpacing()));
        if (this->hasDRW())
            io_manager.write(" DESIGNRULEWIDTH %g",
                lib->dbuToMicrons(this->getDesignRuleWidth()));
        io_manager.write(" ;\n");
        if (this->getWidth() > 0) {
          if (from_port)
            io_manager.write("   ");
          io_manager.write("         WIDTH %g ;\n",
            lib->dbuToMicrons(this->getWidth()));
        }
    } else {
        if (getViaMaster() == nullptr)
            return;
    }
    if (this->getGeometryNum() > 0) {
        for (int i = 0; i < this->getGeometryNum(); i++) {
            Geometry *geo = this->getGeometry(i);
            if (!geo) continue;

            if (isViaLayer()) {
                if (from_port)
                    io_manager.write("   ");
                if (geo->getTopMaskNum() != 0 || geo->getCutMaskNum() != 0 || geo->getBottomMaskNum() != 0) {
                    io_manager.write("      VIA MASK %d%d%d %g %g %s ;\n",
                        geo->getTopMaskNum(), geo->getCutMaskNum(), geo->getBottomMaskNum(),
                        lib->dbuToMicrons(geo->getPoint().getX()),
                        lib->dbuToMicrons(geo->getPoint().getY()),
                        getViaMaster()->getName().c_str());
                } else {
                    io_manager.write("      VIA %g %g %s ;\n",
                        lib->dbuToMicrons(geo->getPoint().getX()),
                        lib->dbuToMicrons(geo->getPoint().getY()),
                        getViaMaster()->getName().c_str());
                }
            } else {
                if (from_port)
                    io_manager.write("   ");
                switch (geo->getType()) {
                case GeometryType::kPath:
                {
                    Polygon *p = geo->getPath();
                    io_manager.write("         PATH");
                    if (geo->getMaskNum() != 0)
                        io_manager.write(" MASK %d", geo->getMaskNum());
                    for (int x = 0; x < p->getNumPoints(); x++) {
                        io_manager.write(" %g %g",
                            lib->dbuToMicrons(p->getPoint(x).getX()),
                            lib->dbuToMicrons(p->getPoint(x).getY()));
                    }
                    io_manager.write(" ;\n");
                    break;
                }
                case GeometryType::kRect:
                {
                    io_manager.write("         RECT");
                    if (geo->getMaskNum() != 0)
                        io_manager.write(" MASK %d", geo->getMaskNum());
                    io_manager.write(" %g %g %g %g ;\n",
                        lib->dbuToMicrons(geo->getBox().getLLX()),
                        lib->dbuToMicrons(geo->getBox().getLLY()),
                        lib->dbuToMicrons(geo->getBox().getURX()),
                        lib->dbuToMicrons(geo->getBox().getURY()));
                    break;
                }
                case GeometryType::kPolygon:
                {
                    Polygon *p = geo->getPolygon();
                    io_manager.write("         POLYGON");
                    if (geo->getMaskNum() != 0)
                        io_manager.write(" MASK %d", geo->getMaskNum());
                    for (int x = 0; x < p->getNumPoints(); x++)
                        io_manager.write(" %g %g",
                            lib->dbuToMicrons(p->getPoint(x).getX()),
                            lib->dbuToMicrons(p->getPoint(x).getY()));
                    io_manager.write(" ;\n");
                    break;
                }
                }
            }
        }
    }
}

}  // namespace db
}  // namespace open_edi
