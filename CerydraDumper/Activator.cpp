#include "pch.h"
#include "Activator.h"

namespace Cerydra::CSharp
{
    Cerydra::Il2Cpp::Class* Activator::StaticClass()
    {
        static auto* klass = RequireClass("System.Activator");
        return klass;
    }

    Object* Activator::CreateInstance(Object* type)
    {
        static auto* method = RequireMethod(StaticClass(), "CreateInstance", { "System.Type" });
        return InvokeStatic<Object*>(method, type);
    }

    Object* Activator::CreateInstanceWithArgs(Object* type, Array<Object*>* args)
    {
        static auto* method = RequireMethod(StaticClass(), "CreateInstance", { "System.Type", "object[]" });
        return InvokeStatic<Object*>(method, type, args);
    }

    Object* Activator::CreateInstanceWithNonpublic(Object* type, bool nonpublic)
    {
        static auto* method = RequireMethod(StaticClass(), "CreateInstance", { "System.Type", "bool" });
        return InvokeStatic<Object*>(method, type, nonpublic);
    }
}
