#include "pch.h"
#include "SystemString.h"
#include "Util.h"

namespace Cerydra::CSharp
{
    Cerydra::Il2Cpp::Class* SystemString::MarshalClass()
    {
        static auto* klass = RequireClass("System.Runtime.InteropServices.Marshal");
        return klass;
    }

    SystemString* SystemString::PtrToStringAnsi(const char* value)
    {
        static auto* method = RequireMethod(MarshalClass(), "PtrToStringAnsi", { "System.IntPtr" });
        return InvokeStatic<SystemString*>(method, value);
    }

    std::string SystemString::ToString() const
    {
        return Il2CppStringToUtf8String(const_cast<Il2CppString*>(reinterpret_cast<const Il2CppString*>(this)));
    }

    std::string SystemString::AsString() const
    {
        return ToString();
    }

    bool SystemString::Equals(const char* rhs) const
    {
        if (!rhs) {
            return false;
        }

        return AsString() == rhs;
    }

    bool SystemString::operator==(const char* rhs) const
    {
        return Equals(rhs);
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
