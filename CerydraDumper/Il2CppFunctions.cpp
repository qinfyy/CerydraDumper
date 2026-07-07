#include "pch.h"
#include "Il2CppFunctions.h"
#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <array>
#include <iostream>
#include "Memory.h"
#include "PrintHelper.h"
#include <sstream>
#include <unordered_set>
#include <vector>
#include <filesystem>

namespace
{
    struct Il2CppApiBinding
    {
        const char* name;
        size_t slot;
    };

    struct Il2CppFixedApiBinding
    {
        const char* name;
        uintptr_t rva;
    };

    constexpr size_t kSrApiTableEntryCount = 195;
    uintptr_t g_srApiTableBase = 0;

    constexpr Il2CppApiBinding kIl2CppApiBindings[] = {
        { "il2cpp_get_corlib", 21 },
        { "il2cpp_assembly_get_image", 22 },
        { "il2cpp_class_get_fields", 31 },
        { "il2cpp_class_get_interfaces", 33 },
        { "il2cpp_class_get_methods", 35 },
        { "il2cpp_class_get_name", 37 },
        { "il2cpp_class_get_namespace", 39 },
        { "il2cpp_class_get_parent", 40 },
        { "il2cpp_class_is_valuetype", 43 },
        { "il2cpp_class_get_flags", 45 },
        { "il2cpp_class_from_type", 49 },
        { "il2cpp_class_is_enum", 53 },
        { "il2cpp_domain_get", 63 },
        { "il2cpp_domain_assembly_open", 64 },
        { "il2cpp_domain_get_assemblies", 65 },
        { "il2cpp_field_get_flags", 72 },
        { "il2cpp_field_get_name", 73 },
        { "il2cpp_field_get_offset", 75 },
        { "il2cpp_field_get_type", 76 },
        { "il2cpp_field_get_value_object", 77 },
        { "il2cpp_method_get_return_type", 116 },
        { "il2cpp_method_get_name", 117 },
        { "il2cpp_method_get_param_count", 123 },
        { "il2cpp_method_get_param", 124 },
        { "il2cpp_object_get_class", 127 },
        { "il2cpp_object_get_virtual_method", 129 },
        { "il2cpp_object_new", 130 },
        { "il2cpp_object_unbox", 131 },
        { "il2cpp_thread_attach", 154 },
        { "il2cpp_type_get_name", 161 },
        { "il2cpp_type_is_byref", 162 },
        { "il2cpp_type_get_attrs", 163 },
        { "il2cpp_image_get_name", 168 },
        { "il2cpp_image_get_class_count", 169 },
        { "il2cpp_image_get_class", 170 },
        { "il2cpp_type_get_object", 158 },
        { "il2cpp_type_get_type", 159 },
        { "il2cpp_class_is_subclass_of", 26 },
        { "il2cpp_class_is_interface", 47 },
        { "il2cpp_class_get_type", 51 },
    };

    constexpr std::array<Il2CppFixedApiBinding, 0> kFixedIl2CppApiBindings = {};

    const std::unordered_set<std::string>& UsedIl2CppApis()
    {
        static const std::unordered_set<std::string> apis{
            "il2cpp_assembly_get_image",
            "il2cpp_class_from_type",
            "il2cpp_class_get_fields",
            "il2cpp_class_get_flags",
            "il2cpp_class_get_methods",
            "il2cpp_class_get_name",
            "il2cpp_class_get_namespace",
            "il2cpp_class_get_parent",
            //"il2cpp_class_get_properties", //1
            "il2cpp_class_get_type", //1 5
            "il2cpp_class_is_enum",
            "il2cpp_class_is_interface", //1 3
            "il2cpp_class_is_subclass_of", //1 3
            "il2cpp_class_is_valuetype",
            "il2cpp_domain_get",
            "il2cpp_domain_get_assemblies",
            "il2cpp_field_get_flags",
            "il2cpp_field_get_name",
            "il2cpp_field_get_offset",
            "il2cpp_field_get_type",
            //"il2cpp_field_static_get_value", //1 3
            //"il2cpp_field_static_set_value", //1 3
            "il2cpp_image_get_class",
            "il2cpp_image_get_class_count",
            //"il2cpp_image_get_filename", //1 0
            "il2cpp_image_get_name",
            //"il2cpp_method_get_flags", //1 1
            "il2cpp_method_get_name",
            "il2cpp_method_get_param",
            "il2cpp_method_get_param_count",
            //"il2cpp_method_get_param_name", //1 0
            "il2cpp_method_get_return_type",
            "il2cpp_object_new",
            //"il2cpp_property_get_get_method", //1 0
            //"il2cpp_property_get_name", //1 0
            //"il2cpp_property_get_set_method", //1 0
            "il2cpp_thread_attach",
            "il2cpp_type_get_attrs",
            "il2cpp_type_get_name",
            "il2cpp_type_get_object", //1 4
            "il2cpp_type_get_type", //1 4
            "il2cpp_type_is_byref",
        };
        return apis;
    }

    std::wstring ToWideApiName(const std::string& value)
    {
        return std::wstring(value.begin(), value.end());
    }

    void AppendApiList(std::wstringstream& ss, const std::vector<std::string>& apis)
    {
        constexpr size_t kMaxDisplayCount = 30;
        const auto displayCount = apis.size() < kMaxDisplayCount ? apis.size() : kMaxDisplayCount;
        for (size_t i = 0; i < displayCount; ++i) {
            ss << L"\n  - " << ToWideApiName(apis[i]);
        }
        if (apis.size() > kMaxDisplayCount) {
            ss << L"\n  - ... 还有 " << (apis.size() - kMaxDisplayCount) << L" 个";
        }
    }

