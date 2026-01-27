#pragma once
#include "Il2CppApiWrapper.h"
#include "Bind.h"
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

	CS_CLASS("Google.Protobuf.Reflection.OriginalNameAttribute");

	inline CSystemString GetName() const {
		return CallIl2CppInstanceObjectMethod<CSystemString>(
			this->ptr,
			"Google.Protobuf.Reflection.OriginalNameAttribute",
			"get_Name",
			{}
		);
	}
};

