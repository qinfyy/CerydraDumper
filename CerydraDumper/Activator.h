#pragma once

#include "CSharpInvoke.h"

namespace Cerydra::CSharp
{
    class Activator : public ObjectRef
    {
    public:
        using ObjectRef::ObjectRef;

        static Cerydra::IL2CPP::Class* StaticClass();
        static Cerydra::IL2CPP::Method* CreateInstanceMethod();
        static Cerydra::IL2CPP::Method* CreateInstanceWithArgsMethod();
        static Cerydra::IL2CPP::Method* CreateInstanceWithNonpublicMethod();

        static RuntimeObject CreateInstance(uintptr_t type);
        static RuntimeObject CreateInstanceWithArgs(uintptr_t type, uintptr_t args);
        static RuntimeObject CreateInstanceWithNonpublic(uintptr_t type, bool nonpublic);
    };
}
