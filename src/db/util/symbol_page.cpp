/** 
 * @file  symbol_page.cpp
 * @date  Sep 7, 2020
 * @brief ""
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "db/util/symbol_page.h"

namespace open_edi {
namespace db {

/// @brief SymbolPage 
SymbolPage::SymbolPage()
{
    symbols_.fill("");
    symbols_size_ = 0;
}

/// @brief ~SymbolPage 
SymbolPage::~SymbolPage()
{
    symbols_.fill("");
}

/// @brief size 
///
/// @return 
int32_t SymbolPage::size()
{
    //return symbols_.size();
    return symbols_size_;
}

/// @brief addSymbol 
///
/// @param index
/// @param name
///
/// @return 
bool SymbolPage::addSymbol(int32_t index, const char *name)
{
    if((index >= 0) && (index < SYMTBL_ARRAY_SIZE))
    {
        symbols_[index] = std::string(name);
        ++symbols_size_;
        return true;
    }
    return false;
}

/// @brief addSymbol 
///
/// @param index
/// @param name
///
/// @return 
bool SymbolPage::addSymbol(int32_t index, std::string &name)
{
    if((index >= 0) && (index < SYMTBL_ARRAY_SIZE))
    {
        symbols_[index] = std::string(name);
        ++symbols_size_;
        return true;
    }
    return false;
}

/// @brief addSymbol 
///
/// @param name
///
/// @return 
int32_t SymbolPage::addSymbol(const char *name)
{
    size_t sz = size();

    if (sz < SYMTBL_ARRAY_SIZE) {
        symbols_[sz] = std::string(name);
        ++symbols_size_;
        return sz;
    }
    return -1;
}

/// @brief addSymbol 
///
/// @param name
///
/// @return 
int32_t SymbolPage::addSymbol(std::string &name)
{
    size_t sz = size();
    if (sz < SYMTBL_ARRAY_SIZE) {
        symbols_[sz] = std::string(name);
        ++symbols_size_;
        return sz;
    }
    return -1;
}

/// @brief getSymbol 
///
/// @param index
///
/// @return 
std::string &SymbolPage::getSymbol(int32_t index)
{
    if((index >= 0) && (index < SYMTBL_ARRAY_SIZE)) {
        return symbols_[index];
    }
    static std::string tmp("");
    return tmp;
}

/// @brief addReference 
///
/// @param index
/// @param ref
///
/// @return 
bool SymbolPage::addSymbolReference(int32_t index, ObjectId ref)
{
    if((index >= 0) && (index < SYMTBL_ARRAY_SIZE))
    {
        references_[index].push_back(ref);
        return true;
    }
    return false;
}

/// @brief getReferences 
///
/// @param index
///
/// @return 
std::vector<ObjectId>& SymbolPage::getReferences(int32_t index)
{
    if((index >= 0) && (index < SYMTBL_ARRAY_SIZE)) {
        return references_[index];
    }
    static std::vector<ObjectId> tmp={0};
    return tmp;    
}

/// @brief getReferenceCount 
///
/// @param index
///
/// @return 
int32_t SymbolPage::getReferenceCount(int32_t index)
{
    if((index >= 0) && (index < SYMTBL_ARRAY_SIZE))
        return references_[index].size();
    else
        return 0;
}

/// @brief removeReference 
///
/// @param index
/// @param ref
///
/// @return 
bool SymbolPage::removeSymbolReference(int32_t index, ObjectId ref)
{
    if((index >= 0) && (index < SYMTBL_ARRAY_SIZE))
    {
        std::vector<ObjectId>::iterator it = 
            std::find(references_[index].begin(), references_[index].end(), ref);
        if (it != references_[index].end())
        {
            it = references_[index].erase(it);
            return true;
        }
    }
    return false;
}

/// @brief  
///
/// @return 
void SymbolPage::writeToFile(util::IOManager &io_manager, bool debug)
{
    // symbols count + (symbol_index + symbol_name_length + symbol_name + reference count + reference id array)
    int32_t real_size = 0;
    int32_t size = this->size();
    for (int32_t i = 0; i < size; ++i) {
        if (getReferenceCount(i) == 0) {
            continue;
        }
        ++real_size;
    }
    io_manager.write(sizeof(int32_t), (char *) &(real_size));

    if (debug) std::cout << "RWDBGINFO: SymbolPage with size " << real_size << std::endl;

    int32_t ref_count = 0;
    for (int32_t index = 0; index < size; ++index) {
        if ((ref_count = getReferenceCount(index)) == 0) {
            continue;
        }
        std::string &symbol_name = getSymbol(index);
        io_manager.write(sizeof(int32_t), (char *) &(index));
        int32_t length = symbol_name.length();
        io_manager.write(sizeof(int32_t), (char *) &(length));
        io_manager.write(sizeof(int32_t), (char *) &(ref_count));
        const char *name_str = symbol_name.c_str();
        io_manager.write(length, (char *) name_str);
        if (debug) std::cout << "RWDBGINFO:    symbol#" << index << " " << symbol_name << "::" << length << " ref objs (" << ref_count << ") are:";
        for (std::vector<ObjectId>::iterator iter = references_[index].begin(); iter != references_[index].end(); ++iter) {
            ObjectId ref_id = (*iter);
            io_manager.write(sizeof(ObjectId), (char *) &(ref_id));
            if (debug) std::cout << " " << ref_id;
        }
        if (debug) std::cout << " ;" << std::endl;
    }
}

/// @brief  readFromFile
///
/// @return 
void SymbolPage::readFromFile(util::IOManager &io_manager, bool debug)
{
    // symbols count + (symbol_index + symbol_name + reference count + reference id array)
    int32_t size = 0;
    int32_t ref_count = 0;
    io_manager.read((char *) &(size), sizeof(int32_t));
    if (debug) std::cout << "RWDBGINFO: SymbolPage with size " << size << std::endl;
    for (int32_t i = 0; i < size; ++i) {
        //std::string symbol_name = "";
        int32_t index = 0;
        int32_t length = 0;
        char *symbol_name;

        io_manager.read((char *) &(index), sizeof(int32_t));
        io_manager.read((char *) &(length), sizeof(int32_t));
        io_manager.read((char *) &(ref_count), sizeof(int32_t));

        symbol_name = new char [length + 1];
        symbol_name[length] = '\0';
        io_manager.read((char *) (symbol_name), length);
        if (debug) std::cout << "RWDBGINFO:    symbol#" << index << " " << symbol_name << "::" << length << " ref objs (" << ref_count << ") are:";
        //1. fill symbols_
        addSymbol(index, symbol_name);
        delete [] symbol_name;
        //2. fill references_ vector
        for (int32_t j = 0; j < ref_count; ++j) {
            ObjectId ref_id = 0;
            io_manager.read((char *) &(ref_id), sizeof(ObjectId));
            addSymbolReference(index, ref_id);
            if (debug) std::cout << " " << ref_id;
        }
        if (debug) std::cout << " ;" << std::endl;
    }
}

}  // namespace db 
}  // namespace open_edi