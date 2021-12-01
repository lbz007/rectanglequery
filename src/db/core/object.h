/* @file  object.h
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef EDI_DB_OBJECT_H_
#define EDI_DB_OBJECT_H_

#include <vector>

#include "util/util.h"

namespace open_edi {
namespace db {

using namespace open_edi::util;

/// @brief object type enum definition
typedef enum ObjectType {
    kObjectTypeNone = 0,
    kObjectTypeCell,
    kObjectTypeHierData,
    kObjectTypeFloorplan,
    kObjectTypeCellSitePattern,
    kObjectTypeForeign,
    kObjectTypeDensity,
    kObjectTypeDensityLayer,
    kObjectTypeTerm,
    kObjectTypeBus,
    kObjectTypePort,
    kObjectTypeInst,
    kObjectTypePin,
    kObjectTypePinAntennaArea,
    kObjectTypeNet,
    kObjectTypeSpecialNet,
    kObjectTypeWire,
    kObjectTypeSpecialWire,
    kObjectTypeVia,
    kObjectTypeTech,
    kObjectTypeUnits,
    kObjectTypeLayer,
    kObjectTypeLayerMinArea,
    kObjectTypeViaMaster,
    kObjectTypeViaRule,
    kObjectTypeNonDefaultRule,
    kObjectTypeSite,
    kObjectTypeRow,
    kObjectTypeTrack,
    kObjectTypeGcellGrid,
    kObjectTypeFill,
    kObjectTypeScanChain,
    kObjectTypeRegion,
    kObjectTypePhysicalConstraint,
    kObjectTypeGrid,
    kObjectTypeShape,
    kObjectTypeLayerGeometry,
    kObjectTypeGeometry,
    kObjectTypeMarker,
    kObjectTypeGroup,
    // Timing related
    kObjectTypeTiming,
    kObjectTypeClock,
    kObjectTypeAnalysisView,
    kObjectTypeAnalysisCorner,
    kObjectTypeAnalysisMode,
    kObjectTypeTLib,
    kObjectTypeTCell,
    kObjectTypeTTerm,
    kObjectTypeTPgTerm,
    kObjectTypeArc,
    kObjectTypeDesign,
    kObjectTypeLibSet,
    kObjectTypeOperatingConditions,
    kObjectTypeTUnits,
    kObjectTypeTPvt,
    kObjectTypeWireLoadTable,
    kObjectTypeWireLoad,
    kObjectTypeWireLoadForArea,
    kObjectTypeWireLoadSelection,
    kObjectTypeTableAxis,
    kObjectTypeTableTemplate,
    kObjectTypeTimingTable,
    kObjectTypeTimingTable0,
    kObjectTypeTimingTable1,
    kObjectTypeTimingTable2,
    kObjectTypeTimingTable3,
    kObjectTypeScaleFactors,
    kObjectTypeTFunction,
    kObjectTypeTimingArc,
    kObjectTypeTimingArcData,
    kObjectTypeDesignParasitics,
    kObjectTypeNetsParasitics,
    kObjectTypeNetParasitics,
    kObjectTypeDNetParasitics,
    kObjectTypeRNetParasitics,
    kObjectTypeParasiticNode,
    kObjectTypeParasiticIntNode,
    kObjectTypeParasiticPinNode,
    kObjectTypeParasiticExtNode,
    kObjectTypeParasiticDevice,
    kObjectTypeParasiticResistor,
    kObjectTypeParasiticXCap,
    kObjectTypeParasiticCap,
    kObjectTypeTimingSequential,
    // Misc
    kObjectTypePropertyDefinition,
    kObjectTypeProperty,
    // for ndr rule...
    kObjectTypeNonDefaultRuleLayer,
    kObjectTypeNonDefaultRuleMinCuts,
    // for layer rule
    kObjectTypeCutLayerRule,
    kObjectTypeCutSpacing,
    kObjectTypeSecondLayer,
    kObjectTypeAdjacentCuts,
    kObjectTypeCutSpacingPrlOvlp,
    kObjectTypeEnclosure,
    kObjectTypeEnclosureEol,
    kObjectTypeEnclosureOverhang,
    kObjectTypeArraySpacing,
    kObjectTypeSpTblOrthogonal,
    kObjectTypeBoundaryEOLBlockage,
    kObjectTypeCornerEOLKeepout,
    kObjectTypeCornerFillSpacing,
    kObjectTypeCornerSpacing,
    kObjectTypeDirSpanLengthSpTbl,
    kObjectTypeSpanLength,
    kObjectTypeExactSLSpacing,
    kObjectTypeEOLKeepout,
    kObjectTypeMinCut,
    kObjectTypeMinEnclArea,
    kObjectTypeMinSize,
    kObjectTypeMinStep,
    kObjectTypeWidthSpTbl,
    kObjectTypeInfluenceSpTbl,
    kObjectTypeParaSpanLenTbl,
    kObjectTypeProtrusionRule,
    kObjectTypeProtrusionWidth,
    kObjectTypeRoutingSpacing,
    kObjectTypeRoutingLayerRule,
    kObjectTypeTrimLayerRule,
    kObjectTypeMEOLLayerRule,
    kObjectTypeAntennaModel,
    kObjectTypeMinArea,
    kObjectTypeCurrentDen,
    kObjectTypeCurrentDenContainer,
    kObjectTypeImplantCoreEdgeLength,
    kObjectTypeImplantSpacing,
    kObjectTypeImplantWidth,
    kObjectTypeImplantLayerRule,
    // for site
    kObjectTypeSitePatternPair,
    kObjectTypeArray,
    kObjectTypeArraySegment,
    kObjectTypeMaxViaStack,
    kObjectTypeAntennaModelTerm,
    // for property definition
    kObjectTypeLibrary,
    kObjectTypeMacro,
    kObjectTypeComponent,
    kObjectTypeComponentPin,
    kObjectTypeFHierData,

    kObjectTypeMax
} ObjectType;

inline std::string toString(ObjectType const &v) {
    switch (v) {
        case ObjectType::kObjectTypeCell:
            return "DESIGN";
        case ObjectType::kObjectTypeInst:
            return "INST";
        case ObjectType::kObjectTypeTerm:
            return "PORT";
        case ObjectType::kObjectTypePort:
            return "PORTSHAPE";
        case ObjectType::kObjectTypeLayer:
            return "LAYER";
        case ObjectType::kObjectTypeLibrary:
            return "LIBRARY";
        case ObjectType::kObjectTypeMacro: 
            return "MACRO";
        case ObjectType::kObjectTypeNonDefaultRule:
            return "NONDEFAULTRULE";
        case ObjectType::kObjectTypePin:
            return "PIN";
        case ObjectType::kObjectTypeVia:
            return "VIA";
        case ObjectType::kObjectTypeViaRule:
            return "VIARULE";
        case ObjectType::kObjectTypeComponent:
            return "COMPONENT";
        case ObjectType::kObjectTypeComponentPin:
            return "COMPONENTPIN";
        case ObjectType::kObjectTypeDesign:
            return "DESIGN";
        case ObjectType::kObjectTypeGroup:
            return "GROUP";
        case ObjectType::kObjectTypeNet:
            return "NET";
        case ObjectType::kObjectTypeRegion:
            return "REGION";
        case ObjectType::kObjectTypeRow:
            return "ROW";
        case ObjectType::kObjectTypeSpecialNet:
            return "SPECIALNET";
        default:
            return "UNKNOWN";
    }
}

/// @brief Explicitly convert const char* to enum (enum should has kUnknown.)
template <typename E>
constexpr inline typename std::enable_if<std::is_enum<E>::value, E>::type
objectTypeToEnumByString(const char *v) noexcept {
    for (int i = 0; i < toInteger(E::kObjectTypeMax); ++i) {
        if (strcasecmp(v, toString(toEnum<E, int>(i)).c_str()) == 0) {
            // if (toString(toEnum<E, int>(i)).compare(v) == 0) {
            return toEnum<E, int>(i);
        }
    }
    return E::kObjectTypeMax;
}

/// @brief Base class for all objects.

#define NEW_ARRAY_OBJECT
#ifndef NEW_ARRAY_OBJECT
template <typename T>
class ArrayObject : public std::vector<T> {
  public:
    ArrayObject() : std::vector<T>(0) {}
};
#endif

#define UNINIT_OBJECT_ID 0
class Cell;
class StorageUtil;
const int64_t kInvalidObjectId = UNINIT_OBJECT_ID;

class Object {
  public:
    using CoordinateType = CoordinateTraits<int32_t>::CoordinateType;
    using IndexType = CoordinateTraits<CoordinateType>::IndexType;

    template <class T>
    static T* addr(ObjectId obj_id);

    template <class T>
    static T *createObject(ObjectType type, ObjectId owner_id);

    template <class T>
    static void destroyObject(T *obj);

    /// @brief default constructor
    Object();

    /// @brief constructor
    Object(Object *o, IndexType id);

    /// @brief copy constructor
    Object(Object const &rhs);

    /// @brief move constructor
    Object(Object &&rhs);

    /// @brief copy assignment
    Object &operator=(Object const &rhs);

    /// @brief move assignment
    Object &operator=(Object &&rhs);

    /// @brief destructor
    ~Object();

    /// @brief summarize memory usage of the object in bytes
    IndexType memory() const;

    /// @brief get the owner of the object
    /// @return the owner id
    ObjectId const getOwnerId() const;

    /// @brief get the owner of the object
    /// @return the owner id
    ObjectId getOwnerId();

    /// @brief get the storage_util of the object
    /// @return the StorageUtil ptr
    StorageUtil *getStorageUtil() const;

    /// @brief get the storage_util by the specified owner_id
    /// @return the StorageUtil ptr
    static StorageUtil *getStorageUtilById(ObjectId owner_id);
    /// @brief get the memory pool by the specified owner_id
    /// @return the MemPagePool ptr
    static MemPagePool *getPoolById(ObjectId owner_id);

    /// @brief set the owner of the object
    void setOwner(Object *v);
    void setOwner(ObjectId id);

    /// @brief get id of the object
    /// @return id
    ObjectId getId() const { return id_; }

    /// @brief set id of the object
    /// @param id
    void setId(IndexType id) { id_ = id; }

    /// @brief get db type of the object
    /// @return type
    ObjectType getObjectType() const { return type_; }

    /// @brief set id of the object
    /// @param type
    void setObjectType(ObjectType type) { type_ = type; }

    void setIsValid(int v);
    int  getIsValid();
    void setIsSelected(int v);
    int  getIsSelected();
    void setIsHighlight(int v);
    int  getIsHighlight();
    void setIsModified(int v);
    int  getIsModified();
    void setIsMarked(int v);
    int  getIsMarked();

  protected:
    /// @brief copy object
    void copy(Object const &rhs);
    /// @brief move object
    void move(Object &&rhs);
    /// @brief overload output stream
    friend OStreamBase &operator<<(OStreamBase &os, Object const &rhs);
    /// @brief overload input stream
    friend IStreamBase &operator>>(IStreamBase &is, Object &rhs);

    ObjectId __createObjectIdArray(int64_t size);
    void __destroyObjectIdArray(ObjectId array_id);

    ObjectId id_ : 56;  ///< object ID
    ObjectIndex owner_;  ///< using for object's hier cell (techlib/timinglib) access.
    ObjectType type_ : 16;
    Bits is_valid_ : 1;   ///< below 16 bits for flag usage.
    Bits is_selected_ : 1;
    Bits is_highlight_ : 1;
    Bits is_modified_ : 1;
    Bits is_marked_ : 1;
    Bits null_ : 11;
};

template <class T>
T* Object::addr(uint64_t obj_id)
{
    if (obj_id == 0) return nullptr;

    MemPagePool *pool = MemPool::getPagePoolByObjectId(obj_id);
    if (pool) {
        return pool->getObjectPtr<T>(obj_id);
    } else {
        return nullptr;
    }
}

/// @brief createObject create object within memory pool
/// @return the object created.
template <class T>
T* Object::createObject(ObjectType type, ObjectId owner_id) {
    assert(type > kObjectTypeNone && type < kObjectTypeMax);
    assert(owner_id != 0);
    MemPagePool *pool = getPoolById(owner_id);

    if (!pool) {
        message->issueMsg(kError,
                          "Cannot create object for type %d because memory "
                          "pool is null.\n",
                          type);
        return nullptr;
    }
    T *obj = nullptr;
    ObjectId id;
    obj = pool->allocate<T>(type, id);
    if (!obj) {
        message->issueMsg(kError, "Pool allocate null object.\n");
        return nullptr;
    }
    obj->setId(id);
    obj->setObjectType(type);
    obj->setIsValid(1);
    obj->setOwner(owner_id);

    return obj;
}

template <class T>
void Object::destroyObject(T *obj) {
    if (!obj) return;

    if (obj->getId()) {
        ObjectType type = obj->getObjectType();
        MemPagePool *pool = MemPool::getPagePoolByObjectId(obj->getId());

        if (!pool) {
            // Internal debug message:
            message->issueMsg(kError,
                              "Cannot destroy object for type %d because memory "
                              "pool is null.\n",
                              type);
            return;
        }

        obj->setIsValid(0);
        pool->free<T>(type, obj);
        obj = nullptr;
    } else {
        delete obj;
    }
}

}  // namespace db
}  // namespace open_edi

#endif
