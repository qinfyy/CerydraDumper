#include "pch.h"
#include "SystemType.h"

namespace Cerydra::CSharp
{
    Cerydra::Il2Cpp::Class* SystemType::StaticClass()
    {
        static auto* klass = RequireClass("System.Type");
        return klass;
    }

    SystemType* SystemType::GetTypeFromHandle(Il2CppType* type)
    {
        static auto* method = RequireMethod(StaticClass(), "GetTypeFromHandle", { "System.RuntimeTypeHandle" });
        return InvokeStatic<SystemType*>(method, type);
    }
}
