#pragma once
#include "Il2CppApiWrapper.h"
#include "Il2CppCache.h"
#include <stdexcept>
#include <sstream>
#include "Util.h"

void InitSehTranslator();

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

template<bool isStatic, typename RetOrWrapper, typename... Args>
RetOrWrapper CallIl2CppInternal(
	uintptr_t thisPtr, // 静态方法会忽略此参数
    CIl2CppClass* klass,
    const char* method_name,
    const std::vector<std::string>& arg_types,
    Args... args)
{
    if (!klass || klass->is_null()) {
        std::ostringstream msg;
        msg << "No such class | method: " << method_name
            << " | this ptr: " << std::hex << thisPtr;
        throw std::runtime_error(msg.str());
    }

    auto class_name = klass->name();
    auto class_meta = GetCachedClassMeta(klass->raw_ptr());
    auto method_meta = class_meta ? class_meta->GetMethod(method_name, arg_types) : nullptr;
    auto method_info = method_meta ? CIl2CppMethod(reinterpret_cast<uintptr_t>(method_meta->address)) : klass->find_method(method_name, arg_types);

    if (!method_meta && !method_info.is_null()) {
        method_meta = GetCachedMethodMeta(method_info.raw_ptr());
    }

    if (method_info.is_null() && !method_meta) {
        std::ostringstream msg;
        msg << "No such method: " << method_name
            << " | class: " << class_name
            << " | this ptr: " << std::hex << thisPtr;
        throw std::runtime_error(msg.str());
    }

    auto function_va = method_meta ? method_meta->Va() : method_info.va();
    if (!function_va) {
        std::ostringstream msg;
        msg << "Method VA is null! Cannot call method: " << method_name
            << " | class: " << class_name
            << " | this ptr: " << std::hex << thisPtr;
        throw std::runtime_error(msg.str());
    }

    try {
        if constexpr (isStatic) {
            // 静态函数
            using FnType = std::conditional_t<
                std::is_base_of_v<CIl2CppWrapBase, RetOrWrapper>,
                uintptr_t(__fastcall*)(Args...),
                std::conditional_t<
                    std::is_same_v<RetOrWrapper, void>,
                    void(__fastcall*)(Args...),
                    RetOrWrapper(__fastcall*)(Args...)
                >
            >;

            auto func = reinterpret_cast<FnType>(function_va);

            if constexpr (std::is_base_of_v<CIl2CppWrapBase, RetOrWrapper>) {
                uintptr_t result = func(args...);
                return RetOrWrapper(result);
            }
            else if constexpr (std::is_same_v<RetOrWrapper, void>) {
                func(args...);
                return;
            }
            else {
                auto result = func(args...);
                return result;
            }
        }
        else {
            // 动态函数
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

            if constexpr (std::is_base_of_v<CIl2CppWrapBase, RetOrWrapper>) {
                uintptr_t result = func(thisPtr, args...);
                return RetOrWrapper(result);
            }
            else if constexpr (std::is_same_v<RetOrWrapper, void>) {
                func(thisPtr, args...);
                return;
            }
            else {
                auto result = func(thisPtr, args...);
                return result;
            }
        }
    }
    catch (Il2CppExceptionWrapper& e)
    {
        std::ostringstream msg;
        msg << "Il2CppExceptionWrapper in method: " << method_name
            << " | class: " << class_name
            << " | this ptr: " << std::hex << thisPtr
            << " | method RVA: " << (method_meta ? method_meta->Rva() : method_info.rva());

        Il2CppException* ex = e.ex;
        if (ex->message)
        {
            std::string exceptionMessage = Il2CppStringToAnsiString(ex->message);
            msg << " | Exception Message: " << exceptionMessage;
            throw std::runtime_error(msg.str());
        }

        msg << " | Exception Message: Unknown";
        throw std::runtime_error(msg.str());
    }
    catch (...) {
        std::ostringstream msg;
        msg << "Unknown exception in method: " << method_name
            << " | class: " << class_name
            << " | this ptr: " << std::hex << thisPtr
            << " | method RVA: " << (method_meta ? method_meta->Rva() : method_info.rva());
        throw std::runtime_error(msg.str());
    }
}

template<typename RetOrWrapper, typename... Args>
RetOrWrapper CallIl2CppStaticMethodInternal(
    CIl2CppClass klass,
    const char* method_name,
    const std::vector<std::string>& arg_types,
    Args... args)
{
    InitSehTranslator();
    return CallIl2CppInternal<true, RetOrWrapper>(0, &klass, method_name, arg_types, args...);
}

template<typename RetOrWrapper, typename... Args>
RetOrWrapper CallIl2CppStaticMethod(
    const char* class_name,
    const char* method_name,
    const std::vector<std::string>& arg_types,
    Args... args)
{
    InitSehTranslator();
    auto klass = GetCachedClass(class_name);

	// 解引用需要检查空指针
    if (!klass || klass->is_null()) {
        std::ostringstream msg;
        msg << "No such class | static method: " << method_name;
        throw std::runtime_error(msg.str());
    }

    return CallIl2CppStaticMethodInternal<RetOrWrapper>(*klass, method_name, arg_types, args...);
}

template<typename RetOrWrapper, typename... Args>
RetOrWrapper CallIl2CppInstanceObjectMethod(
    uintptr_t instance,
    const char* class_name,
    const char* method_name,
    const std::vector<std::string>& arg_types,
    Args... args)
{
    InitSehTranslator();
    auto klass = GetCachedClass(class_name);
    return CallIl2CppInternal<false, RetOrWrapper>(instance, klass, method_name, arg_types, args...);
}

template<typename RetOrWrapper, typename... Args>
RetOrWrapper CallIl2CppInstanceObjectMethodDynamic(
    uintptr_t instance,
    const char* method_name,
    const std::vector<std::string>& arg_types,
    Args... args)
{
    InitSehTranslator();
    auto klass = CIl2CppObject(instance).get_class();
    return CallIl2CppInternal<false, RetOrWrapper>(instance, &klass, method_name, arg_types, args...);
}

#define CS_METHOD_STATIC_NOARGS(fn_name, method_name, fnArgs, ret_type) \
static ret_type fn_name() { \
    return CallIl2CppStaticMethodInternal<ret_type>(GetClass(), method_name, fnArgs); \
}

#define CS_METHOD_STATIC(fn_name, method_name, fnArgs, ret_type, args_decl, args_name) \
static ret_type fn_name args_decl { \
    return CallIl2CppStaticMethodInternal<ret_type>(GetClass(), method_name, fnArgs, args_name); \
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
