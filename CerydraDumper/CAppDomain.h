#pragma once
#include "Il2CppApiWrapper.h"
#include "CSharpModel.h"
#include <string>
#include <vector>
#include <stdexcept>

class CAppDomain : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    static Cerydra::IL2CPP::Class* StaticClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.AppDomain");
        return klass;
    }

    static Cerydra::IL2CPP::Method* M_GetAssemblies()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(StaticClass(), "GetAssemblies", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetCurrentDomain()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(StaticClass(), "get_CurrentDomain", {});
        return method;
    }

    CIl2CppArray GetAssemblies()
    {
        return Cerydra::CSharp::InvokeInstance<CIl2CppArray>(ptr, M_GetAssemblies());
    }

    static CAppDomain GetCurrentDomain()
    {
        return Cerydra::CSharp::InvokeStatic<CAppDomain>(M_GetCurrentDomain());
    }
};
