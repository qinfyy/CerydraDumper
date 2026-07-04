#include "pch.h"
#include "AppDomain.h"

namespace Cerydra::CSharp
{
    Cerydra::Il2Cpp::Class* AppDomain::StaticClass()
    {
        static auto* klass = RequireClass("System.AppDomain");
        return klass;
    }

    Cerydra::Il2Cpp::Method* AppDomain::GetAssembliesMethod()
    {
        static auto* method = RequireMethod(StaticClass(), "GetAssemblies", {});
        return method;
    }

    Cerydra::Il2Cpp::Method* AppDomain::GetCurrentDomainMethod()
    {
        static auto* method = RequireMethod(StaticClass(), "get_CurrentDomain", {});
        return method;
    }

    AppDomain* AppDomain::GetCurrentDomain()
    {
        return InvokeStatic<AppDomain*>(GetCurrentDomainMethod());
    }

    Array<MonoAssembly*>* AppDomain::GetAssemblies() const
    {
        return InvokeInstance<Array<MonoAssembly*>*>(this, GetAssembliesMethod());
    }
}
