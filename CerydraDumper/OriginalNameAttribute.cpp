#include "pch.h"
#include "OriginalNameAttribute.h"

namespace Cerydra::CSharp
{
    Cerydra::Il2Cpp::Class* OriginalNameAttribute::StaticClass()
    {
        static auto* klass = RequireClass("Google.Protobuf.Reflection.OriginalNameAttribute");
        return klass;
    }

    Cerydra::Il2Cpp::Method* OriginalNameAttribute::GetNameMethod()
    {
        static auto* method = RequireMethod(StaticClass(), "get_Name", {});
        return method;
    }

    SystemString* OriginalNameAttribute::GetName() const
    {
        return InvokeInstance<SystemString*>(this, GetNameMethod());
    }
}
