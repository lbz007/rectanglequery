/**
 * @file  property_table.h
 * @date  Dec 17, 2020
 * @brief ""
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef SRC_UTIL_PROPERTY_TABLE_H_
#define SRC_UTIL_PROPERTY_TABLE_H_

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <map>

#include "util/data_traits.h"
#include "util/namespace.h"
#include "db/tech/property.h"
#include "util/util.h"
#include "db/core/object.h"
#include "db/util/property_definition.h"
//#include "db/core/db.h"
#include "db/tech/tech.h"

namespace open_edi {
namespace db {

typedef std::pair<uint64_t, bool> IdType;  // if bool is true, is ObjectId, otherwise, is addr.  <id, name>? better
typedef std::multimap<IdType, db::Property*> SparseMap;
//typedef std::pair<SparseMap::iterator, SparseMap::iterator> SparsePair;
typedef SparseMap::iterator sparseIt;
extern Tech* getTechLib();

class PropertyManager {
  public:
    static PropertyManager* getPropertyManager();
    ~PropertyManager();
    void createProperty(const char *name, PropertyType ptype, PropertyOrigin otype,
                       PropertySaveStatus stype, PropDataType dtype);
    void destroyProperty(const char *name);

    PropertyDefinition* getPropertyDefinition(const char *name);

    // void setProperty(uint64_t id, const char *name, int value);
    // void setProperty(uint64_t id, const char *name, double value);
    // void setProperty(uint64_t id, const char *name, bool value);
    // void setProperty(uint64_t id, const char *name, const char* value);
    // void setProperty(uint64_t id, const char *name, ObjectId value);
    // void setProperty(uint64_t id, const char *name, void* value);

    template <class T>
    void setProperty(T* obj, const char *name, int value);
    template <class T>
    void setProperty(T* obj, const char *name, double value);
    template <class T>
    void setProperty(T* obj, const char *name, bool value);
    template <class T>
    void setProperty(T* obj, const char *name, const char* value);
    template <class T>
    void setProperty(T* obj, const char *name, uint64_t value);
    template <class T>
    void setProperty(T* obj, const char *name, void* value);

    template <class T>
    bool isPropertySet(const T* obj, const char *name);

    bool isPropertyDefined(const char *name);

    // int getPropertyInt(uint64_t id, const char* name) const;
    // bool getPropertyBool(uint64_t id, const char* name) const;
    // double getPropertyDouble(uint64_t id, const char* name) const;
    // std::string getPropertyString(uint64_t id, const char* name) const;
    // uint64_t getPropertyObjectId(uint64_t id, const char* name) const;
    // void* getPropertyPointer(uint64_t id, const char* name) const;
    template <class T>
    void getSparsePropertyList(const T* obj, std::vector<std::string>& properties) const;
    template <class T>
    Property* getSparseProperty(const T* obj, const char* value) const;
    template <class T>
    int getPropertyInt(const T* obj, const char* name) const;
    template <class T>
    double getPropertyDouble(const T* obj, const char* name) const;
    template <class T>
    bool getPropertyBool(const T* obj, const char* name) const;
    template <class T>
    std::string getPropertyString(const T* obj, const char* name) const;
    template <class T>
    uint64_t getPropertyObjectId(const T* obj, const char* name) const;
    template <class T>
    void* getPropertyPointer(const T* obj, const char* name) const;
    
    template <class T>
    int removeProperty(T* obj, const char* name);
    template <class T>
    int removeProperties(T* obj);

    //int removeProperty(void* id, const char* name);
    //int removeProperties(void* id);

    void writeToFile(std::ofstream &outfile, bool debug);
    void readFromFile(std::ifstream &infile, bool debug);

  private:
    PropertyManager() {}
    static PropertyManager* instance_;
    
    SparseMap sparse_map_;
};

template <class T>
void PropertyManager::setProperty(T* obj, const char *name, int value) {
    int type = obj->getObjectType();
    uint64_t id;
    if (type <= kObjectTypeMax) {
        id = obj->getId();
    } else {
        //id = obj->getIndex();
    }

    Tech *lib = getTechLib();
    PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
    if (pd == nullptr) {
        message->issueMsg("DB", 33, kError, name);
        return;
    }
    SparseMap* map;
    if (pd->getPropType() == PropertyType::kPropSparse) {
        map = &sparse_map_;
    } else {
    }

    bool is_object = true;
    auto kSparsePair = map->equal_range(IdType(id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        if (strcmp(name, property->getPropName()) == 0) {
            if (property->getDataType() != PropDataType::kInt) {
                message->issueMsg("DB", 34, kError, name, "int");
                return;
            }
            property->setValue(value);
            return;
        }
    }
    Property* sp = new Property();
 
    sp->setDefinitionId(pd->getId());
    if (sp->getDataType() != PropDataType::kInt) {
        message->issueMsg("DB", 34, kError, name, "int");
        delete sp;
        return;
    }
    sp->setValue(value);
    map->insert(
             std::make_pair(IdType(id, is_object), sp));
}

template <class T>
void PropertyManager::setProperty(T* obj, const char *name, double value) {
    int type = obj->getObjectType();
    uint64_t id;
    if (type <= kObjectTypeMax) {
        id = obj->getId();
    } else {
        //id = obj->getIndex();
    }

    Tech *lib = getTechLib();
    PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
    if (pd == nullptr) {
        message->issueMsg("DB", 33, kError, name);
        return;
    }
    SparseMap* map;
    if (pd->getPropType() == PropertyType::kPropSparse) {
        map = &sparse_map_;
    } else {
    }

    bool is_object = true;
    auto kSparsePair = map->equal_range(IdType(id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        if (strcmp(name, property->getPropName()) == 0) {
            if (property->getDataType() != PropDataType::kReal) {
                message->issueMsg("DB", 34, kError, name, "double");
                return;
            }
            property->setValue(value);
            return;
        }
    }
    Property* sp = new Property();
    sp->setDefinitionId(pd->getId());
    if (sp->getDataType() != PropDataType::kReal) {
        message->issueMsg("DB", 34, kError, name, "double");
        delete sp;
        return;
    }
    sp->setValue(value);
    map->insert(
             std::make_pair(IdType(id, is_object), sp));
}

template <class T>
void PropertyManager::setProperty(T* obj, const char *name, bool value) {
    int type = obj->getObjectType();
    uint64_t id;
    if (type <= kObjectTypeMax) {
        id = obj->getId();
    } else {
        //id = obj->getIndex();
    }

    Tech *lib = getTechLib();
    PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
    if (pd == nullptr) {
        message->issueMsg("DB", 33, kError, name);
        return;
    }
    SparseMap* map;
    if (pd->getPropType() == PropertyType::kPropSparse) {
        map = &sparse_map_;
    } else {
    }

    bool is_object = true;
    auto kSparsePair = map->equal_range(IdType(id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        if (strcmp(name, property->getPropName()) == 0) {
            if (property->getDataType() != PropDataType::kBool) {
                message->issueMsg("DB", 34, kError, name, "bool");
                return;
            }
            property->setValue(value);
            return;
        }
    }
    Property* sp = new Property();
    sp->setDefinitionId(pd->getId());
    if (sp->getDataType() != PropDataType::kBool) {
        message->issueMsg("DB", 34, kError, name, "bool");
        delete sp;
        return;
    }
    sp->setValue(value);
    map->insert(
             std::make_pair(IdType(id, is_object), sp));
}

template <class T>
void PropertyManager::setProperty(T* obj, const char *name, void* value) {
    int type = obj->getObjectType();
    uint64_t id;
    if (type <= kObjectTypeMax) {
        id = obj->getId();
    } else {
        //id = obj->getIndex();
    }

    Tech *lib = getTechLib();
    PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
    if (pd == nullptr) {
        message->issueMsg("DB", 33, kError, name);
        return;
    }
    SparseMap* map;
    if (pd->getPropType() == PropertyType::kPropSparse) {
        map = &sparse_map_;
    } else {
    }

    bool is_object = true;
    auto kSparsePair = map->equal_range(IdType(id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        if (strcmp(name, property->getPropName()) == 0) {
            if (property->getDataType() != PropDataType::kPointer) {
                message->issueMsg("DB", 34, kError, name, "Pointer");
                return;
            }
            property->setValue(value);
            return;
        }
    }

    Property* sp = new Property();
    sp->setDefinitionId(pd->getId());
    if (sp->getDataType() != PropDataType::kPointer) {
        message->issueMsg("DB", 34, kError, name, "Pointer");
        delete sp;
        return;
    }
    sp->setValue(value);
    map->insert(
             std::make_pair(IdType(id, is_object), sp));
}

template <class T>
void PropertyManager::setProperty(T* obj, const char *name, ObjectId value) {
    int type = obj->getObjectType();
    uint64_t id;
    if (type <= kObjectTypeMax) {
        id = obj->getId();
    } else {
        //id = obj->getIndex();
    }

    Tech *lib = getTechLib();
    PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
    if (pd == nullptr) {
        message->issueMsg("DB", 33, kError, name);
        return;
    }
    SparseMap* map;
    if (pd->getPropType() == PropertyType::kPropSparse) {
        map = &sparse_map_;
    } else {
    }

    bool is_object = true;
    auto kSparsePair = map->equal_range(IdType(id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        if (strcmp(name, property->getPropName()) == 0) {
            if (property->getDataType() != PropDataType::kObjectId) {
                message->issueMsg("DB", 34, kError, name, "ObjectId");
                return;
            }
            property->setValue(value);
            return;
        }
    }

    Property* sp = new Property();
    sp->setDefinitionId(pd->getId());
    if (sp->getDataType() != PropDataType::kObjectId) {
        message->issueMsg("DB", 34, kError, name, "ObjectId");
        delete sp;
        return;
    }
    sp->setValue(value);
    map->insert(
             std::make_pair(IdType(id, is_object), sp));
}

template <class T>
void PropertyManager::setProperty(T* obj, const char *name, const char* value) {
    int type = obj->getObjectType();
    uint64_t id;
    if (type <= kObjectTypeMax) {
        id = obj->getId();
    } else {
        //id = obj->getIndex();
    }

    if (value == nullptr) {
        message->issueMsg("DB", 35, kError, name);
        return;
    }
    Tech *lib = getTechLib();
    PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
    if (pd == nullptr) {
        message->issueMsg("DB", 33, kError, name);
        return;
    }
    SparseMap* map;
    if (pd->getPropType() == PropertyType::kPropSparse) {
        map = &sparse_map_;
    } else {
    }

    bool is_object = true;
    auto kSparsePair = map->equal_range(IdType(id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        if (strcmp(name, property->getPropName()) == 0) {
            if (property->getDataType() != PropDataType::kString) {
                message->issueMsg("DB", 34, kError, name, "string");
                return;
            }
            property->setValue(value);
            return;
        }
    }

    Property* sp = new Property();
    sp->setDefinitionId(pd->getId());
    if (sp->getDataType() != PropDataType::kString) {
        message->issueMsg("DB", 34, kError, name, "string");
        delete sp;
        return;
    }
    sp->setValue(value);
    map->insert(
             std::make_pair(IdType(id, is_object), sp));
}




template <class T>
void PropertyManager::getSparsePropertyList(const T* obj, std::vector<std::string>& properties) const {
    int type = obj->getObjectType();
    uint64_t id;
    if (type <= kObjectTypeMax) {
        id = obj->getId();
    } else {
        //id = obj->getIndex();
    }

    const SparseMap* map = &sparse_map_;
    bool is_object = true;
    auto kSparsePair = map->equal_range(IdType(id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        properties.push_back(property->getPropName());
    }
}
    
template <class T>
int PropertyManager::getPropertyInt(const T* obj, const char* name) const {
    int type = obj->getObjectType();
    uint64_t id;
    if (type <= kObjectTypeMax) {
        id = obj->getId();
    } else {
        //id = obj->getIndex();
    }

    Tech *lib = getTechLib();
    PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
    if (pd == nullptr) {
        message->issueMsg("DB", 33, kError, name);
        return 0;
    }
    const SparseMap* map;
    if (pd->getPropType() == PropertyType::kPropSparse) {
        map = &sparse_map_;
    } else {
    }

    bool is_object = true;
    auto kSparsePair = map->equal_range(IdType(id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        if (strcmp(name, property->getPropName()) == 0) {
            if (property->getDataType() != PropDataType::kInt) {
                message->issueMsg("DB", 36, kError, name);
                return 0;
            }
            return property->getIntValue();
        }
    }
    message->issueMsg("DB", 37, kError, name);
    return 0;
}

template <class T>
bool PropertyManager::getPropertyBool(const T* obj, const char* name) const {
    int type = obj->getObjectType();
    uint64_t id;
    if (type <= kObjectTypeMax) {
        id = obj->getId();
    } else {
        //id = obj->getIndex();
    }

    Tech *lib = getTechLib();
    PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
    if (pd == nullptr) {
        message->issueMsg("DB", 33, kError, name);
        return 0;
    }
    const SparseMap* map;
    if (pd->getPropType() == PropertyType::kPropSparse) {
        map = &sparse_map_;
    } else {
    }

    bool is_object = true;
    auto kSparsePair = map->equal_range(IdType(id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        if (strcmp(name, property->getPropName()) == 0) {
            if (property->getDataType() != PropDataType::kBool) {
                message->issueMsg("DB", 36, kError, name);
                return 0;
            }
            return property->getBoolValue();
        }
    }
    message->issueMsg("DB", 37, kError, name);
    return 0;
}

template <class T>
double PropertyManager::getPropertyDouble(const T* obj, const char* name) const {
    int type = obj->getObjectType();
    uint64_t id;
    if (type <= kObjectTypeMax) {
        id = obj->getId();
    } else {
        //id = obj->getIndex();
    }

    Tech *lib = getTechLib();
    PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
    if (pd == nullptr) {
        message->issueMsg("DB", 33, kError, name);
        return 0;
    }
    const SparseMap* map;
    if (pd->getPropType() == PropertyType::kPropSparse) {
        map = &sparse_map_;
    } else {
    }

    bool is_object = true;
    auto kSparsePair = map->equal_range(IdType(id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        if (strcmp(name, property->getPropName()) == 0) {
            if (property->getDataType() != PropDataType::kReal) {
                message->issueMsg("DB", 36, kError, name);
                return 0;
            }
            return property->getRealValue();
        }
    }
    message->issueMsg("DB", 37, kError, name);
    return 0;
}

template <class T>
std::string PropertyManager::getPropertyString(const T* obj, const char* name) const {
    int type = obj->getObjectType();
    uint64_t id;
    if (type <= kObjectTypeMax) {
        id = obj->getId();
    } else {
        //id = obj->getIndex();
    }

    Tech *lib = getTechLib();
    PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
    if (pd == nullptr) {
        message->issueMsg("DB", 33, kError, name);
        return 0;
    }
    const SparseMap* map;
    if (pd->getPropType() == PropertyType::kPropSparse) {
        map = &sparse_map_;
    } else {
    }

    bool is_object = true;
    auto kSparsePair = map->equal_range(IdType(id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        if (strcmp(name, property->getPropName()) == 0) {
            if (property->getDataType() != PropDataType::kString) {
                message->issueMsg("DB", 36, kError, name);
                return "";
            }
            return property->getStringValue();
        }
    }
    message->issueMsg("DB", 37, kError, name);
    return "";
}

template <class T>
uint64_t PropertyManager::getPropertyObjectId(const T* obj, const char* name) const {
    int type = obj->getObjectType();
    uint64_t id;
    if (type <= kObjectTypeMax) {
        id = obj->getId();
    } else {
        //id = obj->getIndex();
    }

    Tech *lib = getTechLib();
    PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
    if (pd == nullptr) {
        message->issueMsg("DB", 33, kError, name);
        return 0;
    }
    const SparseMap* map;
    if (pd->getPropType() == PropertyType::kPropSparse) {
        map = &sparse_map_;
    } else {
    }

    bool is_object = true;
    auto kSparsePair = map->equal_range(IdType(id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        if (strcmp(name, property->getPropName()) == 0) {
            if (property->getDataType() != PropDataType::kObjectId) {
                message->issueMsg("DB", 36, kError, name);
                return 0;
            }
            return property->getObjectIdValue();
        }
    }
    message->issueMsg("DB", 37, kError, name);
    return 0;
}

template <class T>
void* PropertyManager::getPropertyPointer(const T* obj, const char* name) const {
    int type = obj->getObjectType();
    uint64_t id;
    if (type <= kObjectTypeMax) {
        id = obj->getId();
    } else {
        //id = obj->getIndex();
    }

    Tech *lib = getTechLib();
    PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
    if (pd == nullptr) {
        message->issueMsg("DB", 33, kError, name);
        return 0;
    }
    const SparseMap* map;
    if (pd->getPropType() == PropertyType::kPropSparse) {
        map = &sparse_map_;
    } else {
    }

    bool is_object = true;
    auto kSparsePair = map->equal_range(IdType(id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        if (strcmp(name, property->getPropName()) == 0) {
            if (property->getDataType() != PropDataType::kPointer) {
                message->issueMsg("DB", 36, kError, name);
                return nullptr;
            }
            return property->getPointerValue();
        }
    }
    message->issueMsg("DB", 37, kError, name);
    return nullptr;
}

template <class T>
bool PropertyManager::isPropertySet(const T* obj, const char *name) {
    uint64_t id = obj->getId();

    bool is_object = true;
    auto kSparsePair = sparse_map_.equal_range(IdType((uint64_t)id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        if (strcmp(name, property->getPropName()) == 0) {
            return true;
        }
    }
    return false;
}

template <class T>
Property* PropertyManager::getSparseProperty(const T* obj, const char *name) const {
    uint64_t id = obj->getId();
    bool is_object = true;
    auto kSparsePair = sparse_map_.equal_range(IdType((uint64_t)id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        if (strcmp(name, property->getPropName()) == 0) {
            return property;
        }
    }
    return nullptr;
}

template <class T>
int PropertyManager::removeProperty(T* obj, const char* name) {
    int type = obj->getObjectType();
    uint64_t id;
    if (type <= kObjectTypeMax) {
        id = obj->getId();
    } else {
        //id = obj->getIndex();
    }

    bool is_object = true;
    auto kSparsePair = sparse_map_.equal_range(IdType(id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        if (strcmp(name, property->getPropName()) == 0) {
            delete property;
            sparse_map_.erase(kSparseIt);
            return 0;
        }
    }
    return -1;
}

template <class T>
int PropertyManager::removeProperties(T* obj) {  // 
    int type = obj->getObjectType();
    uint64_t id;
    if (type <= kObjectTypeMax) {
        id = obj->getId();
    } else {
        //id = obj->getIndex();
    }

    bool is_object = true;
    auto kSparsePair = sparse_map_.equal_range(IdType(id, is_object));
    for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
        Property *property = kSparseIt->second;
        if (property == nullptr)
            continue;
        delete property;
        sparse_map_.erase(kSparseIt);
    }
    return 0;
}


}  // namespace db
}  // namespace open_edi

#endif  //  SRC_UTIL_PROPERTY_TABLE_H_
