#pragma once
#include "Il2CppApiWrapper.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>

class CActivator : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.Activator");

    CS_METHOD_STATIC_AUTO_CTOR(CreateInstance, "CreateInstance", FN_ARGS("System.Type"), CIl2CppObject, (uintptr_t value), (value));
    CS_METHOD_STATIC_AUTO_CTOR(CreateInstanceWithArgs, "CreateInstance", FN_ARGS("System.Type", "object[]"), CIl2CppObject, (uintptr_t value, uintptr_t args), (value, args));
    CS_METHOD_STATIC_AUTO_CTOR(CreateInstanceWithNonpublic, "CreateInstance", FN_ARGS("System.Type", "bool"), CIl2CppObject, (uintptr_t value, bool flag), (value, flag));
};

