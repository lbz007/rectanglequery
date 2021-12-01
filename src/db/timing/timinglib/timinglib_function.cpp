/**
 * @file timinglib_function.cpp
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
#include "db/timing/timinglib/timinglib_function.h"

#include "db/core/db.h"
#include "db/timing/timinglib/timinglib_term.h"
#include "db/timing/timinglib/timinglib_cell.h"
#include "db/timing/timinglib/timinglib_sequential.h"
#include <stack>


namespace open_edi {
namespace db {

TFuncConstant operator&(const TFuncConstant& c1, const TFuncConstant& c2) 
{
    if (c1 == TFuncConstant::kConstantZero || c2 == TFuncConstant::kConstantZero) { 
        return TFuncConstant::kConstantZero;
    } else if (c1 == TFuncConstant::kUnknown || c2 == TFuncConstant::kUnknown) {
        return TFuncConstant::kUnknown;
    } else {
        return TFuncConstant::kConstantOne;
    }
}

TFuncConstant operator+(const TFuncConstant& c1, const TFuncConstant& c2) 
{
    if (c1 == TFuncConstant::kConstantOne || c2 == TFuncConstant::kConstantOne) { 
        return TFuncConstant::kConstantOne;
    } else if (c1 == TFuncConstant::kUnknown || c2 == TFuncConstant::kUnknown) {
        return TFuncConstant::kUnknown;
    } else {
        return TFuncConstant::kConstantZero;
    }
}

TFuncConstant operator^(const TFuncConstant& c1, const TFuncConstant& c2) 
{
    if (c1 == TFuncConstant::kUnknown || c2 == TFuncConstant::kUnknown) { 
        return TFuncConstant::kUnknown;
    } else if (c1 != c2) {
        return TFuncConstant::kConstantOne;
    } else {
        return TFuncConstant::kConstantZero;
    }
}

TFuncConstant operator!(const TFuncConstant& c1) 
{
    if (c1 == TFuncConstant::kUnknown) { 
        return TFuncConstant::kUnknown;
    } else if (c1 == TFuncConstant::kConstantZero) {
        return TFuncConstant::kConstantOne;
    } else {
        return TFuncConstant::kConstantZero;
    }
}

const char tfunction_separator[] = {
    '&', '*', 
    '+', '|', 
    '!', '\'', 
    '^', 
    '(', ')', ' ', '\t',
};

const FuncOpType tfunction_separator_op[] = {
    FuncOpType::kOP_AND, FuncOpType::kOP_AND, 
    FuncOpType::kOP_OR, FuncOpType::kOP_OR,
    FuncOpType::kOP_NOT, FuncOpType::kOP_NOT, 
    FuncOpType::kOP_XOR, 
    FuncOpType::kUnknown, FuncOpType::kUnknown, FuncOpType::kUnknown
};

const std::unordered_map<char,int> operator_precedence({
    {'+', 1}, {'|', 1},
    {'&', 2}, {'*', 2},
    {'^', 2}, 
    {'!', 3}, {'\'', 3},
    {'(',4}, {')', 0},
});



TFunction::TFunction()
    : TFunction::BaseType(),
      func_str_(0),
      op_(FuncOpType::kUnknown),
      tterm_(UNINIT_OBJECT_ID),
      left_(UNINIT_OBJECT_ID),
      right_(UNINIT_OBJECT_ID),
      group_id_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTFunction);
}

TFunction::~TFunction() {}

TFunction::TFunction(Object* owner, TFunction::IndexType id)
    : TFunction::BaseType(owner, id),
      func_str_(0),
      op_(FuncOpType::kUnknown),
      tterm_(UNINIT_OBJECT_ID),
      left_(UNINIT_OBJECT_ID),
      right_(UNINIT_OBJECT_ID),
      group_id_(UNINIT_OBJECT_ID) {
    setObjectType(ObjectType::kObjectTypeTFunction);
}

TFunction::TFunction(TFunction const& rhs) { copy(rhs); }

TFunction::TFunction(TFunction&& rhs) noexcept { move(std::move(rhs)); }

TFunction& TFunction::operator=(TFunction const& rhs) {
    if (this != &rhs) {
        copy(rhs);
    }
    return *this;
}

TFunction& TFunction::operator=(TFunction&& rhs) noexcept {
    if (this != &rhs) {
        move(std::move(rhs));
    }
    return *this;
}

void TFunction::print(std::ostream& stream) {}

void TFunction::copy(TFunction const& rhs) {
    this->BaseType::copy(rhs);

    func_str_ = rhs.func_str_;
    op_ = rhs.op_;
    tterm_ = rhs.tterm_;
    left_ = rhs.left_;
    right_ = rhs.right_;
    group_id_ = rhs.group_id_;
}

void TFunction::move(TFunction&& rhs) {
    this->BaseType::move(std::move(rhs));

    func_str_ = std::move(rhs.func_str_);
    op_ = std::move(rhs.op_);
    tterm_ = std::move(rhs.tterm_);
    left_ = std::move(rhs.left_);
    right_ = std::move(rhs.right_);
    group_id_ = std::move(rhs.group_id_);
}

TFunction::IndexType TFunction::memory() const {
    IndexType ret = this->BaseType::memory();

    ret += sizeof(func_str_);
    ret += sizeof(op_);
    ret += sizeof(tterm_);
    ret += sizeof(left_);
    ret += sizeof(right_);
    ret += sizeof(group_id_);

    return ret;
}

/// set
void TFunction::setFuncStr(const std::string& str) {
    Timing* timing_lib = getTimingLib();
    if (timing_lib != nullptr) {
        SymbolIndex idx = timing_lib->getOrCreateSymbol(str.c_str());
        if (idx != kInvalidSymbolIndex) {
            func_str_ = idx;
            timing_lib->addSymbolReference(func_str_, this->getId());
        }
    }
}
void TFunction::setTterm(ObjectId id) { tterm_ = id; }
void TFunction::setOp(FuncOpType t) { op_ = t; }
void TFunction::setLeft(ObjectId id) { left_ = id; }
void TFunction::setRight(ObjectId id) { right_ = id; }
void TFunction::setGroupId(ObjectId id) { group_id_ = id; }

/// get
TTerm* TFunction::getTterm(void) {
    if (tterm_ != UNINIT_OBJECT_ID)
        return addr<TTerm>(tterm_);
    else
        return nullptr;
}
FuncOpType TFunction::getOp(void) { return op_; }
TFunction* TFunction::getLeft(void) {
    if (left_ != UNINIT_OBJECT_ID)
        return addr<TFunction>(left_);
    else
        return nullptr;
}
TFunction* TFunction::getRight(void) {
    if (right_ != UNINIT_OBJECT_ID)
        return addr<TFunction>(right_);
    else
        return nullptr;
}
ObjectId TFunction::getGroupId(void) { return group_id_; }

TimingSense TFunction::getTimingSense(const TTerm* pTerm) {
    switch (op_) {
        case FuncOpType::kOP_ONE:
        case FuncOpType::kOP_ZERO: {
            return TimingSense::kNone;
        }
        case FuncOpType::kOP_OR:
        case FuncOpType::kOP_AND: {
            TimingSense ls = TimingSense::kUnknown;
            TimingSense rs = TimingSense::kUnknown;
            TFunction* pLeft = getLeft();
            TFunction* pRight = getRight();
            if (pLeft) ls = pLeft->getTimingSense(pTerm);
            if (pRight) rs = pRight->getTimingSense(pTerm);
            if (ls == rs)
                return ls;
            else if (ls == TimingSense::kNon_Unate ||
                     rs == TimingSense::kNon_Unate ||
                     (ls == TimingSense::kPositive_Unate &&
                      rs == TimingSense::kNegative_Unate) ||
                     (ls == TimingSense::kNegative_Unate &&
                      rs == TimingSense::kPositive_Unate))
                return TimingSense::kNon_Unate;
            else if (ls == TimingSense::kNone || ls == TimingSense::kUnknown)
                return rs;
            else if (rs == TimingSense::kNone || rs == TimingSense::kUnknown)
                return ls;
            else
                return TimingSense::kUnknown;
        }
        case FuncOpType::kOP_NOT: {
            TFunction* pLeft = getLeft();
            if (pLeft != nullptr) {
                switch (pLeft->getTimingSense(pTerm)) {
                    case TimingSense::kPositive_Unate:
                        return TimingSense::kNegative_Unate;
                    case TimingSense::kNegative_Unate:
                        return TimingSense::kPositive_Unate;
                    case TimingSense::kNon_Unate:
                        return TimingSense::kNon_Unate;
                    case TimingSense::kNone:
                        return TimingSense::kNone;
                    case TimingSense::kUnknown:
                        return TimingSense::kUnknown;
                }
            }
            return TimingSense::kUnknown;
        }
        case FuncOpType::kOP_XOR: {
            TimingSense ls = TimingSense::kUnknown;
            TimingSense rs = TimingSense::kUnknown;
            TFunction* pLeft = getLeft();
            TFunction* pRight = getRight();
            if (pLeft) ls = pLeft->getTimingSense(pTerm);
            if (pRight) rs = pRight->getTimingSense(pTerm);
            if (ls == TimingSense::kPositive_Unate ||
                ls == TimingSense::kNegative_Unate ||
                ls == TimingSense::kNon_Unate ||
                rs == TimingSense::kPositive_Unate ||
                rs == TimingSense::kNegative_Unate ||
                rs == TimingSense::kNon_Unate)
                return TimingSense::kNon_Unate;
            else
                return TimingSense::kUnknown;
        }
        case FuncOpType::kOP_TTERM: {
            if (pTerm == getTterm())
                return TimingSense::kPositive_Unate;
            else
                return TimingSense::kNone;
        }
    }
    return TimingSense::kUnknown;
}

OStreamBase& operator<<(OStreamBase& os, TFunction const& rhs) {
    os << DataTypeName(className(rhs)) << DataBegin("(");

    TFunction::BaseType const& base = rhs;
    os << base << DataDelimiter();

    os << DataFieldName("func_str_");
    {
        Timing* timing_lib = getTimingLib();
        if (timing_lib != nullptr && rhs.func_str_ != 0) {
            os << timing_lib->getSymbolByIndex(rhs.func_str_);
        } else {
            os << "";
        }
        os << DataDelimiter();
    }
    os << DataFieldName("op_") << rhs.op_ << DataDelimiter();
    os << DataFieldName("tterm_") << rhs.tterm_ << DataDelimiter();

    os << DataFieldName("left_") << rhs.left_;
    {
        os << DataBegin("[");
        if (rhs.left_ != UNINIT_OBJECT_ID) {
            auto p = Object::addr<TFunction>(rhs.left_);
            if (p) os << *p;
        }
        os << DataEnd("]") << DataDelimiter();
    }
    os << DataFieldName("right_") << rhs.right_;
    {
        os << DataBegin("[");
        if (rhs.right_ != UNINIT_OBJECT_ID) {
            auto p = Object::addr<TFunction>(rhs.right_);
            if (p) os << *p;
        }
        os << DataEnd("]") << DataDelimiter();
    }
    os << DataFieldName("group_id_") << rhs.group_id_;

    os << DataEnd(")");
    return os;
}

void
TFunction::tokenize(const std::string& func_str, std::vector<TFuncToken>& tokens) 
{
    tokens.clear();
    std::string substr = "";
    std::vector<TFuncToken> substr_tokens;
    for (int pos=0; pos<func_str.size(); pos++) {
        char token = func_str[pos];
        bool is_separator = false;
        int sep_index = 0;
        for (; sep_index<(sizeof(tfunction_separator)/sizeof(char)); sep_index++) {
            char sep_char = tfunction_separator[sep_index];
            if (token == sep_char) {
                is_separator = true;
                break;
            }
        }
        if (is_separator) {
            if (substr.length() > 0) {
                TFuncToken t;
                t.token = substr;
                t.is_operator = false;
                t.op = FuncOpType::kUnknown;
                substr_tokens.push_back(t);
                substr = "";
            }
            if (token != ' ' && token != '\t') {
                FuncOpType op = tfunction_separator_op[sep_index];
                TFuncToken t;
                t.token = std::string(1, token);
                t.is_operator = true;
                t.op = op;
                substr_tokens.push_back(t);
            }
        } else {
            substr += token;
        }
    }
    if (substr.length() > 0) {
        TFuncToken t;
        t.token = substr;
        t.is_operator = false;
        t.op = FuncOpType::kUnknown;
        substr_tokens.push_back(t);
        substr = "";
    }
    /*
     * Add missing AND operators back, and finalize token type (operator, oprand)
     * AND operators (&) are often omitted in the function string, need to add it back 
     * before/when constructing TFunction. Below are the situations that &s are omitted:
     * 1. "oprand oprand". E.g., "A B" should be "A&B"
     * 2. "oprand (right associative operators)". E.g., "A (B+C)" should "A&(B+C)", "A !B" should be "A&(!B)"
     * 3. ") oprand". E.g. "(A+B) C" should be "(A+B)&C"
     */
    for (int i=0; i<substr_tokens.size(); i++) {
        TFuncToken& t = substr_tokens[i];
        if (t.is_operator) {
            if (i == 0) {
                if (t.token != "(" && t.token != "!") {
                    printf("ERROR: Expecting parenthesis or ! operator or oprand at position %d of %s, got %c\n", i, func_str.c_str(), (t.token).c_str());
                } else {
                    tokens.push_back(t);
                }
            } else {
                TFuncToken& before = substr_tokens[i-1];
                if (!(before.is_operator) && (t.token == "(" || t.token == "!")) {
                    // The 2nd situation in above description
                    TFuncToken a;
                    a.token = "&";
                    a.is_operator = true;
                    a.op = FuncOpType::kOP_AND;
                    tokens.push_back(a);
                }
                tokens.push_back(t);
            }
        } else {
            FuncOpType op = FuncOpType::kOP_TTERM;
            if (t.token == "0") {
                op = FuncOpType::kOP_ZERO;
            } else if (t.token == "1") {
                op = FuncOpType::kOP_ONE;
            }
            if (i > 0) {
                TFuncToken& before = substr_tokens[i-1];
                if (before.is_operator && before.token == ")") {
                    // The 3rd situation
                    TFuncToken a;
                    a.token = "&";
                    a.is_operator = true;
                    a.op = FuncOpType::kOP_AND;
                    tokens.push_back(a);
                } else if (!(before.is_operator)) {
                    // The 1st situation
                    TFuncToken a;
                    a.token = "&";
                    a.is_operator = true;
                    a.op = FuncOpType::kOP_AND;
                    tokens.push_back(a);
                }
            }
            t.op = op;
            tokens.push_back(t);
        }
    }
}

