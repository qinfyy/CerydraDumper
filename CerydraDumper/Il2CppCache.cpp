#include "pch.h"
#include "Il2CppCache.h"
#include <iostream>
#include <filesystem>
#include "PrintHelper.h"

void InitCache() {
    std::call_once(INIT_ONCE_FLAG, [=]() {
        DebugPrintA("[Cache] 初始化 RuntimeCache ...\n");

        Il2CppRuntimeCache::Init();

        FUNCTIONS_TABLE.clear();
        TYPE_TABLE.clear();

        for (auto klass : Il2CppRuntimeCache::Classes()) {
            if (!klass || !klass->address) {
                continue;
            }

            CIl2CppClass legacyClass(reinterpret_cast<uintptr_t>(klass->address));
            if (klass->byvalType && !klass->byvalType->name.empty()) {
                TYPE_TABLE[klass->byvalType->name] = legacyClass;
            }
            if (!klass->fullName.empty()) {
                TYPE_TABLE[klass->fullName] = legacyClass;
            }
            if (!klass->name.empty()) {
                TYPE_TABLE.try_emplace(klass->name, legacyClass);
            }

            for (auto method : klass->methods) {
                if (!method || !method->address) {
                    continue;
                }

                FUNCTIONS_TABLE[method->SignatureKey()] = CIl2CppMethod(reinterpret_cast<uintptr_t>(method->address));
            }
        }

        DebugPrintA("[Cache] RuntimeCache 兼容表已回填: classes=%zu, methods=%zu\n", TYPE_TABLE.size(), FUNCTIONS_TABLE.size());
    });
}

CIl2CppMethod* GetNativeMethod(const std::string& key) {
    auto it = FUNCTIONS_TABLE.find(key);
    return it != FUNCTIONS_TABLE.end() ? &it->second : nullptr;
}

CIl2CppClass* GetCachedClass(const std::string& key) {
    auto it = TYPE_TABLE.find(key);
    return it != TYPE_TABLE.end() ? &it->second : nullptr;
}

Cerydra::IL2CPP::Method* GetCachedMethodMeta(const std::string& key) {
    return Il2CppRuntimeCache::GetMethod(key);
}

Cerydra::IL2CPP::Method* GetCachedMethodMeta(uintptr_t address) {
    return Il2CppRuntimeCache::GetMethodByAddress(address);
}

Cerydra::IL2CPP::Class* GetCachedClassMeta(const std::string& key) {
    return Il2CppRuntimeCache::GetClass(key);
}

Cerydra::IL2CPP::Class* GetCachedClassMeta(uintptr_t address) {
    return Il2CppRuntimeCache::GetClassByAddress(address);
}
