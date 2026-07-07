#include "pch.h"
#include "Initializer.h"
#include "AppDomain.h"
#include "Il2CppDumper.h"
#include "Il2CppFunctions.h"
#include "Il2CppRuntimeCache.h"
#include "ProtoDumper.h"
#include "PrintHelper.h"
#include "RuntimeType.h"
#include "SystemString.h"
#include "UnityResolve.hpp"
#include <DbgHelp.h>
#include <filesystem>
#include <optional>
#include "DumpCs2.h"
#pragma comment(lib, "DbgHelp.lib")

using namespace Cerydra::CSharp;

void WriteFullDump(EXCEPTION_POINTERS* ep)
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    char path[MAX_PATH];
    sprintf_s(path, "Crash_%04d%02d%02d_%02d%02d%02d.dmp", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    HANDLE hFile = CreateFileA(path, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    MINIDUMP_EXCEPTION_INFORMATION mei{};
    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = ep;
    mei.ClientPointers = FALSE;

    auto dumpType = static_cast<MINIDUMP_TYPE>(
        MiniDumpWithFullMemory | MiniDumpWithHandleData | MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules);
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, dumpType, &mei, nullptr, nullptr);

    CloseHandle(hFile);
}

LONG WINAPI GlobalExceptionFilter(EXCEPTION_POINTERS* ep)
{
    WriteFullDump(ep);
    return EXCEPTION_EXECUTE_HANDLER;
}

void TestPrintAllImageNamesWrapper()
{
    DebugPrintA("[Test] Loaded Assemblies and Images:\n");
    for (const auto* assembly : Il2CppRuntimeCache::Assemblies()) {
        if (!assembly || !assembly->image) {
            continue;
        }

        DebugPrintA("[Test] %s\n", assembly->image->name.c_str());
    }
}

void TestWrapper()
{
    try {
        using namespace Cerydra::Il2Cpp;
        auto findStringLengthMethod = [](const char* assemblyName) -> Method* {
            auto* assembly = Get(assemblyName);
            auto* image = assembly ? assembly->Get() : nullptr;
            auto* klass = image ? image->Get("String", "System") : nullptr;
            return klass ? klass->Get<Method>("get_Length") : nullptr;
        };

        auto* assemblyName = "mscorlib.dll";
        auto* method = findStringLengthMethod(assemblyName);
        if (!method) {
            assemblyName = "System.Private.CoreLib.dll";
            method = findStringLengthMethod(assemblyName);
        }

        DebugPrintA("[URGet] %s System.String.get_Length method: %p\n", assemblyName, method);
    }
    catch (const std::exception& e) {
        DebugPrintA("[URGet] Exception: %s\n", e.what());
    }

    try {
        auto* str = SystemString::PtrToStringAnsi("Hello il2cpp");
        DebugPrintA("[String] Converted string: %s\n", str ? str->AsString().c_str() : "");
    }
    catch (const std::exception& e) {
        DebugPrintA("[String] Exception: %s\n", e.what());
        return;
    }

    try {
        auto* domain = AppDomain::GetCurrentDomain();
        auto* assemblies = domain ? domain->GetAssemblies() : nullptr;
        DebugPrintA("[Domain] Assembly count: %zu\n", assemblies ? assemblies->Length() : 0);
    }
    catch (const std::exception& e) {
        DebugPrintA("[Domain] Exception: %s\n", e.what());
        return;
    }

    try {
        auto* stringType = RuntimeType::FromName("System.String");
        DebugPrintA("[Type] System.String ptr: %p\n", stringType);
        DebugPrintA("[Type] FullName: %s\n", stringType->GetFullName()->AsString().c_str());
        DebugPrintA("[Type] Namespace: %s\n", stringType->GetNamespace()->AsString().c_str());
        DebugPrintA("[Type] IsEnum: %s\n", stringType->IsEnum() ? "true" : "false");
        DebugPrintA("[Type] IsGenericType: %s\n", stringType->IsGenericType() ? "true" : "false");
        DebugPrintA("[Type] IsValueType: %s\n", stringType->IsValueType() ? "true" : "false");

        auto* baseType = stringType->GetBaseType();
        DebugPrintA("[Type] BaseType: %s\n", baseType ? baseType->GetFullName()->AsString().c_str() : "");
    }
    catch (const std::exception& e) {
        DebugPrintA("[Type] Exception: %s\n", e.what());
        return;
    }

    try {
        auto* stringType = RuntimeType::FromName("System.String");
        auto* field = stringType->GetFieldObject(
            SystemString::PtrToStringAnsi("Empty"),
            kPublicStaticFlattenHierarchyBindingFlags);

        DebugPrintA("[Field] Name: %s\n", field->GetName()->AsString().c_str());
        DebugPrintA("[Field] DeclaringType: %s\n", field->GetDeclaringType()->GetFullName()->AsString().c_str());
        DebugPrintA("[Field] FieldType: %s\n", field->GetFieldType()->GetFullName()->AsString().c_str());
        DebugPrintA("[Field] IsLiteral: %s\n", field->IsLiteral() ? "true" : "false");
        DebugPrintA("[Field] MetadataToken: %d\n", field->GetMetadataToken());

        auto* emptyObj = field->GetValue(nullptr);
        DebugPrintA("[Field] Empty value ptr: %p\n", emptyObj);
    }
    catch (const std::exception& e) {
        DebugPrintA("[Field] Exception: %s\n", e.what());
        return;
    }

    try {
        auto* stringType = RuntimeType::FromName("System.String");
        auto* prop = stringType->GetProperty(SystemString::PtrToStringAnsi("Length"));
        DebugPrintA("[Property] ptr: %p\n", prop);
        DebugPrintA("[Property] Name: %s\n", prop->GetName()->AsString().c_str());
        DebugPrintA("[Property] DeclaringType: %s\n", prop->GetDeclaringType()->GetFullName()->AsString().c_str());
        DebugPrintA("[Property] PropertyType: %s\n", prop->GetPropertyType()->GetFullName()->AsString().c_str());

        auto* testStr = SystemString::PtrToStringAnsi("abcdef");
        auto* lenObj = prop->GetValue(testStr);
        DebugPrintA("[Property] Length value ptr: %p\n", lenObj);
    }
    catch (const std::exception& e) {
        DebugPrintA("[Property] Exception: %s\n", e.what());
        return;
    }

    try {
        auto* helloStr = SystemString::PtrToStringAnsi("Hello SystemDynamic");
        auto* dynObj = reinterpret_cast<SystemDynamic*>(helloStr);
        DebugPrintA("[Dynamic] ptr: %p\n", dynObj);
        DebugPrintA("[Dynamic] ToString: %s\n", dynObj->ToString()->AsString().c_str());
    }
    catch (const std::exception& e) {
        DebugPrintA("[Dynamic] Exception: %s\n", e.what());
    }

    DebugPrintA("[TestWrapper] All tests finished successfully.\n");
}

