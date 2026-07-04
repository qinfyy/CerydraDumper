#pragma once

#include "Il2CppModel.h"
#include <string>
#include <vector>

class Il2CppRuntimeCache
{
public:
    static void Init();
    static bool IsInitialized();

    static Cerydra::Il2Cpp::Assembly* GetAssembly(const std::string& name);
    static const std::vector<Cerydra::Il2Cpp::Assembly*>& Assemblies();

private:
    static void BuildAssemblies();
    static void BuildClasses(Cerydra::Il2Cpp::Assembly* assembly, Cerydra::Il2Cpp::Image* image);
    static void BuildFields(Cerydra::Il2Cpp::Class* klass);
    static void BuildMethods(Cerydra::Il2Cpp::Class* klass);
    static Cerydra::Il2Cpp::Type* CreateType(const Il2CppType* type);
};
