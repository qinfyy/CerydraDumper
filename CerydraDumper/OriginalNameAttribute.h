#pragma once

#include "CSharpInvoke.h"
#include "SystemString.h"

namespace Cerydra::CSharp
{
    class OriginalNameAttribute : public Object
    {
    public:
        static Cerydra::IL2CPP::Class* StaticClass();
        static Cerydra::IL2CPP::Method* GetNameMethod();

        SystemString* GetName() const;
    };
}
