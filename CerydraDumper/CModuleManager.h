#pragma once
#include "Il2CppApiWrapper.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>
#include "CSharpRuntime.h"

class CModuleManager : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

	CS_CLASS("RPG.Client.ModuleManager");

	CS_FIELD_INSTANCE(Modules, "modules", CNativeList);

};