void yep()
{
    try {
        Cerydra::Il2Cpp::Assembly* protoAssembly = nullptr;

        for (auto* assembly : Il2CppRuntimeCache::Assemblies()) {
            if (!assembly) {
                continue;
            }

            DebugPrintA("[yep] Assembly: %s\n", assembly->name.c_str());
            if (assembly->name == "Game") {
                protoAssembly = assembly;
            }
        }

        DumpCs(".\\output\\dump.cs");
        DumpCs2(".\\output\\dump2.cs");

        if (protoAssembly) {
            DumpProtos2(protoAssembly, ".\\output\\dump.proto");
        }
        else {
            DebugPrintA("[ERROR] RPG.Network.Proto not found\n");
        }
    }
    catch (const std::exception& e) {
        DebugPrintA("[yep] Exception: %s\n", e.what());
    }
}

DWORD WINAPI MainThread(LPVOID)
{
    SetUnhandledExceptionFilter(GlobalExceptionFilter);
    DebugPrintA("[INFO] CerydraDumper\n");
    DebugPrintA("[INFO] Waiting for GameAssembly.dll ...\n");

    HMODULE base;
    while (!(base = GetModuleHandle(L"GameAssembly.dll"))) {
        Sleep(200);
    }

    DebugPrintA("[INFO] GameAssembly.dll loaded, base: 0x%llX, Starting dump ...\n", base);

    for (int i = 25; i > 0; --i) {
        DebugPrintA("\r[INFO] Wait for %d seconds before starting il2cpp dump ...  ", i);
        Sleep(1000);
    }

    DebugPrintA("\n");
    DebugPrintA("[INFO] Start il2cpp dump!\n");

    //UnityResolve::Init(base, UnityResolve::Mode::Il2Cpp);

    InitIl2CppFunctions();
    Il2CppRuntimeCache::Init();

    TestPrintAllImageNamesWrapper();
    TestWrapper();
    yep();

	DumpProtos2(Il2CppRuntimeCache::GetAssembly("Game"), ".\\output\\dump.proto");

    DebugPrintA("[INFO] All done.\n");
    return 0;
}
