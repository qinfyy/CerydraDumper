#pragma once
#include "Il2CppApiWrapper.h"
#include "Il2CppCache.h"
#include <stdexcept>

#define CS_CLASS(cs_name_literal) \
public: \
    static inline CIl2CppClass GetClass() \
    { \
        auto klass = GetCachedClass(cs_name_literal); \
        if (klass->is_null()) \
            throw std::runtime_error("No such class: " cs_name_literal); \
        return *klass; \
    } \
    __declspec(noinline) inline CIl2CppObject AsObject() const \
    { \
        return CIl2CppObject(ptr); \
    }

#define CS_METHOD_STATIC(fn_name, method_name, fnArgs, ret_type, args_decl, args_name) \
inline static ret_type fn_name args_decl { \
    auto klass = GetClass(); \
    auto method_info = klass.find_method(method_name, fnArgs); \
    if (method_info.is_null()) { \
        throw std::runtime_error(std::string("No such static method: ") + method_name); \
    } \
    auto func = reinterpret_cast<ret_type(__fastcall*) args_decl>(method_info.va()); \
    try { \
        auto result = func args_name; \
        return result; \
    } catch (...) { \
        throw std::runtime_error(std::string("Exception in static method: ") + method_name); \
    } \
}

#define CS_METHOD_STATIC_AUTO_CTOR(fn_name, method_name, fnArgs, ret_type, args_decl, args_name) \
inline static ret_type fn_name args_decl { \
    auto klass = GetClass(); \
    auto method_info = klass.find_method(method_name, fnArgs); \
    if (method_info.is_null()) { \
        throw std::runtime_error(std::string("No such static method: ") + method_name); \
    } \
    auto func = reinterpret_cast<uintptr_t(__fastcall*) args_decl>(method_info.va()); \
    try { \
        auto result = func args_name; \
        return ret_type(result); \
    } catch (...) { \
        throw std::runtime_error(std::string("Exception in static method: ") + method_name); \
    } \
}

#define CS_METHOD_CUSTOM(fn_name, method_name, fnArgs, class_name_literal, ret_type, args_decl, args_name) \
inline ret_type fn_name args_decl { \
    auto klass = GetCachedClass(class_name_literal); \
    if (klass.is_null()) { \
        throw std::runtime_error(std::string("No such class: ") + class_name_literal); \
    } \
    auto method_info = klass.find_method(method_name, fnArgs); \
    if (method_info.is_null()) { \
        throw std::runtime_error(std::string("No such method: ") + method_name); \
    } \
    auto func = reinterpret_cast<ret_type(__fastcall*)(uintptr_t, args_decl)> (method_info.va()); \
    try { \
        return func(ptr, args_name); \
    } catch (...) { \
        throw std::runtime_error(std::string("Exception in method: ") + method_name); \
    } \
}
