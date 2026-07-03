#pragma once

#include "CSharpInvoke.h"

namespace Cerydra::CSharp
{
    class Activator final
    {
    public:
        static Cerydra::IL2CPP::Class* StaticClass();
        static Cerydra::IL2CPP::Method* CreateInstanceMethod();
        static Cerydra::IL2CPP::Method* CreateInstanceWithArgsMethod();
        static Cerydra::IL2CPP::Method* CreateInstanceWithNonpublicMethod();

        static Object* CreateInstance(Object* type);
        static Object* CreateInstanceWithArgs(Object* type, Array<Object*>* args);
        static Object* CreateInstanceWithNonpublic(Object* type, bool nonpublic);
    };
}
