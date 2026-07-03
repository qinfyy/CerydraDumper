#include "pch.h"
#include "CSharpInvoke.h"
#include <eh.h>
#include <mutex>

namespace Cerydra::CSharp
{
    namespace
    {
        void SehTranslator(unsigned int code, _EXCEPTION_POINTERS* ep)
        {
            std::ostringstream msg;
            msg << "SEH 异常: 0x" << std::hex << code
                << " | address: " << ep->ExceptionRecord->ExceptionAddress;
            throw std::runtime_error(msg.str());
        }
    }

    void InstallSehTranslator()
    {
        static std::once_flag flag;
        std::call_once(flag, [] {
            _set_se_translator(SehTranslator);
        });
    }

    Cerydra::IL2CPP::Class* RequireClass(const char* className)
    {
        auto* klass = Il2CppRuntimeCache::GetClass(className);
        if (!klass) {
            throw std::runtime_error(std::string("找不到 C# 类: ") + className);
        }

        return klass;
    }

    Cerydra::IL2CPP::Method* RequireMethod(
        Cerydra::IL2CPP::Class* klass,
        const char* methodName,
        const std::vector<std::string>& argTypes)
    {
        if (!klass) {
            throw std::runtime_error(std::string("类缓存为空，无法查找方法: ") + methodName);
        }

        auto* method = klass->GetMethod(methodName, argTypes);
        if (!method) {
            throw std::runtime_error("找不到 C# 方法: " + klass->fullName + "::" + methodName);
        }

        return method;
    }

    Cerydra::IL2CPP::Method* RequireDynamicMethod(
        const void* instance,
        const char* methodName,
        const std::vector<std::string>& argTypes)
    {
        if (!instance) {
            throw std::runtime_error(std::string("对象为空，无法动态查找方法: ") + methodName);
        }

        auto* nativeClass = *reinterpret_cast<Il2CppClass* const*>(instance);
        auto* klass = Il2CppRuntimeCache::GetClassByAddress(reinterpret_cast<uintptr_t>(nativeClass));
        if (!klass) {
            throw std::runtime_error(std::string("找不到对象运行时类，无法动态查找方法: ") + methodName);
        }

        return RequireMethod(klass, methodName, argTypes);
    }
}

void InitSehTranslator()
{
    Cerydra::CSharp::InstallSehTranslator();
}
