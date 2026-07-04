#pragma once

#include "CSharpInvoke.h"

namespace Cerydra::CSharp
{
    class SystemType : public Object
    {
    public:
        static Cerydra::Il2Cpp::Class* StaticClass();
        static SystemType* GetTypeFromHandle(Il2CppType* type);
    };
}
