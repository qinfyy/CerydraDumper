#include "pch.h"
#include "Il2CppFunctions.h"
#include <Windows.h>
#include <iostream>
#include "Memory.h"
#include "PrintHelper.h"
#include <sstream>

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
    address_.clear();

#define DO_API(r, n, p) \
    do { \
        totalApi++; \
        void* apiAddress = reinterpret_cast<void*>(GetProcAddress(hGameAssembly, #n)); \
        address_[#n] = apiAddress; \
        DebugPrintA("[IL2CPP] %-55s -> %p\n", #n, apiAddress); \
        if (!apiAddress) failedApi++; \
    } while (false);

#define DO_API_NO_RETURN(r, n, p) DO_API(r, n, p)
#include "il2cpp-api-functions.h"
#undef DO_API
#undef DO_API_NO_RETURN

    if (failedApi > 0) {
        std::wstringstream ss;
        ss << L"检测到 " << failedApi << L"/" << totalApi
            << L" 个 IL2CPP API 绑定失败!" << std::endl
            << L"请检查游戏 Unity 版本，或确认游戏是否被加密/保护。";

        MessageBoxW(NULL, ss.str().c_str(), L"IL2CPP API 绑定错误", MB_OK | MB_ICONERROR);
        ExitProcess(1);
    }
}
