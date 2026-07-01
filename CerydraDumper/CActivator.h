#pragma once
#include "Il2CppApiWrapper.h"
#include "CSharpModel.h"
#include <string>
#include <vector>
#include <stdexcept>

class CActivator : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    static Cerydra::IL2CPP::Class* StaticClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.Activator");
        return klass;
    }

    static Cerydra::IL2CPP::Method* M_CreateInstance()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(StaticClass(), "CreateInstance", { "System.Type" });
        return method;
    }

    static Cerydra::IL2CPP::Method* M_CreateInstanceWithArgs()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(StaticClass(), "CreateInstance", { "System.Type", "object[]" });
        return method;
    }

    static Cerydra::IL2CPP::Method* M_CreateInstanceWithNonpublic()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(StaticClass(), "CreateInstance", { "System.Type", "bool" });
        return method;
    }

    static CIl2CppObject CreateInstance(uintptr_t value)
    {
        return Cerydra::CSharp::InvokeStatic<CIl2CppObject>(M_CreateInstance(), value);
    }

    static CIl2CppObject CreateInstanceWithArgs(uintptr_t value, uintptr_t args)
    {
        return Cerydra::CSharp::InvokeStatic<CIl2CppObject>(M_CreateInstanceWithArgs(), value, args);
    }

    static CIl2CppObject CreateInstanceWithNonpublic(uintptr_t value, bool flag)
    {
        return Cerydra::CSharp::InvokeStatic<CIl2CppObject>(M_CreateInstanceWithNonpublic(), value, flag);
    }
};

