#pragma once

#include "Il2CppRuntimeCache.h"
#include "RuntimeObject.h"
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
    struct Il2CppException
    {
        void* klass;
        void* monitor;
        Il2CppString* className;
        Il2CppString* message;
        Il2CppObject* data;
        Il2CppException* innerException;
    };

    struct Il2CppExceptionWrapper
    {
        Il2CppException* ex;
        explicit Il2CppExceptionWrapper(Il2CppException* value) : ex(value) {}
    };

    Cerydra::IL2CPP::Class* RequireClass(const char* className);
    Cerydra::IL2CPP::Method* RequireMethod(
        Cerydra::IL2CPP::Class* klass,
        const char* methodName,
        const std::vector<std::string>& argTypes);
    Cerydra::IL2CPP::Method* RequireDynamicMethod(
        uintptr_t instance,
        const char* methodName,
        const std::vector<std::string>& argTypes);

    template <typename Ret, typename NativeRet>
    Ret WrapReturn(NativeRet value)
    {
        if constexpr (std::is_base_of_v<ObjectRef, Ret>) {
            return Ret(static_cast<uintptr_t>(value));
        }
        else {
            return static_cast<Ret>(value);
        }
    }

    template <bool isStatic, typename Ret, typename... Args>
    Ret InvokeCachedInternal(uintptr_t thisPtr, Cerydra::IL2CPP::Method* method, Args... args)
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
                << " | this ptr: 0x" << std::hex << thisPtr;
            throw std::runtime_error(msg.str());
        }

        try {
            if constexpr (isStatic) {
                using NativeRet = std::conditional_t<std::is_base_of_v<ObjectRef, Ret>, uintptr_t, Ret>;
                using FnType = std::conditional_t<
                    std::is_same_v<Ret, void>,
                    void(__fastcall*)(Args...),
                    NativeRet(__fastcall*)(Args...)
                >;

                auto func = reinterpret_cast<FnType>(functionVa);
                if constexpr (std::is_same_v<Ret, void>) {
                    func(args...);
                    return;
                }
                else {
                    return WrapReturn<Ret>(func(args...));
                }
            }
            else {
                using NativeRet = std::conditional_t<std::is_base_of_v<ObjectRef, Ret>, uintptr_t, Ret>;
                using FnType = std::conditional_t<
                    std::is_same_v<Ret, void>,
                    void(__fastcall*)(uintptr_t, Args...),
                    NativeRet(__fastcall*)(uintptr_t, Args...)
                >;

                auto func = reinterpret_cast<FnType>(functionVa);
                if constexpr (std::is_same_v<Ret, void>) {
                    func(thisPtr, args...);
                    return;
                }
                else {
                    return WrapReturn<Ret>(func(thisPtr, args...));
                }
            }
        }
        catch (Il2CppExceptionWrapper& e) {
            std::ostringstream msg;
            msg << "IL2CPP 方法抛出异常: " << methodName
                << " | class: " << className
                << " | this ptr: 0x" << std::hex << thisPtr
                << " | method RVA: 0x" << method->Rva();

            if (e.ex && e.ex->message) {
                msg << " | 异常消息: " << Il2CppStringToAnsiString(e.ex->message);
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
                << " | this ptr: 0x" << std::hex << thisPtr
                << " | method RVA: 0x" << method->Rva();
            throw std::runtime_error(msg.str());
        }
    }

    template <typename Ret, typename... Args>
    Ret InvokeStatic(Cerydra::IL2CPP::Method* method, Args... args)
    {
        return InvokeCachedInternal<true, Ret>(0, method, args...);
    }

    template <typename Ret, typename... Args>
    Ret InvokeInstance(uintptr_t instance, Cerydra::IL2CPP::Method* method, Args... args)
    {
        return InvokeCachedInternal<false, Ret>(instance, method, args...);
    }

    template <typename Ret, typename... Args>
    Ret InvokeDynamic(
        uintptr_t instance,
        const char* methodName,
        const std::vector<std::string>& argTypes,
        Args... args)
    {
        return InvokeInstance<Ret>(instance, RequireDynamicMethod(instance, methodName, argTypes), args...);
    }
}