ObjectId 
TFunction::newTFuncNode(FuncOpType op, std::string& op_char, ObjectId left, ObjectId right, ObjectId tterm)
{
    ObjectId nodeId = UNINIT_OBJECT_ID;
    Timing *timing_lib = getTimingLib();
    if (timing_lib) {
        auto p = Object::createObject<TFunction>(kObjectTypeTFunction,
                                                 timing_lib->getId());
        if (p) {
            p->setOwner(this);
            p->setGroupId(this->getId());
            nodeId = p->getId();
            p->setFuncStr(op_char);
            p->setLeft(left);
            p->setRight(right);
            p->setTterm(tterm);
        }
    }
    return nodeId;
}

ObjectId
TFunction::newOprandNode(TFuncToken& t, const TTerm* related_pin, str_omap_t<std::vector<tb_namespace::TTerm *>>& bus_pin_map)
{
    if (t.op == FuncOpType::kOP_ZERO) {
        std::string zero("0");
        return newTFuncNode(FuncOpType::kOP_ZERO, zero, UNINIT_OBJECT_ID, UNINIT_OBJECT_ID, UNINIT_OBJECT_ID);
    } else if (t.op == FuncOpType::kOP_ONE) {
        std::string one("1");
        return newTFuncNode(FuncOpType::kOP_ONE, one, UNINIT_OBJECT_ID, UNINIT_OBJECT_ID, UNINIT_OBJECT_ID);
    } else if (t.op == FuncOpType::kOP_TTERM) {
        TTerm* libpin = nullptr;
        if (related_pin != nullptr && (related_pin->isBus() || related_pin->isBundle())) {
            unsigned int bit = related_pin->busBit();
            std::vector<tb_namespace::TTerm*>& oprand_bus = bus_pin_map[t.token];
            libpin = oprand_bus[bit];
        } else {
            TCell* cell = nullptr;
            if (related_pin != nullptr) {
                cell = Object::addr<TCell>(related_pin->getGroupId());
            }
            else {
                auto p = Object::addr<TSequential>(getGroupId());
                if (p) cell = Object::addr<TCell>(p->getGroupId());
            }
            if (cell != nullptr)
                libpin = cell->getTerm(t.token);
        }
        if (libpin != nullptr) {
            ObjectId tterm_id = libpin->getId();
            return newTFuncNode(FuncOpType::kOP_TTERM, t.token, UNINIT_OBJECT_ID, UNINIT_OBJECT_ID, tterm_id);
        } else {
            return UNINIT_OBJECT_ID;
        }
    } else {
        // This function cannot handle other scenarios
        assert(true);
    }
    return UNINIT_OBJECT_ID;
}

