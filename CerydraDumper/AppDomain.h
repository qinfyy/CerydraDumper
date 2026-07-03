#pragma once

#include "CSharpInvoke.h"

namespace Cerydra::CSharp
{
    class MonoAssembly;

    class AppDomain : public Object
    {
    public:
        static Cerydra::IL2CPP::Class* StaticClass();
        static Cerydra::IL2CPP::Method* GetAssembliesMethod();
        static Cerydra::IL2CPP::Method* GetCurrentDomainMethod();

        static AppDomain* GetCurrentDomain();
        Array<MonoAssembly*>* GetAssemblies() const;
    };
}
