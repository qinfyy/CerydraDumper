#pragma once
#include <unordered_map>
#include "Il2CppApiWrapper.h"
#include "Il2CppRuntimeCache.h"
#include <mutex>

inline std::unordered_map<std::string, CIl2CppMethod> FUNCTIONS_TABLE;
inline std::unordered_map<std::string, CIl2CppClass> TYPE_TABLE;
inline std::once_flag INIT_ONCE_FLAG;

void InitCache();

CIl2CppMethod* GetNativeMethod(const std::string& key);

CIl2CppClass* GetCachedClass(const std::string& key);

Cerydra::IL2CPP::Method* GetCachedMethodMeta(const std::string& key);
Cerydra::IL2CPP::Method* GetCachedMethodMeta(uintptr_t address);
Cerydra::IL2CPP::Class* GetCachedClassMeta(const std::string& key);
Cerydra::IL2CPP::Class* GetCachedClassMeta(uintptr_t address);
