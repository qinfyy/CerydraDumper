#include "pch.h"
#include "SystemType.h"

namespace Cerydra::CSharp
{
    Cerydra::IL2CPP::Class* SystemType::StaticClass()
    {
        static auto* klass = RequireClass("System.Type");
        return klass;
    }

    Cerydra::IL2CPP::Method* SystemType::GetTypeFromHandleMethod()
    {
        static auto* method = RequireMethod(StaticClass(), "GetTypeFromHandle", { "System.RuntimeTypeHandle" });
        return method;
    }

    SystemType* SystemType::GetTypeFromHandle(Il2CppType* type)
    {
        return InvokeStatic<SystemType*>(GetTypeFromHandleMethod(), type);
    }
}
