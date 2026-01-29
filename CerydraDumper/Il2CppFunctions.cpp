#include "pch.h"
#include "Il2CppFunctions.h"
#include <Windows.h>
#include <iostream>
#include "Memory.h"
#include "PrintHelper.h"

static uintptr_t API_BASE_PTR = 0;

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
    HMODULE hUnityPlayer = (HMODULE)GetUnityPlayerModuleBase();
    if (!hUnityPlayer) {
        MessageBoxA(NULL, "UnityPlayer.dll not found!", "Error", MB_OK | MB_ICONERROR);
        ExitProcess(1);
        return;
    }

    uintptr_t target = Scan(hUnityPlayer, "48 8B 05 ? ? ? ? 48 8D 0D ? ? ? ? FF D0");
    DebugPrintA("[INFO] Target: %p, RVA: 0x%llX\n", target, target - GetUnityPlayerModuleBase());

    if (target != 0) {
        API_BASE_PTR = ExtractQwordTarget(target);
        DebugPrintA("[INFO] IL2CPP functions table: %p\n", API_BASE_PTR);
    }
    else {
        MessageBoxA(NULL, "Failed to find IL2CPP functions table!", "Error", MB_OK | MB_ICONERROR);
        ExitProcess(1);
    }
}
