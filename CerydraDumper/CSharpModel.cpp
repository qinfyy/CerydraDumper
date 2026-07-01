#include "pch.h"
#include "CSharpModel.h"

namespace Cerydra::CSharp
{
    Cerydra::IL2CPP::Class* RequireClass(const char* className)
    {
        auto klass = GetCachedClassMeta(className);
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

        auto method = klass->GetMethod(methodName, argTypes);
        if (!method) {
            throw std::runtime_error(
                "找不到 C# 方法: " + klass->fullName + "::" + methodName);
        }

        return method;
    }

    Cerydra::IL2CPP::Method* RequireDynamicMethod(
        uintptr_t instance,
        const char* methodName,
        const std::vector<std::string>& argTypes)
    {
        if (!instance) {
            throw std::runtime_error(std::string("对象为空，无法动态查找方法: ") + methodName);
        }

        auto nativeClass = CIl2CppObject(instance).get_class();
        auto klass = GetCachedClassMeta(nativeClass.raw_ptr());
        if (!klass) {
            throw std::runtime_error(std::string("找不到对象运行时类，无法动态查找方法: ") + methodName);
        }

        return RequireMethod(klass, methodName, argTypes);
    }
}
