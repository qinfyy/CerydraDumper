#pragma once

#include "CSharpInvoke.h"
#include <string>

namespace Cerydra::CSharp
{
    class SystemString : public Object
    {
    public:
        int32_t stringLength{};
        wchar_t firstChar[32]{};

        static Cerydra::IL2CPP::Class* MarshalClass();
        static Cerydra::IL2CPP::Method* PtrToStringAnsiMethod();
        static SystemString* PtrToStringAnsi(const char* value);

        std::string ToString() const;
        std::string AsString() const;

        bool Equals(const char* rhs) const;
        bool operator==(const char* rhs) const;
        bool operator!=(const char* rhs) const;
        operator std::string() const;
    };
}
