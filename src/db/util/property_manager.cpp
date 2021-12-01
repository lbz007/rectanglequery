/** 
 * @file  property_table.cpp
 * @date  12/17s/2020PM CST
 * @brief ""
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "db/util/property_manager.h"
#include "db/tech/property.h"
#include "util/message.h"
#include "db/util/property_definition.h"
#include "db/tech/tech.h"
#include "db/core/cell.h"
#include "db/core/db.h"
#include "util/util.h"

#include <fstream>
#include <iostream>
//#include <cstdint>
#include <cstring>

namespace open_edi {
namespace db {


/// @brief  constructor of PropertyManager
///
/// @return
PropertyManager* PropertyManager::instance_ = nullptr;
PropertyManager* PropertyManager::getPropertyManager() {
    if (instance_ == nullptr) {
            instance_ = new PropertyManager();
    }
    return instance_;
}

/// @brief  destructor of PropertyManager
///
/// @return
PropertyManager::~PropertyManager() {
    if (sparse_map_.size() != 0 ) {
        for (sparseIt property = sparse_map_.begin(); property != sparse_map_.end(); property++) {
            if (property->second != nullptr) {
                delete property->second;
            }
        }
        sparse_map_.clear();
    }
}

void PropertyManager::createProperty(const char *name, PropertyType ptype, PropertyOrigin otype,
     PropertySaveStatus stype, PropDataType dtype) {
    Tech *lib = getTechLib();
    if (lib->getPropertyDefinitionId(name) != 0) {
        message->issueMsg("DB", 32, kError, name);
        return;
    }
    Cell* top_cell = getTopCell();
    if (!top_cell) return;

    PropertyDefinition* prop_definition =
        top_cell->createObject<PropertyDefinition>(
            kObjectTypePropertyDefinition);
    prop_definition->setPropType(ptype);
    prop_definition->setPropOriginType(otype);
    prop_definition->setPropName(name);
    prop_definition->setDataType(dtype);
    prop_definition->setPropSaveType(stype);

    lib->addPropertyDefinition(prop_definition->getId());
}

PropertyDefinition* getPropertyDefinition(const char *name) {
    Tech *lib = getTechLib();
    return lib->getPropertyDefinition(name);
}


void PropertyManager::destroyProperty(const char *name) {
    Tech *lib = getTechLib();
    PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
    delete pd;
}

// void PropertyManager::setProperty(uint64_t id, const char *name, int value) {
//    Tech *lib = getTechLib();
//     PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
//     if (pd == nullptr) {
//         message->issueMsg(kError, "Property %s has not been created yet.\n", name);
//         return;
//     }
//     SparseMap* map;
//     if (pd->getPropType() == PropertyType::kPropSparse) {
//         map = &sparse_map_;
//     } else {
//         Object* obj = Object::addr<Object>(id); 
//         int index = obj->getObjectType();
//     }

//     bool is_object = true;
//     auto kSparsePair = map->equal_range(IdType(id, is_object));
//     for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
//         Property *property = kSparseIt->second;
//         if (property == nullptr)
//             continue;
//         if (strcmp(name, property->getPropName()) == 0) {
//             if (property->getDataType() != PropDataType::kInt) {
//                 message->issueMsg(kError, "Property %s has int type.\n", name);
//                 return;
//             }
//             property->setValue(value);
//             return;
//         }
//     }
//     Property* sp = new Property();
 
//     sp->setDefinitionId(pd->getId());
//     if (sp->getDataType() != PropDataType::kInt) {
//         message->issueMsg(kError, "Property %s has int type.\n", name);
//         delete sp;
//         return;
//     }
//     sp->setValue(value);
//     map->insert(
//              std::make_pair(IdType(id, is_object), sp));
// }




// int PropertyManager::getPropertyInt(uint64_t id, const char* name) const {
//     Tech *lib = getTechLib();
//     PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
//     if (pd == nullptr) {
//         message->issueMsg(kError, "Property %s has not been created yet.\n", name);
//         return 0;
//     }
//     const SparseMap* map;
//     if (pd->getPropType() == PropertyType::kPropSparse) {
//         map = &sparse_map_;
//     } else {
//     }

//     bool is_object = true;
//     auto kSparsePair = map->equal_range(IdType(id, is_object));
//     for (auto kSparseIt = kSparsePair.first; kSparseIt != kSparsePair.second; ++kSparseIt) {
//         Property *property = kSparseIt->second;
//         if (property == nullptr)
//             continue;
//         if (strcmp(name, property->getPropName()) == 0) {
//             if (property->getDataType() != PropDataType::kInt) {
//                 message->issueMsg(kError, "Property %s data type error.\n", name);
//                 return 0;
//             }
//             return property->getIntValue();
//         }
//     }
//     message->issueMsg(kError, "Cannot find property %s \n", name);
//     return 0;
// }


bool PropertyManager::isPropertyDefined(const char *name) {
    Tech *lib = getTechLib();
    PropertyDefinition* pd = Object::addr<PropertyDefinition>(lib->getPropertyDefinitionId(name));
    if (pd == nullptr) {
        return false;
    }
    return true;
}



/// @brief  writeToFile
///
/// @return
void PropertyManager::writeToFile(std::ofstream &outfile, bool debug) {

}

/// @brief  readFromFile
///
/// @return
void PropertyManager::readFromFile(std::ifstream &infile, bool debug) {

}


}  // namespace db
}  // namespace open_edi
