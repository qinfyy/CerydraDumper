#include "pch.h"
#include "MonoAssembly.h"
#include "RuntimeType.h"

namespace Cerydra::CSharp
{
    Cerydra::IL2CPP::Class* MonoAssembly::AssemblyClass()
    {
        static auto* klass = RequireClass("System.Reflection.Assembly");
        return klass;
    }

    Cerydra::IL2CPP::Class* MonoAssembly::AssemblyNameClass()
    {
        static auto* klass = RequireClass("System.Reflection.AssemblyName");
        return klass;
    }

    Cerydra::IL2CPP::Method* MonoAssembly::GetNameMethod()
    {
        static auto* method = RequireMethod(AssemblyClass(), "GetName", {});
        return method;
    }

    Cerydra::IL2CPP::Method* MonoAssembly::AssemblyNameGetNameMethod()
    {
        static auto* method = RequireMethod(AssemblyNameClass(), "get_Name", {});
        return method;
    }

    Cerydra::IL2CPP::Method* MonoAssembly::GetTypesMethod()
    {
        static auto* method = RequireMethod(AssemblyClass(), "GetTypes", { "bool" });
        return method;
    }

    Cerydra::IL2CPP::Method* MonoAssembly::GetTypeMethod()
    {
        static auto* method = RequireMethod(AssemblyClass(), "GetType", { "string" });
        return method;
    }

    SystemString MonoAssembly::GetName() const
    {
        auto assemblyName = InvokeInstance<uintptr_t>(ptr, GetNameMethod());
        return InvokeInstance<SystemString>(assemblyName, AssemblyNameGetNameMethod());
    }

    ArrayObject MonoAssembly::GetTypes(bool exportedOnly) const
    {
        return InvokeInstance<ArrayObject, bool>(ptr, GetTypesMethod(), exportedOnly);
    }

    RuntimeType MonoAssembly::GetTypeByName(SystemString name) const
    {
        return InvokeInstance<RuntimeType>(ptr, GetTypeMethod(), name);
    }
}
