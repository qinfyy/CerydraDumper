#include "pch.h"
#include "Il2CppFunctions.h"
#include <Windows.h>
#include <iostream>
#include "Memory.h"
#include "PrintHelper.h"
#include <sstream>
#include <unordered_set>
#include <vector>

namespace
{
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
            "il2cpp_class_get_properties",
            "il2cpp_class_get_type",
            "il2cpp_class_is_enum",
            "il2cpp_class_is_interface",
            "il2cpp_class_is_subclass_of",
            "il2cpp_class_is_valuetype",
            "il2cpp_domain_get",
            "il2cpp_domain_get_assemblies",
            "il2cpp_field_get_flags",
            "il2cpp_field_get_name",
            "il2cpp_field_get_offset",
            "il2cpp_field_get_type",
            "il2cpp_field_static_get_value",
            "il2cpp_field_static_set_value",
            "il2cpp_image_get_class",
            "il2cpp_image_get_class_count",
            "il2cpp_image_get_filename",
            "il2cpp_image_get_name",
            "il2cpp_method_get_flags",
            "il2cpp_method_get_name",
            "il2cpp_method_get_param",
            "il2cpp_method_get_param_count",
            "il2cpp_method_get_param_name",
            "il2cpp_method_get_return_type",
            "il2cpp_object_new",
            "il2cpp_property_get_get_method",
            "il2cpp_property_get_name",
            "il2cpp_property_get_set_method",
            "il2cpp_thread_attach",
            "il2cpp_type_get_attrs",
            "il2cpp_type_get_name",
            "il2cpp_type_get_object",
            "il2cpp_type_get_type",
            "il2cpp_type_is_byref",
        };
        return apis;
    }

    bool IsUsedIl2CppApi(const char* apiName)
    {
        return UsedIl2CppApis().contains(apiName);
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
}

uintptr_t GetGameAssemblyModuleBase()
{
    HMODULE mod = GetModuleHandleA("GameAssembly.dll");
    return reinterpret_cast<uintptr_t>(mod);
}

void* FindIl2CppAddress(const std::string& funcName)
{
    auto it = address_.find(funcName);
    if (it == address_.end() || it->second == nullptr) {
        throw std::runtime_error("IL2CPP API 未绑定: " + funcName);
    }

    return it->second;
}

void InitIl2CppFunctions()
{
    HMODULE hGameAssembly = reinterpret_cast<HMODULE>(GetGameAssemblyModuleBase());
    if (!hGameAssembly) {
        MessageBoxW(NULL, L"未找到 GameAssembly.dll!", L"错误", MB_OK | MB_ICONERROR);
        ExitProcess(1);
        return;
    }

    int totalApi = 0;
    int failedApi = 0;
    int usedApi = 0;
    int failedUsedApi = 0;
    int failedUnusedApi = 0;
    std::vector<std::string> failedUsedApis;
    std::vector<std::string> failedUnusedApis;
    address_.clear();

#define DO_API(r, n, p) \
    do { \
        totalApi++; \
        void* apiAddress = reinterpret_cast<void*>(GetProcAddress(hGameAssembly, #n)); \
        address_[#n] = apiAddress; \
        DebugPrintA("[IL2CPP] %-55s -> %p\n", #n, apiAddress); \
        const bool usedApi_ = IsUsedIl2CppApi(#n); \
        if (usedApi_) usedApi++; \
        if (!apiAddress) { \
            failedApi++; \
            if (usedApi_) { \
                failedUsedApi++; \
                failedUsedApis.emplace_back(#n); \
            } \
            else { \
                failedUnusedApi++; \
                failedUnusedApis.emplace_back(#n); \
            } \
        } \
    } while (false);

#define DO_API_NO_RETURN(r, n, p) DO_API(r, n, p)
#include "il2cpp-api-functions.h"
#undef DO_API
#undef DO_API_NO_RETURN

    if (failedUsedApi > 0) {
        std::wstringstream ss;
        ss << L"检测到项目使用的 IL2CPP API 绑定失败: " << failedUsedApi << L"/" << usedApi
            << std::endl
            << L"这些 API 会被当前代码调用，程序无法继续运行。";

        AppendApiList(ss, failedUsedApis);
        ss << std::endl << L"请检查游戏 Unity 版本，或确认游戏是否被加密/保护。";

        MessageBoxW(NULL, ss.str().c_str(), L"IL2CPP API 绑定错误", MB_OK | MB_ICONERROR);
        ExitProcess(1);
    }

    if (failedUnusedApi > 0) {
        std::wstringstream ss;
        ss << L"检测到 IL2CPP API 绑定失败: " << failedUnusedApi << L"/" << totalApi<< std::endl;
        AppendApiList(ss, failedUnusedApis);

        MessageBoxW(NULL, ss.str().c_str(), L"IL2CPP API 绑定警告", MB_OK | MB_ICONWARNING);
    }
}
