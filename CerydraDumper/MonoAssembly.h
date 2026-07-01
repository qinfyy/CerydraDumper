#pragma once

#include "CSharpInvoke.h"
#include "SystemString.h"

namespace Cerydra::CSharp
{
    class RuntimeType;

    class MonoAssembly : public ObjectRef
    {
    public:
        using ObjectRef::ObjectRef;

        static Cerydra::IL2CPP::Class* AssemblyClass();
        static Cerydra::IL2CPP::Class* AssemblyNameClass();
        static Cerydra::IL2CPP::Method* GetNameMethod();
        static Cerydra::IL2CPP::Method* AssemblyNameGetNameMethod();
        static Cerydra::IL2CPP::Method* GetTypesMethod();
        static Cerydra::IL2CPP::Method* GetTypeMethod();

        SystemString GetName() const;
        ArrayObject GetTypes(bool exportedOnly) const;
        RuntimeType GetTypeByName(SystemString name) const;
    };
}
