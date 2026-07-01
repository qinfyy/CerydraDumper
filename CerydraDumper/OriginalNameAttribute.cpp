#include "pch.h"
#include "OriginalNameAttribute.h"

namespace Cerydra::CSharp
{
    Cerydra::IL2CPP::Class* OriginalNameAttribute::StaticClass()
    {
        static auto* klass = RequireClass("Google.Protobuf.Reflection.OriginalNameAttribute");
        return klass;
    }

    Cerydra::IL2CPP::Method* OriginalNameAttribute::GetNameMethod()
    {
        static auto* method = RequireMethod(StaticClass(), "get_Name", {});
        return method;
    }

    SystemString OriginalNameAttribute::GetName() const
    {
        return InvokeInstance<SystemString>(ptr, GetNameMethod());
    }
}
