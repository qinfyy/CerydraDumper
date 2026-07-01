#pragma once

#include "CSharpInvoke.h"

namespace Cerydra::CSharp
{
    class AppDomain : public ObjectRef
    {
    public:
        using ObjectRef::ObjectRef;

        static Cerydra::IL2CPP::Class* StaticClass();
        static Cerydra::IL2CPP::Method* GetAssembliesMethod();
        static Cerydra::IL2CPP::Method* GetCurrentDomainMethod();

        static AppDomain GetCurrentDomain();
        ArrayObject GetAssemblies() const;
    };
}
