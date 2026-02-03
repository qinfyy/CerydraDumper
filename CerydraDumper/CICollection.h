#pragma once
#include "Il2CppApiWrapper.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>
#include "CSharpRuntime.h"

class CICollection : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

	CS_CLASS("System.Collections.ICollection");

	uintptr_t get_Count() const {
		return CallIl2CppInstanceObjectMethodDynamic<uintptr_t>(
			this->ptr,
			"get_Count",
			{}
		);
	}
};

