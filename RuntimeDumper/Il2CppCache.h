#pragma once
#include <unordered_map>
#include "Il2CppApiWrapper.h"
#include <mutex>

inline std::unordered_map<std::string, CIl2CppMethod> FUNCTIONS_TABLE;
inline std::unordered_map<std::string, CIl2CppClass> TYPE_TABLE;
inline std::once_flag INIT_ONCE_FLAG;

__declspec(noinline) inline CIl2CppMethod* GetNativeMethod(const std::string& key) {
    auto it = FUNCTIONS_TABLE.find(key);
    return it != FUNCTIONS_TABLE.end() ? &it->second : nullptr;
}

__declspec(noinline) inline CIl2CppClass* GetCachedClass(const std::string& key) {
    auto it = TYPE_TABLE.find(key);
    return it != TYPE_TABLE.end() ? &it->second : nullptr;
}

void InitCache();
