#include "pch.h"
#include "AppDomain.h"

namespace Cerydra::CSharp
{
    Cerydra::IL2CPP::Class* AppDomain::StaticClass()
    {
        static auto* klass = RequireCoreLibClass("AppDomain");
        return klass;
    }

    Cerydra::IL2CPP::Method* AppDomain::GetAssembliesMethod()
    {
        static auto* method = RequireMethod(StaticClass(), "GetAssemblies", {});
        return method;
    }

    Cerydra::IL2CPP::Method* AppDomain::GetCurrentDomainMethod()
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
