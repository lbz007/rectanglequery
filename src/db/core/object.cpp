/* @file  object.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "db/core/object.h"
#include "db/core/cell.h"
#include "db/core/db.h"
#include "db/util/array.h"

namespace open_edi {
namespace db {
using IdArray = ArrayObject<ObjectId>;

/// @brief Object 
Object::Object() {
    setId(UNINIT_OBJECT_ID);
    owner_ = 0;
}

/// @brief Object 
///
/// @param o
/// @param id
Object::Object(Object* o, IndexType id) : id_(id) {
    //owner_ = o->getId();
}

/// @brief Object 
///
/// @param rhs
Object::Object(Object const& rhs) { copy(rhs); }

/// @brief Object 
///
/// @param rhs
Object::Object(Object&& rhs) { move(std::move(rhs)); }

/// @brief operator= 
///
/// @param rhs
///
/// @return 
Object& Object::operator=(Object const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

/// @brief operator= 
///
/// @param rhs
///
/// @return 
Object& Object::operator=(Object&& rhs) {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

/// @brief ~Object 
Object::~Object() {}

/// @brief memory 
///
/// @return 
Object::IndexType Object::memory() const { return sizeof(*this); }

/// @brief copy 
///
/// @param rhs
void Object::copy(Object const& rhs) {
    owner_ = rhs.owner_;
    id_ = rhs.id_;
}

/// @brief move 
///
/// @param rhs
void Object::move(Object&& rhs) {
    owner_ = std::move(rhs.owner_);
    // owner_ = std::exchange(rhs.owner_, nullptr);
    id_ = std::move(static_cast<unsigned long>(rhs.id_));
}

/// @brief setIsValid 
///
/// @param v
void Object::setIsValid(int v) {is_valid_ = v;}

/// @brief getIsValid 
///
/// @return 
int Object::getIsValid() {return is_valid_;}

/// @brief setIsSelected
///
/// @param v
void Object::setIsSelected(int v) {is_selected_ = v;}

/// @brief getIsSelected
///
/// @return 
int Object::getIsSelected() {return is_selected_;}

/// @brief setIsHighlight 
///
/// @param v
void Object::setIsHighlight(int v) {is_highlight_ = v;}

/// @brief getIsHighlight 
///
/// @return 
int Object::getIsHighlight() {return is_highlight_;}

/// @brief setIsModified 
///
/// @param v
void Object::setIsModified(int v) {is_modified_ = v;}

/// @brief getIsModified 
///
/// @return 
int Object::getIsModified() {return is_modified_;}

/// @brief setIsMarked 
///
/// @param v
void Object::setIsMarked(int v) {is_marked_ = v;}

/// @brief getIsMarked 
///
/// @return 
int Object::getIsMarked() {return is_marked_;}

ObjectId const Object::getOwnerId() const { return getRoot()->getHierOwnerObjectId(owner_); }

/// @brief get the owner of the object=
/// @return the owner id
ObjectId Object::getOwnerId() { return getRoot()->getHierOwnerObjectId(owner_); }

/// @brief setOwner
///
/// @return 
void Object::setOwner(ObjectId owner_id) {
    ediAssert(owner_id != 0);
    ObjectIndex index = getRoot()->getHierOwnerIndex(owner_id);
    if (index == 0) {
        // when the specified owner_id is a leaf cell
        // we need to get its owner to fetch the real hier owner.
        Object *owner_ptr = addr<Object>(owner_id);
        ediAssert(owner_ptr != nullptr);
        ObjectId owner_owner_id = owner_ptr->getOwnerId();
        index = getRoot()->getHierOwnerIndex(owner_owner_id);
    }
    ediAssert(index != 0);
    owner_ = index;
}

/// @brief setOwner
///
/// @return 
void Object::setOwner(Object *owner_ptr) {
    ediAssert(owner_ptr != nullptr);
    ObjectId owner_id = owner_ptr->getId();
    setOwner(owner_id);
}

/// @brief getStorageUtilById
///
/// @return StorageUtil
StorageUtil *Object::getStorageUtilById(ObjectId owner_id) {
    Object *owner_ptr = addr<Object>(owner_id);
    StorageUtil *storage_util = nullptr;
    switch (owner_ptr->getObjectType()) {
        case kObjectTypeCell:
        {
            Cell *cell = addr<Cell>(owner_id);
            storage_util = cell->getStorageUtil();
            break;
        }
        case kObjectTypeTiming:
        {
            Timing *timing = addr<Timing>(owner_id);
            storage_util = timing->getStorageUtil();
            break;
        }
        case kObjectTypeTech:
        {
            Tech *tech = addr<Tech>(owner_id);
            storage_util = tech->getStorageUtil();
            break;
        }
        default:
            break;
    }
    return storage_util;
}

/// @brief getPoolById
///
/// @return MemPagePool
MemPagePool *Object::getPoolById(ObjectId owner_id) {
    StorageUtil *storage_util = getStorageUtilById(owner_id);
    assert(storage_util != nullptr);
    MemPagePool *pool = storage_util->getPool();
    return pool;
}

/// @brief getStorageUtil
///
/// @return StorageUtil
StorageUtil *Object::getStorageUtil() const {
    ediAssert(owner_ != 0);
    ObjectId owner_object_id = getRoot()->getHierOwnerObjectId(owner_);
    ediAssert(owner_object_id != 0);
    return Object::getStorageUtilById(owner_object_id);
}

ObjectId Object::__createObjectIdArray(int64_t size) {
    if (size <= 0) return 0;
    ObjectId owner_id = getOwnerId();
    if (!owner_id) return 0;
    IdArray *id_array_ptr =
      Object::createObject<IdArray>(kObjectTypeArray, owner_id);
    ediAssert(id_array_ptr != nullptr);
    StorageUtil *storage_util = getStorageUtilById(owner_id);
    assert(storage_util != nullptr);
    id_array_ptr->setPool(storage_util->getPool());
    id_array_ptr->reserve(size);
    return (id_array_ptr->getId());
}

void Object::__destroyObjectIdArray(ObjectId array_id) {
    if (!array_id) return;
    IdArray *id_array_ptr = addr<IdArray>(array_id);
    ediAssert(id_array_ptr != nullptr);
    Object::destroyObject(id_array_ptr);
}

/// @brief operator<< 
///
/// @param os
/// @param rhs
///
/// @return 
OStreamBase& operator<<(OStreamBase& os, Object const& rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");

    os << DataFieldName("owner_");
    if (rhs.owner_) {
        os << rhs.owner_;
        // os << rhs.owner_->getId();
    } else {
        os << UNINIT_OBJECT_ID;
    }
    os << DataDelimiter();
    os << DataFieldName("id_") << rhs.id_ << DataDelimiter();

    os << DataEnd(")");
    return os;
}

/// @brief operator>> 
///
/// @param is
/// @param rhs
///
/// @return 
IStreamBase& operator>>(IStreamBase& is, Object& rhs) {
    is >> DataTypeName(className(rhs)) >> DataBegin("(");

    auto context = is.getContext();

    Object::IndexType owner_id = rhs.owner_;
    Object::IndexType id = rhs.id_;
    is >> DataFieldName("owner_") >> owner_id >> DataDelimiter();
    is >> DataFieldName("id_") >> id >> DataDelimiter();

    if (context) {
        ediAssertMsg(owner_id == context->getId(),
                     "owner id %u mismatch %u in object %u", owner_id,
                     context->getId(), rhs.getId());
    } else {
        ediAssertMsg(owner_id == UNINIT_OBJECT_ID,
                     "owner %u mismatch nullptr in object %u", owner_id,
                     rhs.getId());
    }

    // read owner_
    rhs.owner_ = context->getId();

    is >> DataEnd(")");
    return is;
}

}  // namespace db
}  // namespace open_edi
