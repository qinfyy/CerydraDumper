#include "pch.h"
#include "OriginalNameAttribute.h"

namespace Cerydra::CSharp
{
    Cerydra::Il2Cpp::Class* OriginalNameAttribute::StaticClass()
    {
        static auto* klass = RequireClass("Google.Protobuf.Reflection.OriginalNameAttribute");
        return klass;
    }

    SystemString* OriginalNameAttribute::GetName() const
    {
        static auto* method = RequireMethod(StaticClass(), "get_Name", {});
        return InvokeInstance<SystemString*>(this, method);
    }
}
