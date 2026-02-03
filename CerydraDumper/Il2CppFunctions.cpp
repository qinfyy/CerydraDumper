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

void InitIl2CppFunctions()
{
    HMODULE hGameAssembly = reinterpret_cast<HMODULE>(GetGameAssemblyModuleBase());
    if (!hGameAssembly) {
        MessageBoxA(NULL, "GameAssembly.dll not found!", "Error", MB_OK | MB_ICONERROR);
        ExitProcess(1);
        return;
    }

    int totalApi = 0;
    int failedApi = 0;

#define DO_API(r, n, p) \
    totalApi++; \
    n = (r (*) p) GetProcAddress(hGameAssembly, #n); \
    DebugPrintA("[IL2CPP] %-55s -> %p\n", #n, n); \
    if (!n) failedApi++;

#define DO_API_NO_RETURN(r, n, p) DO_API(r, n, p)
#include "il2cpp-api-functions.h"
#undef DO_API
#undef DO_API_NO_RETURN

    if (failedApi > 0) {
        std::stringstream ss;
        ss << "Detected " << failedApi << "/" << totalApi
            << " IL2CPP APIs failed to load!" << std::endl
            << "Please check your game Unity version or whether the game is encrypted/protected.";

        MessageBoxA(NULL, ss.str().c_str(), "IL2CPP API Binding Error", MB_OK | MB_ICONERROR);
        ExitProcess(1);
    }
}