int
TFunction::operatorPrecedence(const TFuncToken& t)
{
    if (!t.is_operator) {return -1;} 
    const std::string& op = t.token;
    char c = op[0];
    auto found = operator_precedence.find(c);
    if (found == operator_precedence.end()) {
        return -1;
    } else {
        return found->second;
    }
}

ObjectId
TFunction::buildExpressionTree(std::vector<TFuncToken>& tokens, const TTerm* related_pin, str_omap_t<std::vector<tb_namespace::TTerm *>>& bus_pin_map)
{
    std::stack<TFuncToken> token_stack;
    std::stack<ObjectId> node_stack;
    for (int i=0; i<tokens.size(); i++) {
        TFuncToken& t = tokens[i];
        if (t.token == "(") {
            token_stack.emplace(t);
        } else if (!(t.is_operator)) {
            ObjectId oprand = newOprandNode(t, related_pin, bus_pin_map);
            node_stack.push(oprand);
        } else if (t.is_operator && t.token != ")") {
            while (!token_stack.empty() && 
                token_stack.top().token != "(" && 
                operatorPrecedence(token_stack.top()) >= operatorPrecedence(t)) {
                TFuncToken op = token_stack.top();
                token_stack.pop();
                ObjectId oprand1 = node_stack.top();
                node_stack.pop();
                ObjectId new_node = UNINIT_OBJECT_ID;
                if (op.op == FuncOpType::kOP_NOT) {
                    new_node = newTFuncNode(op.op, op.token, oprand1, UNINIT_OBJECT_ID, UNINIT_OBJECT_ID);
                } else {
                    ObjectId oprand2 = node_stack.top();
                    node_stack.pop();
                    new_node = newTFuncNode(op.op, op.token, oprand1, oprand2, UNINIT_OBJECT_ID);
                }
                node_stack.push(new_node);
            }
            token_stack.push(t);
        } else if (t.token == ")") {
            while (!token_stack.empty() && token_stack.top().token != "(") {
                TFuncToken op = token_stack.top();
                token_stack.pop();
                ObjectId oprand1 = node_stack.top();
                node_stack.pop();
                ObjectId new_node = UNINIT_OBJECT_ID;
                if (op.op == FuncOpType::kOP_NOT) {
                    new_node = newTFuncNode(op.op, op.token, oprand1, UNINIT_OBJECT_ID, UNINIT_OBJECT_ID);
                } else {
                    ObjectId oprand2 = node_stack.top();
                    node_stack.pop();
                    new_node = newTFuncNode(op.op, op.token, oprand1, oprand2, UNINIT_OBJECT_ID);
                }
                node_stack.push(new_node);
            }
            token_stack.pop();
        } 
    }
    return node_stack.top();
}

