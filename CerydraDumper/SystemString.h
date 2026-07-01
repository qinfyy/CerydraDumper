#pragma once

#include "CSharpInvoke.h"
#include <string>

namespace Cerydra::CSharp
{
    class SystemString : public ObjectRef
    {
    public:
        using ObjectRef::ObjectRef;

        SystemString() = default;
        explicit SystemString(const char* value)
        {
            *this = PtrToStringAnsi(value);
        }

        static Cerydra::IL2CPP::Class* MarshalClass();
        static Cerydra::IL2CPP::Method* PtrToStringAnsiMethod();
        static SystemString PtrToStringAnsi(const char* value);

        std::string AsString() const;

        bool operator==(const char* rhs) const;
        bool operator!=(const char* rhs) const;
        operator std::string() const;
    };
}
