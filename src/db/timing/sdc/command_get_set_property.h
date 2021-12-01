/**
 * @file command_get_set_property.h
 * @date 2020-11-25
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

#ifndef EDI_DB_TIMING_SDC_COMMAND_GET_SET_PROPERTY_H_
#define EDI_DB_TIMING_SDC_COMMAND_GET_SET_PROPERTY_H_

#define COMMAND_GET_SET_VAR(var, Var) \
    void set##Var (const decltype(var##_)& var) { var##_ = var; } \
    const decltype(var##_)& get##Var##Ref () const { return var##_; } \
    const decltype(var##_)& get##Var () const { return var##_; } \
    decltype(var##_)& get##Var () { return var##_; }

#define COMMAND_GET_SET_FLAG(var, Var) \
  private: \
    bool var##_ : 1; \
  public: \
    void set##Var () { var##_ = true; } \
    void reset##Var () { var##_ = false; } \
    const bool get##Var () const { return var##_; } \
    const bool is##Var () const { return var##_; }

#define COMMAND_BITSET_FLAG(flags, id, var, Var)  \
    void set##Var () { flags##_.set(id, 1); } \
    void reset##Var () { flags##_.set(id, 0); } \
    const bool get##Var () const { return flags##_.test(id); } \
    const bool is##Var () const { return flags##_.test(id); }

#define CONDITION_GET_SET_VAR(Var) \
    void set##Var (const decltype(condition_.get##Var##Ref())& var) { condition_.set##Var(var); } \
    const decltype(condition_.get##Var##Ref())& get##Var () const { return condition_.get##Var##Ref(); } \
    const decltype(condition_.get##Var##Ref())& get##Var##Ref () const { return condition_.get##Var##Ref(); } \
    decltype(condition_.get##Var())& get##Var () { return condition_.get##Var(); }

#define CONDITION_GET_SET_FLAG(Var) \
    void set##Var () { condition_.set##Var(); } \
    void reset##Var () { condition_.reset##Var(); } \
    const bool get##Var () const { return condition_.get##Var(); } \
    const bool is##Var () const { return condition_.is##Var(); }

#define SPLIT_ONE_GROUP(U, V, A1, A2) \
    void split(std::vector<V> &vec, V &ptr) { \
        vec.reserve(2); \
        vec.emplace_back(ptr); \
        if (!ptr->get##A1() and !ptr->get##A2()) { \
            ptr->set##A1(); \
            ptr->set##A2(); \
        } \
        if (ptr->get##A1() and ptr->get##A2()) { \
            auto deep_copy_ptr = std::make_shared<U>(*ptr); \
            ptr->reset##A2(); \
            deep_copy_ptr->reset##A1(); \
            vec.emplace_back(deep_copy_ptr); \
        } \
    }

#define SPLIT_TWO_GROUP(U, V, A1, A2, B1, B2) \
    void split(std::vector<V> &vec, V &ptr) { \
        vec.reserve(4); \
        vec.emplace_back(ptr); \
        if (!ptr->get##A1() and !ptr->get##A2()) { \
            ptr->set##A1(); \
            ptr->set##A2(); \
        } \
        if (!ptr->get##B1() and !ptr->get##B2()) { \
            ptr->set##B1(); \
            ptr->set##B2(); \
        } \
        if (ptr->get##A1() and ptr->get##A2()) { \
            auto deep_copy_ptr = std::make_shared<U>(*ptr); \
            ptr->reset##A2(); \
            deep_copy_ptr->reset##A1(); \
            vec.emplace_back(deep_copy_ptr); \
        } \
        if (ptr->get##B1() and ptr->get##B2()) { \
            for (auto &v : vec) { \
                auto deep_copy_ptr = std::make_shared<U>(*v); \
                v->reset##B1(); \
                deep_copy_ptr->reset##B2(); \
                vec.emplace_back(deep_copy_ptr); \
            } \
        } \
    }

#define SPLIT_THREE_GROUP(U, V, A1, A2, B1, B2, C1, C2) \
    void split(std::vector<V> &vec, V &ptr) { \
        vec.reserve(8); \
        vec.emplace_back(ptr); \
        if (!ptr->get##A1() and !ptr->get##A2()) { \
            ptr->set##A1(); \
            ptr->set##A2(); \
        } \
        if (!ptr->get##B1() and !ptr->get##B2()) { \
            ptr->set##B1(); \
            ptr->set##B2(); \
        } \
        if (!ptr->get##C1() and !ptr->get##C2()) { \
            ptr->set##C1(); \
            ptr->set##C2(); \
        } \
        if (ptr->get##A1() and ptr->get##A2()) { \
            auto deep_copy_ptr = std::make_shared<U>(*ptr); \
            ptr->reset##A2(); \
            deep_copy_ptr->reset##A1(); \
            vec.emplace_back(deep_copy_ptr); \
        } \
        if (ptr->get##B1() and ptr->get##B2()) { \
            for (auto &v : vec) { \
                auto deep_copy_ptr = std::make_shared<U>(*v); \
                v->reset##B1(); \
                deep_copy_ptr->reset##B2(); \
                vec.emplace_back(deep_copy_ptr); \
            } \
        } \
        if (ptr->get##C1() and ptr->get##C2()) { \
            for (auto &v : vec) { \
                auto deep_copy_ptr = std::make_shared<U>(*v); \
                v->reset##C1(); \
                deep_copy_ptr->reset##C2(); \
                vec.emplace_back(deep_copy_ptr); \
            } \
        } \
    }

#define SPLIT_FOUR_GROUP(U, V, A1, A2, B1, B2, C1, C2, D1, D2) \
    void split(std::vector<V> &vec, V &ptr) { \
        vec.reserve(16); \
        vec.emplace_back(ptr); \
        if (!ptr->get##A1() and !ptr->get##A2()) { \
            ptr->set##A1(); \
            ptr->set##A2(); \
        } \
        if (!ptr->get##B1() and !ptr->get##B2()) { \
            ptr->set##B1(); \
            ptr->set##B2(); \
        } \
        if (!ptr->get##C1() and !ptr->get##C2()) { \
            ptr->set##C1(); \
            ptr->set##C2(); \
        } \
        if (!ptr->get##D1() and !ptr->get##D2()) { \
            ptr->set##D1(); \
            ptr->set##D2(); \
        } \
        if (ptr->get##A1() and ptr->get##A2()) { \
            auto deep_copy_ptr = std::make_shared<U>(*ptr); \
            ptr->reset##A2(); \
            deep_copy_ptr->reset##A1(); \
            vec.emplace_back(deep_copy_ptr); \
        } \
        if (ptr->get##B1() and ptr->get##B2()) { \
            for (auto &v : vec) { \
                auto deep_copy_ptr = std::make_shared<U>(*v); \
                v->reset##B1(); \
                deep_copy_ptr->reset##B2(); \
                vec.emplace_back(deep_copy_ptr); \
            } \
        } \
        if (ptr->get##C1() and ptr->get##C2()) { \
            for (auto &v : vec) { \
                auto deep_copy_ptr = std::make_shared<U>(*v); \
                v->reset##C1(); \
                deep_copy_ptr->reset##C2(); \
                vec.emplace_back(deep_copy_ptr); \
            } \
        } \
        if (ptr->get##D1() and ptr->get##D2()) { \
            for (auto &v : vec) { \
                auto deep_copy_ptr = std::make_shared<U>(*v); \
                v->reset##D1(); \
                deep_copy_ptr->reset##D2(); \
                vec.emplace_back(deep_copy_ptr); \
            } \
        } \
    }

#endif
