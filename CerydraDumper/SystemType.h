#pragma once

#include "CSharpInvoke.h"

namespace Cerydra::CSharp
{
    class SystemType : public ObjectRef
    {
    public:
        using ObjectRef::ObjectRef;

        static Cerydra::IL2CPP::Class* StaticClass();
        static Cerydra::IL2CPP::Method* GetTypeFromHandleMethod();
        static SystemType GetTypeFromHandle(Il2CppType* type);
    };
}
