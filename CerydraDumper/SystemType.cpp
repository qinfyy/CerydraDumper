#include "pch.h"
#include "SystemType.h"

namespace Cerydra::CSharp
{
    Cerydra::Il2Cpp::Class* SystemType::StaticClass()
    {
        static auto* klass = RequireClass("System.Type");
        return klass;
    }

    Cerydra::Il2Cpp::Method* SystemType::GetTypeFromHandleMethod()
    {
        static auto* method = RequireMethod(StaticClass(), "GetTypeFromHandle", { "System.RuntimeTypeHandle" });
        return method;
    }

    SystemType* SystemType::GetTypeFromHandle(Il2CppType* type)
    {
        return InvokeStatic<SystemType*>(GetTypeFromHandleMethod(), type);
    }
}
