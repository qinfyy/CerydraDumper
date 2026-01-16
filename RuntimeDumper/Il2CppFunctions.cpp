#include "pch.h"
#include "Il2CppFunctions.h"
#include <Windows.h>
#include <iostream>
#include "Memory.h"

uintptr_t GetUnityPlayerModuleBase()
{
    HMODULE mod = GetModuleHandleA("UnityPlayer.dll");
    return reinterpret_cast<uintptr_t>(mod);
}

uintptr_t GetGameAssemblyModuleBase()
{
    HMODULE mod = GetModuleHandleA("GameAssembly.dll");
    return reinterpret_cast<uintptr_t>(mod);
}

uintptr_t API_BASE_PTR = 0;

uintptr_t GetApiBase()
{
    return API_BASE_PTR;
}

uintptr_t ExtractQwordTarget(uintptr_t instruction_address) {
    int32_t relative_offset = *reinterpret_cast<int32_t*>(instruction_address + 3);
    uintptr_t next_instruction = instruction_address + 7;
    return next_instruction + relative_offset;
}

void InitIl2CppFunctions()
{
    HMODULE hUnityPlayer = GetModuleHandleA("UnityPlayer.dll");
    if (!hUnityPlayer) {
        MessageBoxA(NULL, "UnityPlayer.dll not found!", "Error", MB_OK | MB_ICONERROR);
        ExitProcess(1);
        return;
    }

    uintptr_t target = Scan(hUnityPlayer, "48 8B 05 ? ? ? ? 48 8D 0D ? ? ? ? FF D0");

    if (target != 0) {
        API_BASE_PTR = ExtractQwordTarget(target);
        std::cout << "[INFO] IL2CPP functions table: " << std::hex << API_BASE_PTR << std::endl;
    }
    else {
        MessageBoxA(NULL, "Failed to find IL2CPP functions table!", "Error", MB_OK | MB_ICONERROR);
        ExitProcess(1);
    }
}
