#pragma once
#include "Il2CppApiWrapper.h"
#include "CSharpModel.h"
#include <string>
#include <vector>
#include <stdexcept>

class CSystemType : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    static Cerydra::IL2CPP::Class* StaticClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.Type");
        return klass;
    }

    static Cerydra::IL2CPP::Method* M_GetTypeFromHandle()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(StaticClass(), "GetTypeFromHandle", { "System.RuntimeTypeHandle" });
        return method;
    }

    static CSystemType GetTypeFromHandle(Il2CppType* ty)
    {
        return Cerydra::CSharp::InvokeStatic<CSystemType>(M_GetTypeFromHandle(), ty);
    }
};

