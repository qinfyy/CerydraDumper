#pragma once

#include "Il2CppModel.h"
#include <string>
#include <vector>

class Il2CppRuntimeCache
{
public:
    static void Init();
    static bool IsInitialized();

    static Cerydra::IL2CPP::Assembly* GetAssembly(const std::string& name);
    static const std::vector<Cerydra::IL2CPP::Assembly*>& Assemblies();

private:
    static void BuildAssemblies();
    static void BuildClasses(Cerydra::IL2CPP::Assembly* assembly, Cerydra::IL2CPP::Image* image);
    static void BuildFields(Cerydra::IL2CPP::Class* klass);
    static void BuildMethods(Cerydra::IL2CPP::Class* klass);
    static Cerydra::IL2CPP::Type* CreateType(const Il2CppType* type);
};
