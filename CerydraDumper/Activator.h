#pragma once

#include "CSharpInvoke.h"

namespace Cerydra::CSharp
{
    class Activator final
    {
    public:
        static Cerydra::Il2Cpp::Class* StaticClass();

        static Object* CreateInstance(Object* type);
        static Object* CreateInstanceWithArgs(Object* type, Array<Object*>* args);
        static Object* CreateInstanceWithNonpublic(Object* type, bool nonpublic);
    };
}
