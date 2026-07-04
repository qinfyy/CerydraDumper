#pragma once

#include "CSharpInvoke.h"

namespace Cerydra::CSharp
{
    class MonoAssembly;

    class AppDomain : public Object
    {
    public:
        static Cerydra::Il2Cpp::Class* StaticClass();
        static Cerydra::Il2Cpp::Method* GetAssembliesMethod();
        static Cerydra::Il2Cpp::Method* GetCurrentDomainMethod();

        static AppDomain* GetCurrentDomain();
        Array<MonoAssembly*>* GetAssemblies() const;
    };
}
