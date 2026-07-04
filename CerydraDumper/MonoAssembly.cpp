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

    SystemString* MonoAssembly::GetName() const
    {
        static auto* getNameMethod = RequireMethod(AssemblyClass(), "GetName", {});
        static auto* assemblyNameGetNameMethod = RequireMethod(AssemblyNameClass(), "get_Name", {});
        auto* assemblyName = InvokeInstance<Object*>(this, getNameMethod);
        return assemblyName ? InvokeInstance<SystemString*>(assemblyName, assemblyNameGetNameMethod) : nullptr;
    }

    Array<RuntimeType*>* MonoAssembly::GetTypes(bool exportedOnly) const
    {
        static auto* method = RequireMethod(AssemblyClass(), "GetTypes", { "bool" });
        return InvokeInstance<Array<RuntimeType*>*, bool>(this, method, exportedOnly);
    }

    RuntimeType* MonoAssembly::GetTypeByName(SystemString* name) const
    {
        static auto* method = RequireMethod(AssemblyClass(), "GetType", { "string" });
        return InvokeInstance<RuntimeType*>(this, method, name);
    }
}
