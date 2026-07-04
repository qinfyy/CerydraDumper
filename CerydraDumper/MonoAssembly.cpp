#include "pch.h"
#include "MonoAssembly.h"
#include "RuntimeType.h"

namespace Cerydra::CSharp
{
    Cerydra::Il2Cpp::Class* MonoAssembly::AssemblyClass()
    {
        static auto* klass = RequireClass("System.Reflection.Assembly");
        return klass;
    }

    Cerydra::Il2Cpp::Class* MonoAssembly::AssemblyNameClass()
    {
        static auto* klass = RequireClass("System.Reflection.AssemblyName");
        return klass;
    }

    Cerydra::Il2Cpp::Method* MonoAssembly::GetNameMethod()
    {
        static auto* method = RequireMethod(AssemblyClass(), "GetName", {});
        return method;
    }

    Cerydra::Il2Cpp::Method* MonoAssembly::AssemblyNameGetNameMethod()
    {
        static auto* method = RequireMethod(AssemblyNameClass(), "get_Name", {});
        return method;
    }

    Cerydra::Il2Cpp::Method* MonoAssembly::GetTypesMethod()
    {
        static auto* method = RequireMethod(AssemblyClass(), "GetTypes", { "bool" });
        return method;
    }

    Cerydra::Il2Cpp::Method* MonoAssembly::GetTypeMethod()
    {
        static auto* method = RequireMethod(AssemblyClass(), "GetType", { "string" });
        return method;
    }

    SystemString* MonoAssembly::GetName() const
    {
        auto* assemblyName = InvokeInstance<Object*>(this, GetNameMethod());
        return assemblyName ? InvokeInstance<SystemString*>(assemblyName, AssemblyNameGetNameMethod()) : nullptr;
    }

    Array<RuntimeType*>* MonoAssembly::GetTypes(bool exportedOnly) const
    {
        return InvokeInstance<Array<RuntimeType*>*, bool>(this, GetTypesMethod(), exportedOnly);
    }

    RuntimeType* MonoAssembly::GetTypeByName(SystemString* name) const
    {
        return InvokeInstance<RuntimeType*>(this, GetTypeMethod(), name);
    }
}
