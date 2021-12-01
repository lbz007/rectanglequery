/**
 * @file timinglib_function.h
 * @date 2020-10-09
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
#ifndef SRC_DB_TIMING_TIMINGLIB_TIMINGLIB_FUNCTION_H_
#define SRC_DB_TIMING_TIMINGLIB_TIMINGLIB_FUNCTION_H_

#include <algorithm>
#include <string>
#include <utility>
#include <unordered_map>

#include "db/core/object.h"
#include "db/timing/timinglib/timinglib_commondef.h"
#include "db/timing/timinglib/timinglib_libbuilder.h"

namespace open_edi {
namespace db {

class TTerm;

struct TFuncToken {
    std::string token;
    bool is_operator;
    FuncOpType op;
};

enum class TFuncConstant : unsigned char {
    kConstantZero,
    kConstantOne,
    kUnknown
};

TFuncConstant operator&(const TFuncConstant& c1, const TFuncConstant& c2);
TFuncConstant operator+(const TFuncConstant& c1, const TFuncConstant& c2);
TFuncConstant operator^(const TFuncConstant& c1, const TFuncConstant& c2);
TFuncConstant operator!(const TFuncConstant& c1);

class TFunction : public Object {
  public:
    using BaseType = Object;

    /// @brief default constructor
    TFunction();

    /// @brief destructor
    ~TFunction();

    /// @brief constructor
    TFunction(Object *owner, IndexType id);

    /// @brief copy constructor
    TFunction(TFunction const &rhs);

    /// @brief move constructor
    TFunction(TFunction &&rhs) noexcept;

    /// @brief copy assignment
    TFunction &operator=(TFunction const &rhs);

    /// @brief move assignment
    TFunction &operator=(TFunction &&rhs) noexcept;

    /// @brief summarize memory usage of the object in bytes
    IndexType memory() const;

    /// set
    void setTterm(ObjectId id);
    void setOp(FuncOpType t);
    void setLeft(ObjectId id);
    void setRight(ObjectId id);
    void setGroupId(ObjectId id);
    void setFuncStr(const std::string &str);

    /// get
    TTerm *getTterm(void);
    FuncOpType getOp(void);
    TFunction *getLeft(void);
    TFunction *getRight(void);
    ObjectId getGroupId(void);
    /// When timing sense is not specified, we can find it from function.
    TimingSense getTimingSense(const TTerm* pTerm);

    /// @brief output the information
    void print(std::ostream &stream);
    void buildExpressionTree(const std::string& str, const TTerm* related_pin, str_omap_t<std::vector<tb_namespace::TTerm *>>& bus_pin_map);
    void buildExpressionTree(const TTerm* related_pin, str_omap_t<std::vector<tb_namespace::TTerm *>>& bus_pin_map);
    TFuncConstant eval(std::unordered_map<TTerm*, TFuncConstant>& pin_values);

  protected:
    /// @brief copy object
    void copy(TFunction const &rhs);
    /// @brief move object
    void move(TFunction &&rhs);
    /// @brief overload output stream
    friend OStreamBase &operator<<(OStreamBase &os, TFunction const &rhs);

    static void tokenize(const std::string& func_str, std::vector<TFuncToken>& tokens);
    //void tokenize(std::vector<TFuncToken>& tokens) const;
    ObjectId newTFuncNode(FuncOpType op, std::string& op_char, ObjectId left, ObjectId right, ObjectId tterm);
    ObjectId newOprandNode(TFuncToken& t, const TTerm* related_pin, str_omap_t<std::vector<tb_namespace::TTerm *>>& bus_pin_map);
    ObjectId buildExpressionTree(std::vector<TFuncToken>& tokens, const TTerm* related_pin, str_omap_t<std::vector<tb_namespace::TTerm *>>& bus_pin_map);
    int operatorPrecedence(const TFuncToken& t);

  private:
    SymbolIndex func_str_;
    FuncOpType op_;
    ObjectId tterm_;
    ObjectId left_;
    ObjectId right_;
    ObjectId group_id_;
};
}  // namespace db
}  // namespace open_edi

#endif  // SRC_DB_TIMING_TIMINGLIB_TIMINGLIB_FUNCTION_H_
