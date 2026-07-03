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

    Object* Activator::CreateInstance(Object* type)
    {
        return InvokeStatic<Object*>(CreateInstanceMethod(), type);
    }

    Object* Activator::CreateInstanceWithArgs(Object* type, Array<Object*>* args)
    {
        return InvokeStatic<Object*>(CreateInstanceWithArgsMethod(), type, args);
    }

    Object* Activator::CreateInstanceWithNonpublic(Object* type, bool nonpublic)
    {
        return InvokeStatic<Object*>(CreateInstanceWithNonpublicMethod(), type, nonpublic);
    }
}
