#pragma once

#include "Il2CppApiWrapper.h"
#include "Il2CppCache.h"
#include "Util.h"
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

void InitSehTranslator();

namespace Cerydra::CSharp
{
    Cerydra::IL2CPP::Class* RequireClass(const char* className);
    Cerydra::IL2CPP::Method* RequireMethod(
        Cerydra::IL2CPP::Class* klass,
        const char* methodName,
        const std::vector<std::string>& argTypes);
    Cerydra::IL2CPP::Method* RequireDynamicMethod(
        uintptr_t instance,
        const char* methodName,
        const std::vector<std::string>& argTypes);

    class ObjectRef
    {
    protected:
        uintptr_t ptr{};

    public:
        ObjectRef() = default;
        explicit ObjectRef(uintptr_t value) : ptr(value) {}

        explicit operator bool() const { return ptr != 0; }
        bool is_null() const { return ptr == 0; }
        uintptr_t raw_ptr() const { return ptr; }
    };

    template <bool isStatic, typename RetOrWrapper, typename... Args>
    RetOrWrapper InvokeCachedInternal(uintptr_t thisPtr, Cerydra::IL2CPP::Method* method, Args... args)
    {
        InitSehTranslator();

        const auto className = method && method->klass ? method->klass->fullName : std::string("<unknown>");
        const auto methodName = method ? method->name : std::string("<null>");

        if (!method) {
            throw std::runtime_error("方法缓存为空，无法调用");
        }

        const auto functionVa = method->Va();
        if (!functionVa) {
            std::ostringstream msg;
            msg << "方法地址为空，无法调用: " << methodName
                << " | class: " << className
                << " | this ptr: " << std::hex << thisPtr;
            throw std::runtime_error(msg.str());
        }

        try {
            if constexpr (isStatic) {
                using FnType = std::conditional_t<
                    std::is_base_of_v<CIl2CppWrapBase, RetOrWrapper>,
                    uintptr_t(__fastcall*)(Args...),
                    std::conditional_t<
                        std::is_same_v<RetOrWrapper, void>,
                        void(__fastcall*)(Args...),
                        RetOrWrapper(__fastcall*)(Args...)
                    >
                >;

                auto func = reinterpret_cast<FnType>(functionVa);
                if constexpr (std::is_base_of_v<CIl2CppWrapBase, RetOrWrapper>) {
                    return RetOrWrapper(func(args...));
                }
                else if constexpr (std::is_same_v<RetOrWrapper, void>) {
                    func(args...);
                    return;
                }
                else {
                    return func(args...);
                }
            }
            else {
                using FnType = std::conditional_t<
                    std::is_base_of_v<CIl2CppWrapBase, RetOrWrapper>,
                    uintptr_t(__fastcall*)(uintptr_t, Args...),
                    std::conditional_t<
                        std::is_same_v<RetOrWrapper, void>,
                        void(__fastcall*)(uintptr_t, Args...),
                        RetOrWrapper(__fastcall*)(uintptr_t, Args...)
                    >
                >;

                auto func = reinterpret_cast<FnType>(functionVa);
                if constexpr (std::is_base_of_v<CIl2CppWrapBase, RetOrWrapper>) {
                    return RetOrWrapper(func(thisPtr, args...));
                }
                else if constexpr (std::is_same_v<RetOrWrapper, void>) {
                    func(thisPtr, args...);
                    return;
                }
                else {
                    return func(thisPtr, args...);
                }
            }
        }
        catch (Il2CppExceptionWrapper& e) {
            std::ostringstream msg;
            msg << "IL2CPP 方法抛出异常: " << methodName
                << " | class: " << className
                << " | this ptr: " << std::hex << thisPtr
                << " | method RVA: " << method->Rva();

            Il2CppException* ex = e.ex;
            if (ex && ex->message) {
                msg << " | 异常消息: " << Il2CppStringToAnsiString(ex->message);
            }
            else {
                msg << " | 异常消息: Unknown";
            }
            throw std::runtime_error(msg.str());
        }
        catch (...) {
            std::ostringstream msg;
            msg << "调用 IL2CPP 方法时出现未知异常: " << methodName
                << " | class: " << className
                << " | this ptr: " << std::hex << thisPtr
                << " | method RVA: " << method->Rva();
            throw std::runtime_error(msg.str());
        }
    }

    template <typename RetOrWrapper, typename... Args>
    RetOrWrapper InvokeStatic(Cerydra::IL2CPP::Method* method, Args... args)
    {
        return InvokeCachedInternal<true, RetOrWrapper>(0, method, args...);
    }

    template <typename RetOrWrapper, typename... Args>
    RetOrWrapper InvokeInstance(uintptr_t instance, Cerydra::IL2CPP::Method* method, Args... args)
    {
        return InvokeCachedInternal<false, RetOrWrapper>(instance, method, args...);
    }

    template <typename RetOrWrapper, typename... Args>
    RetOrWrapper InvokeDynamic(
        uintptr_t instance,
        const char* methodName,
        const std::vector<std::string>& argTypes,
        Args... args)
    {
        return InvokeInstance<RetOrWrapper>(instance, RequireDynamicMethod(instance, methodName, argTypes), args...);
    }
}
