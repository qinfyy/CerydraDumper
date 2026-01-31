#pragma once
#include "Il2CppApiWrapper.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>

class CSystemString : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.Runtime.InteropServices.Marshal");

    CS_METHOD_STATIC_AUTO_CTOR(PtrToStringAnsi, "PtrToStringAnsi", {"System.IntPtr"}, CSystemString, (const char* charPtr), (charPtr));

    std::string AsString() const;

    CSystemString(const char* str) {
        *this = PtrToStringAnsi(str);
    }

    bool operator==(const char* rhs) const {
        if (!rhs)
            return this->is_null();

        if (this->is_null())
            return false;

        return this->AsString() == rhs;
    }

    bool operator!=(const char* rhs) const {
        return !(*this == rhs);
    }

};
