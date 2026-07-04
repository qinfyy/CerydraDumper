#pragma once

#include "CSharpInvoke.h"
#include "SystemString.h"

namespace Cerydra::CSharp
{
    class OriginalNameAttribute : public Object
    {
    public:
        static Cerydra::Il2Cpp::Class* StaticClass();
        static Cerydra::Il2Cpp::Method* GetNameMethod();

        SystemString* GetName() const;
    };
}
