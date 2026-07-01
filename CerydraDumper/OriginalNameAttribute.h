#pragma once
#include "Il2CppApiWrapper.h"
#include "CSharpModel.h"
#include <string>
#include <vector>
#include <stdexcept>
#include "CSystemType.h"
#include "CSystemString.h"
#include "PrintHelper.h"
#include "CActivator.h"

class OriginalNameAttribute : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    static Cerydra::IL2CPP::Class* StaticClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("Google.Protobuf.Reflection.OriginalNameAttribute");
        return klass;
    }

    static Cerydra::IL2CPP::Method* M_GetName()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(StaticClass(), "get_Name", {});
        return method;
    }

    CSystemString GetName() const
    {
        return Cerydra::CSharp::InvokeInstance<CSystemString>(ptr, M_GetName());
    }
};

