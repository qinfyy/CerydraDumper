#pragma once
#include "Il2CppApiWrapper.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>

class CAppDomain : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.AppDomain");

    CIl2CppArray GetAssemblies() {
        return CallIl2CppInstanceObjectMethod<CIl2CppArray>(ptr, "System.AppDomain", "GetAssemblies", {});
    }

    CS_METHOD_STATIC_NOARGS(GetCurrentDomain, "get_CurrentDomain", {}, CAppDomain, (), ());
};
