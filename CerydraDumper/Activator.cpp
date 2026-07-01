#include "pch.h"
#include "Activator.h"

namespace Cerydra::CSharp
{
    Cerydra::IL2CPP::Class* Activator::StaticClass()
    {
        static auto* klass = RequireClass("System.Activator");
        return klass;
    }

    Cerydra::IL2CPP::Method* Activator::CreateInstanceMethod()
    {
        static auto* method = RequireMethod(StaticClass(), "CreateInstance", { "System.Type" });
        return method;
    }

    Cerydra::IL2CPP::Method* Activator::CreateInstanceWithArgsMethod()
    {
        static auto* method = RequireMethod(StaticClass(), "CreateInstance", { "System.Type", "object[]" });
        return method;
    }

    Cerydra::IL2CPP::Method* Activator::CreateInstanceWithNonpublicMethod()
    {
        static auto* method = RequireMethod(StaticClass(), "CreateInstance", { "System.Type", "bool" });
        return method;
    }

    RuntimeObject Activator::CreateInstance(uintptr_t type)
    {
        return InvokeStatic<RuntimeObject>(CreateInstanceMethod(), type);
    }

    RuntimeObject Activator::CreateInstanceWithArgs(uintptr_t type, uintptr_t args)
    {
        return InvokeStatic<RuntimeObject>(CreateInstanceWithArgsMethod(), type, args);
    }

    RuntimeObject Activator::CreateInstanceWithNonpublic(uintptr_t type, bool nonpublic)
    {
        return InvokeStatic<RuntimeObject>(CreateInstanceWithNonpublicMethod(), type, nonpublic);
    }
}
