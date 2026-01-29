#pragma once
#include "Il2CppApiWrapper.h"
#include "CSystemString.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>

class CMonoAssembly : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.Runtime.InteropServices.Marshal");

    inline CSystemString GetFullName() {
        return CallIl2CppInstanceObjectMethod<CSystemString>(ptr, "System.Reflection.Assembly", "get_FullName",{});
    }

    inline CIl2CppArray GetTypes(bool flags) {
        return CallIl2CppInstanceObjectMethod<CIl2CppArray, bool>(ptr, "System.Reflection.Assembly", "GetTypes",{ "bool" }, flags);
    }
};

