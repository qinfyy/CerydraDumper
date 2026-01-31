#pragma once
#include "Il2CppApiWrapper.h"
#include "Il2CppCache.h"
#include <stdexcept>
#include <sstream>

#define FN_ARGS(...) { __VA_ARGS__ }

#define CS_CLASS(cs_name_literal) \
public: \
    static CIl2CppClass GetClass() \
    { \
        auto klass = GetCachedClass(cs_name_literal); \
        if (klass->is_null()) \
            throw std::runtime_error("No such class: " cs_name_literal); \
        return *klass; \
    } \
    CIl2CppObject AsObject() const \
    { \
        return CIl2CppObject(ptr); \
    }

#define CS_METHOD_STATIC(fn_name, method_name, fnArgs, ret_type, args_decl, args_name) \
static ret_type fn_name args_decl { \
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
static ret_type fn_name args_decl { \
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

void InitSehTranslator();

template<typename RetOrWrapper, typename... Args>
RetOrWrapper CallIl2CppInstanceObjectMethod(uintptr_t instance,
    const char* class_name,
    const char* method_name,
    const std::vector<std::string>& arg_types,
    Args... args)
{
    InitSehTranslator();

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

    // 这tm谁来了都看不懂啊
    using FnType = std::conditional_t<
        std::is_base_of_v<CIl2CppWrapBase, RetOrWrapper>,
        uintptr_t(__fastcall*)(uintptr_t, Args...),
        std::conditional_t<
            std::is_same_v<RetOrWrapper, void>,
            void(__fastcall*)(uintptr_t, Args...),
            RetOrWrapper(__fastcall*)(uintptr_t, Args...)
        >
    >;

    auto func = reinterpret_cast<FnType>(function_va);
    auto func_raw = reinterpret_cast<void*>(function_va);

    try {
        if constexpr (std::is_base_of_v<CIl2CppWrapBase, RetOrWrapper>) {
            auto func = reinterpret_cast<uintptr_t(__fastcall*)(uintptr_t, Args...)>(func_raw);
            uintptr_t ret = func(instance, args...);
            return RetOrWrapper(ret); // wrap
        }
        else if constexpr (std::is_same_v<RetOrWrapper, void>) {
            auto func = reinterpret_cast<void(__fastcall*)(uintptr_t, Args...)>(func_raw);
            func(instance, args...);
            return;
        }
        else {
            auto func = reinterpret_cast<RetOrWrapper(__fastcall*)(uintptr_t, Args...)>(func_raw);
            return func(instance, args...);
        }
    }
    catch (...) {
        std::ostringstream msg;
        msg << "Unknown exception in method: " << method_name
            << " | class: " << class_name
            << " | instance ptr: " << std::hex << instance
            << " | method RVA: " << method_info.rva();
        throw std::runtime_error(msg.str());
    }
}

template<typename RetOrWrapper, typename... Args>
RetOrWrapper CallIl2CppInstanceObjectMethodDynamic(uintptr_t instance,
    const char* method_name,
    const std::vector<std::string>& arg_types,
    Args... args)
{
    InitSehTranslator();

    auto obj_class = CIl2CppObject(instance).get_class();
	auto class_name = (obj_class.is_null()) ? "<null>" : obj_class.name();
    if (obj_class.is_null()) {
        throw std::runtime_error("Instance class is null!");
    }

    auto method_info = obj_class.find_method(method_name, arg_types);
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

    using FnType = std::conditional_t<
        std::is_base_of_v<CIl2CppWrapBase, RetOrWrapper>,
        uintptr_t(__fastcall*)(uintptr_t, Args...),
        std::conditional_t<
            std::is_same_v<RetOrWrapper, void>,
            void(__fastcall*)(uintptr_t, Args...),
            RetOrWrapper(__fastcall*)(uintptr_t, Args...)
        >
    >;

    auto func = reinterpret_cast<FnType>(function_va);

    try {
        if constexpr (std::is_base_of_v<CIl2CppWrapBase, RetOrWrapper>) {
            uintptr_t ret = func(instance, args...);
            return RetOrWrapper(ret);
        }
        else if constexpr (std::is_same_v<RetOrWrapper, void>) {
            func(instance, args...);
            return;
        }
        else {
            return func(instance, args...);
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

// 用于实例字段
#define CS_FIELD_INSTANCE(fn_name, field_name_literal, ConverterType) \
ConverterType fn_name() const { \
    if (!ptr) \
        throw std::runtime_error("Object is null! Cannot access field " field_name_literal); \
    auto obj_class = CRuntimeType::FromClass(GetClass()); \
    auto field_info = obj_class.GetField(field_name_literal); \
    if (!field_info) throw std::runtime_error("No such field: " field_name_literal); \
    auto value_obj = field_info.get_value_object(CIl2CppObject(ptr)); \
    if (!value_obj) throw std::runtime_error("Field " field_name_literal " is null"); \
    return ConverterType(value_obj.raw_ptr()); \
}

// 用于静态字段
#define CS_FIELD_STATIC(fn_name, field_name_literal, ConverterType) \
static ConverterType fn_name() { \
    auto klass = GetClass(); \
    auto field_info = klass.get_field(field_name_literal); \
    if (!field_info) throw std::runtime_error("No such static field: " field_name_literal); \
    auto value_obj = field_info.get_value_object(CIl2CppObject(0)); \
    if (!value_obj) throw std::runtime_error("Static field " field_name_literal " is null"); \
    return ConverterType(value_obj.raw_ptr()); \
}