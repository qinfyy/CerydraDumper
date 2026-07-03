#include "pch.h"
#include "SystemString.h"
#include "Util.h"

namespace Cerydra::CSharp
{
    Cerydra::IL2CPP::Class* SystemString::MarshalClass()
    {
        static auto* klass = RequireCoreLibClass("Marshal", "System.Runtime.InteropServices");
        return klass;
    }

    Cerydra::IL2CPP::Method* SystemString::PtrToStringAnsiMethod()
    {
        static auto* method = RequireMethod(MarshalClass(), "PtrToStringAnsi", { "System.IntPtr" });
        return method;
    }

    SystemString* SystemString::PtrToStringAnsi(const char* value)
    {
        return InvokeStatic<SystemString*>(PtrToStringAnsiMethod(), value);
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
