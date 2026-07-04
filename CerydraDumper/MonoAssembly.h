#pragma once

#include "CSharpInvoke.h"
#include "SystemString.h"

namespace Cerydra::CSharp
{
    class RuntimeType;

    class MonoAssembly : public Object
    {
    public:
        static Cerydra::Il2Cpp::Class* AssemblyClass();
        static Cerydra::Il2Cpp::Class* AssemblyNameClass();
        static Cerydra::Il2Cpp::Method* GetNameMethod();
        static Cerydra::Il2Cpp::Method* AssemblyNameGetNameMethod();
        static Cerydra::Il2Cpp::Method* GetTypesMethod();
        static Cerydra::Il2Cpp::Method* GetTypeMethod();

        SystemString* GetName() const;
        Array<RuntimeType*>* GetTypes(bool exportedOnly) const;
        RuntimeType* GetTypeByName(SystemString* name) const;
    };
}