    void DebugLogAddress(HMODULE gameAssembly, uintptr_t apiTableBase)
    {
		const char* outputPath = ".\\output\\Il2CppAddress.txt";

        const std::filesystem::path filePath(outputPath);
        const auto directory = filePath.parent_path();
        if (!directory.empty() && !std::filesystem::exists(directory)) {
            std::filesystem::create_directories(directory);
        }

        FILE* file = nullptr;
        if (fopen_s(&file, outputPath, "w") != 0 || !file) {
            DebugPrintA("[IL2CPP ERROR] 无法写入 Il2CppAddress.txt\n");
            return;
        }

        const auto gameAssemblyBase = reinterpret_cast<uintptr_t>(gameAssembly);
        fprintf(file, "GameAssembly.dll Base=%p\n", reinterpret_cast<void*>(gameAssemblyBase));
        for (size_t i = 0; i < kSrApiTableEntryCount; ++i) {
            uintptr_t address = 0;
            if (apiTableBase) {
                address = *reinterpret_cast<uintptr_t*>(apiTableBase + i * sizeof(uintptr_t));
            }

            constexpr uintptr_t kIdaImageBase = 0x180000000;
            uint64_t rva = 0;
            uint64_t idaVa = 0;
            const char* name = "Unknown";
            const bool decoded = address >= gameAssemblyBase && address - gameAssemblyBase < kIdaImageBase;
            if (decoded) {
                rva = static_cast<uint64_t>(address - gameAssemblyBase);
                idaVa = static_cast<uint64_t>(kIdaImageBase + rva);
                for (const auto& binding : kIl2CppApiBindings) {
                    if (binding.slot == i) {
                        name = binding.name;
                        break;
                    }
                }
            }

            fprintf(file, "index=%d, name=%s -> VA=%p, RVA=0x%llX, IDA VA=0x%llX\n", static_cast<int>(i), name, reinterpret_cast<void*>(address), rva, idaVa);
        }

        fclose(file);
    }
}

uintptr_t GetGameAssemblyModuleBase()
{
    HMODULE mod = GetModuleHandleA("GameAssembly.dll");
    return reinterpret_cast<uintptr_t>(mod);
}

uintptr_t GetUnityPlayerModuleBase()
{
    HMODULE mod = GetModuleHandleA("UnityPlayer.dll");
    return reinterpret_cast<uintptr_t>(mod);
}

void* FindIl2CppAddress(const std::string& funcName)
{
    auto it = address_.find(funcName);
    if (it == address_.end() || it->second == nullptr) {
		return nullptr;
    }

    return it->second;
}

void InitIl2CppFunctions()
{
    HMODULE hGameAssembly = reinterpret_cast<HMODULE>(GetGameAssemblyModuleBase());
    if (!hGameAssembly) {
        MessageBoxW(NULL, L"未找到 GameAssembly.dll!", L"严重错误", MB_OK | MB_ICONERROR);
        ExitProcess(1);
        return;
    }

    int usedApi = 0;
    int failedUsedApi = 0;
    std::vector<std::string> failedUsedApis;
    address_.clear();

    HMODULE hUnityPlayer = reinterpret_cast<HMODULE>(GetUnityPlayerModuleBase());
    if (!hUnityPlayer) {
        MessageBox(NULL, L"未找到 UnityPlayer.dll", L"严重错误", MB_OK | MB_ICONERROR);
        ExitProcess(1);
    }

    constexpr LPCSTR kApiTablePattern = "48 8B 05 ? ? ? ? 48 8D 0D ? ? ? ? FF D0";
    uintptr_t instructionAddress = Scan(hUnityPlayer, kApiTablePattern);
    if (!instructionAddress) {
        DebugPrintA("[IL2CPP ERROR] 特征码匹配失败，未找到 API table\n");
    }
    else {
        g_srApiTableBase = ExtractQwordTarget(instructionAddress);
        DebugPrintA("[IL2CPP] API table: %p\n", reinterpret_cast<void*>(g_srApiTableBase));
    }

    DebugLogAddress(hGameAssembly, g_srApiTableBase);
    for (const auto& binding : kIl2CppApiBindings) {
        void* apiAddress = nullptr;
        if (g_srApiTableBase) {
            apiAddress = *reinterpret_cast<void**>(g_srApiTableBase + binding.slot * sizeof(void*));
        }

        address_[binding.name] = apiAddress;
        DebugPrintA("[IL2CPP] %-55s -> %p\n", binding.name, apiAddress);
        if (UsedIl2CppApis().contains(binding.name)) {
            usedApi++;
            if (!apiAddress) {
                failedUsedApi++;
                failedUsedApis.emplace_back(binding.name);
            }
        }
    }

    for (const auto& binding : kFixedIl2CppApiBindings) {
        void* apiAddress = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(hGameAssembly) + binding.rva);
        address_[binding.name] = apiAddress;
        DebugPrintA("[IL2CPP] %-55s -> %p\n", binding.name, apiAddress);
        if (UsedIl2CppApis().contains(binding.name)) {
            usedApi++;
            if (!apiAddress) {
                failedUsedApi++;
                failedUsedApis.emplace_back(binding.name);
            }
        }
    }

    if (failedUsedApi > 0) {
        std::wstringstream ss;
        ss << L"检测到项目使用的 IL2CPP API 绑定失败: " << failedUsedApi << L"/" << usedApi
            << std::endl
            << L"这些 API 会被当前代码调用，程序无法继续运行。";

        AppendApiList(ss, failedUsedApis);
        ss << std::endl << L"请检查游戏 Unity 版本，或确认游戏是否被加密/保护。";

        MessageBoxW(NULL, ss.str().c_str(), L"严重错误", MB_OK | MB_ICONERROR);
        ExitProcess(1);
    }
}
