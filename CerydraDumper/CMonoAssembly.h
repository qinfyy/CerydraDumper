#pragma once
#include "Il2CppApiWrapper.h"
#include "CSystemString.h"
#include "CSharpRuntime.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>

class CMonoAssembly : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.Runtime.InteropServices.Marshal");

    CSystemString GetName() {
        auto an = CallIl2CppInstanceObjectMethod<uintptr_t>(ptr, "System.Reflection.Assembly", "GetName", {});
        return CallIl2CppInstanceObjectMethod<CSystemString>(an, "System.Reflection.AssemblyName", "get_Name", {});
    }

    CIl2CppArray GetTypes(bool flags) {
        return CallIl2CppInstanceObjectMethod<CIl2CppArray, bool>(ptr, "System.Reflection.Assembly", "GetTypes",{ "bool" }, flags);
    }

    CRuntimeType GetTypeByName(CSystemString name) {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(this->ptr, "System.Reflection.Assembly", "GetType", { "string" }, name);
    }
};

