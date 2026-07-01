#pragma once
#include "Il2CppApiWrapper.h"
#include "CSystemString.h"
#include "CSharpRuntime.h"
#include "CSharpModel.h"
#include <string>
#include <vector>
#include <stdexcept>

class CMonoAssembly : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    static Cerydra::IL2CPP::Class* AssemblyClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.Reflection.Assembly");
        return klass;
    }

    static Cerydra::IL2CPP::Class* AssemblyNameClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.Reflection.AssemblyName");
        return klass;
    }

    static Cerydra::IL2CPP::Method* M_GetName()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(AssemblyClass(), "GetName", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_AssemblyName_get_Name()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(AssemblyNameClass(), "get_Name", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetTypes()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(AssemblyClass(), "GetTypes", { "bool" });
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetType()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(AssemblyClass(), "GetType", { "string" });
        return method;
    }

    CSystemString GetName()
    {
        auto assemblyName = Cerydra::CSharp::InvokeInstance<uintptr_t>(ptr, M_GetName());
        return Cerydra::CSharp::InvokeInstance<CSystemString>(assemblyName, M_AssemblyName_get_Name());
    }

    CIl2CppArray GetTypes(bool flags)
    {
        return Cerydra::CSharp::InvokeInstance<CIl2CppArray, bool>(ptr, M_GetTypes(), flags);
    }

    CRuntimeType GetTypeByName(CSystemString name)
    {
        return Cerydra::CSharp::InvokeInstance<CRuntimeType>(ptr, M_GetType(), name);
    }
};