void
TFunction::buildExpressionTree(const std::string& str, const TTerm* related_pin, str_omap_t<std::vector<tb_namespace::TTerm *>>& bus_pin_map)
{
    if (func_str_ != 0) {
        std::vector<TFuncToken> tokens;
        TFunction::tokenize(str, tokens);
        ObjectId root = buildExpressionTree(tokens, related_pin, bus_pin_map);
        TFunction* f = Object::addr<TFunction>(root);
        if (f != nullptr) {
            op_ = f->op_;
            left_ = f->left_;
            right_ = f->right_;
            tterm_ = f->tterm_;
        }
    }
}

void
TFunction::buildExpressionTree(const TTerm* related_pin, str_omap_t<std::vector<tb_namespace::TTerm *>>& bus_pin_map)
{
    Timing* timing_lib = getTimingLib();
    if (timing_lib != nullptr && func_str_ != 0) {
        std::string& func_str = timing_lib->getSymbolByIndex(func_str_);
        std::vector<TFuncToken> tokens;
        TFunction::tokenize(func_str, tokens);
        ObjectId root = buildExpressionTree(tokens, related_pin, bus_pin_map);
        TFunction* f = Object::addr<TFunction>(root);
        if (f != nullptr) {
            op_ = f->op_;
            left_ = f->left_;
            right_ = f->right_;
            tterm_ = f->tterm_;
        }
    }
}


