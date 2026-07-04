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

        SystemString* GetName() const;
        Array<RuntimeType*>* GetTypes(bool exportedOnly) const;
        RuntimeType* GetTypeByName(SystemString* name) const;
    };
}
