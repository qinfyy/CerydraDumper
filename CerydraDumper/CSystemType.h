#pragma once
#include "Il2CppApiWrapper.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>

class CSystemType : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.Type");

    CS_METHOD_STATIC(GetTypeFromHandle, "GetTypeFromHandle", FN_ARGS("System.RuntimeTypeHandle"), CSystemType, (Il2CppType* ty), (ty));
};

