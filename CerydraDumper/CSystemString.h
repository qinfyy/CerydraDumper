#pragma once
#include "Il2CppApiWrapper.h"
#include "CSharpModel.h"
#include <string>
#include <vector>
#include <stdexcept>

class CSystemString : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    static Cerydra::IL2CPP::Class* MarshalClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.Runtime.InteropServices.Marshal");
        return klass;
    }

    static Cerydra::IL2CPP::Method* M_PtrToStringAnsi()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(MarshalClass(), "PtrToStringAnsi", { "System.IntPtr" });
        return method;
    }

    static CSystemString PtrToStringAnsi(const char* charPtr)
    {
        return Cerydra::CSharp::InvokeStatic<CSystemString>(M_PtrToStringAnsi(), charPtr);
    }

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

    operator std::string() const {
        return AsString();
    }
};