TFuncConstant
TFunction::eval(std::unordered_map<TTerm*, TFuncConstant>& pin_values)
{
    TFunction* left = Object::addr<TFunction>(left_);
    TFunction* right = Object::addr<TFunction>(right_);
    switch (op_) {
        case FuncOpType::kOP_ONE: {
            return TFuncConstant::kConstantOne;
        }
        case FuncOpType::kOP_ZERO: {
            return TFuncConstant::kConstantZero;
        }
        case FuncOpType::kOP_TTERM: {
            TFuncConstant pin_value = TFuncConstant::kUnknown;
            TTerm* libpin = Object::addr<TTerm>(tterm_);
            auto found = pin_values.find(libpin);
            if (found != pin_values.end()) {
                pin_value = found->second;
            }
            return pin_value;
        }
        case FuncOpType::kOP_AND: {
            return left->eval(pin_values) & right->eval(pin_values);
        }
        case FuncOpType::kOP_NOT: {
            if (left) {
                return !(left->eval(pin_values));
            } else {
                return !(right->eval(pin_values));
            }
        }
        case FuncOpType::kOP_OR: {
            return left->eval(pin_values) + right->eval(pin_values);
        }
        case FuncOpType::kOP_XOR: {
            return left->eval(pin_values) ^ right->eval(pin_values);
        }
        case FuncOpType::kUnknown: {
            return TFuncConstant::kUnknown;
        }
        default: {
            // If the program gets here, means new FuncOpType memeber is added and this function need changes
            assert(true);
        }
    }
    return TFuncConstant::kUnknown;
}


}  // namespace db
}  // namespace open_edi
