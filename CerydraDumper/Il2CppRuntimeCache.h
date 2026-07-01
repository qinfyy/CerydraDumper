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
    static Cerydra::IL2CPP::Class* GetClass(const std::string& fullOrAliasName);
    static Cerydra::IL2CPP::Class* GetClass(const std::string& namespaze, const std::string& name);
    static Cerydra::IL2CPP::Class* GetClassByAddress(uintptr_t address);
    static Cerydra::IL2CPP::Method* GetMethod(const std::string& key);
    static Cerydra::IL2CPP::Method* GetMethodByAddress(uintptr_t address);
    static Cerydra::IL2CPP::Field* GetFieldByAddress(uintptr_t address);
    static Cerydra::IL2CPP::Type* GetTypeByAddress(uintptr_t address);

    static const std::vector<Cerydra::IL2CPP::Assembly*>& Assemblies();
    static const std::vector<Cerydra::IL2CPP::Class*>& Classes();
    static const std::vector<Cerydra::IL2CPP::Method*>& Methods();

private:
    static void BuildAssemblies();
    static void BuildClasses(Cerydra::IL2CPP::Assembly* assembly, Cerydra::IL2CPP::Image* image);
    static void BuildFields(Cerydra::IL2CPP::Class* klass);
    static void BuildMethods(Cerydra::IL2CPP::Class* klass);
    static Cerydra::IL2CPP::Type* GetOrCreateType(const Il2CppType* type);
};
