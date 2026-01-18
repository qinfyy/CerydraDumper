#pragma once
#include "Il2CppApiWrapper.h"
#include "Il2CppCache.h"
#include <stdexcept>
#include <sstream>

#define FN_ARGS(...) { __VA_ARGS__ }

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

template<typename RetOrWrapper, typename... Args>
RetOrWrapper InvokeIl2CppInstanceObjectMethod(uintptr_t instance,
    const char* class_name,
    const char* method_name,
    const std::vector<std::string>& arg_types,
    Args... args)
{
    // 获取类
    auto klass = GetCachedClass(class_name);
    if (klass->is_null()) {
        throw std::runtime_error("No such class: " + std::string(class_name));
    }

    // 获取方法
    auto method_info = klass->find_method(method_name, arg_types);
    if (method_info.is_null()) {
        throw std::runtime_error("No such method: " + std::string(method_name));
    }

    auto function_va = method_info.va();
    if (!function_va) {
        std::ostringstream msg;
        msg << "Method VA is null! Cannot call method: " << method_name
            << " | class: " << class_name
            << " | instance ptr: " << std::hex << instance;
        throw std::runtime_error(msg.str());
    }

    // 原生函数指针
    using Fn = uintptr_t(__fastcall*)(uintptr_t, Args...);
    auto func = reinterpret_cast<Fn>(function_va);

    try {
        uintptr_t ret = func(instance, args...);

        if constexpr (std::is_base_of_v<CIl2CppWrapBase, RetOrWrapper>) {
            return RetOrWrapper(ret);
        }
        else if constexpr (std::is_convertible_v<uintptr_t, RetOrWrapper>) {
            return static_cast<RetOrWrapper>(ret);
        }
        else if constexpr (std::is_same_v<RetOrWrapper, void>) {
            (void)ret;
        }
        else {
            static_assert(std::is_base_of_v<CIl2CppWrapBase, RetOrWrapper> || std::is_convertible_v<uintptr_t, RetOrWrapper>,
                "InvokeIl2CppInstanceObjectMethod: RetOrWrapper 必须是源自 CIl2CppWrapBase 的类，或者能够从 uintptr_t 类型转换而来。");
        }
    }
    catch (...) {
        std::ostringstream msg;
        msg << "Unknown exception in method: " + std::string(method_name) << " | class: " << class_name
            << " | instance ptr: " << std::hex << instance
            << " | method RVA: " << method_info.rva();
        throw std::runtime_error(msg.str());
    }
}

template<typename RetOrWrapper, typename... Args>
RetOrWrapper InvokeIl2CppInstanceObjectMethodDynamic(uintptr_t instance,
    const char* method_name,
    const std::vector<std::string>& arg_types,
    Args... args)
{
    auto obj_class = CIl2CppObject(instance).get_class();
    if (obj_class.is_null())
        throw std::runtime_error("Instance class is null!");

    auto method_info = obj_class.find_method(method_name, arg_types);
    auto function_va = method_info.va();
    if (!function_va)
        throw std::runtime_error("Method VA is null!");

    using Fn = uintptr_t(__fastcall*)(uintptr_t, Args...);
    auto func = reinterpret_cast<Fn>(function_va);

    uintptr_t ret = func(instance, args...);
    if constexpr (std::is_base_of_v<CIl2CppWrapBase, RetOrWrapper>)
        return RetOrWrapper(ret);
    else
        return static_cast<RetOrWrapper>(ret);
}

