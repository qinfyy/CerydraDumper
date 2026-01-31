#include "pch.h"
#include "Il2CppCache.h"
#include <iostream>
#include <filesystem>
#include "PrintHelper.h"

void InitCache() {
    std::call_once(INIT_ONCE_FLAG, [=]() {
		DebugPrintA("[Cache] Caching classes ...\n");

        CIl2CppDomain domain = CIl2CppDomain::get();

        il2cpp_thread_attach(domain);

        for (auto& assembly : domain.assemblies()) {
            CIl2CppImage image = assembly.get_image();

            for (auto& klass : image.classes()) {
                std::string type_name = klass.byval_arg().name();
				//DebugPrintA("[Cache] Caching type: %s\n", type_name.c_str());

                for (auto& method : klass.methods()) {
                    std::string key = type_name + "::" + method.format_params();
                    FUNCTIONS_TABLE[key] = method;
                }

                TYPE_TABLE[type_name] = klass;
            }
        }
        DebugPrintA("[Cache] Cached\n");
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
