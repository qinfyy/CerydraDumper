#include "pch.h"
#include "AppDomain.h"

namespace Cerydra::CSharp
{
    Cerydra::Il2Cpp::Class* AppDomain::StaticClass()
    {
        static auto* klass = RequireClass("System.AppDomain");
        return klass;
    }

    AppDomain* AppDomain::GetCurrentDomain()
    {
        static auto* method = RequireMethod(StaticClass(), "get_CurrentDomain", {});
        return InvokeStatic<AppDomain*>(method);
    }

    Array<MonoAssembly*>* AppDomain::GetAssemblies() const
    {
        static auto* method = RequireMethod(StaticClass(), "GetAssemblies", {});
        return InvokeInstance<Array<MonoAssembly*>*>(this, method);
    }
}
