#include "pch.h"
#include "SystemString.h"
#include "Util.h"

namespace Cerydra::CSharp
{
    Cerydra::IL2CPP::Class* SystemString::MarshalClass()
    {
        static auto* klass = RequireClass("System.Runtime.InteropServices.Marshal");
        return klass;
    }

    Cerydra::IL2CPP::Method* SystemString::PtrToStringAnsiMethod()
    {
        static auto* method = RequireMethod(MarshalClass(), "PtrToStringAnsi", { "System.IntPtr" });
        return method;
    }

    SystemString SystemString::PtrToStringAnsi(const char* value)
    {
        return InvokeStatic<SystemString>(PtrToStringAnsiMethod(), value);
    }

    std::string SystemString::AsString() const
    {
        if (IsNull()) {
            return "";
        }

        return Il2CppStringToUtf8String(reinterpret_cast<Il2CppString*>(ptr));
    }

    bool SystemString::operator==(const char* rhs) const
    {
        if (!rhs) {
            return IsNull();
        }

        return !IsNull() && AsString() == rhs;
    }

    bool SystemString::operator!=(const char* rhs) const
    {
        return !(*this == rhs);
    }

    SystemString::operator std::string() const
    {
        return AsString();
    }
}
