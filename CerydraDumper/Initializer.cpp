#include "pch.h"
#include "Initializer.h"
#include "PrintHelper.h"
#include "Il2CppFunctions.h"
#include "Il2CppDumper.h"
#include "Il2CppCache.h"
#include "CAppDomain.h"
#include "CMonoAssembly.h"
#include "Il2CppApiWrapper.h"
#include "CSystemString.h"
#include <iostream>
#include "CSharpRuntime.h"
#include <optional>
#include "CmdIdOut.h"
#include "ProtoDumper.h"

#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")

void WriteFullDump(EXCEPTION_POINTERS* ep)
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    char path[MAX_PATH];
    sprintf_s(path, "Crash_%04d%02d%02d_%02d%02d%02d.dmp", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    HANDLE hFile = CreateFileA(path, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return;

    MINIDUMP_EXCEPTION_INFORMATION mei{};
    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = ep;
    mei.ClientPointers = FALSE;

    MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(MiniDumpWithFullMemory | MiniDumpWithHandleData | MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules);
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, dumpType, &mei, nullptr, nullptr);

    CloseHandle(hFile);
}

LONG WINAPI GlobalExceptionFilter(EXCEPTION_POINTERS* ep)
{
    WriteFullDump(ep);
    return EXCEPTION_EXECUTE_HANDLER;
}

#define DUMPCS_RENDER 1


void TestPrintAllImageNames()
{
    // 获取当前域
    Il2CppDomain* domain = il2cpp_domain_get();
    if (!domain) {
        DebugPrintA("[Test] [ERROR] Failed to get il2cpp domain.\n");
        return;
    }

    // 获取域里的程序集
    size_t assemblyCount = 0;
    Il2CppAssembly** assemblies = il2cpp_domain_get_assemblies(domain, &assemblyCount);
    if (!assemblies || assemblyCount == 0) {
        DebugPrintA("[Test] [ERROR] No assemblies found.\n");
        return;
    }

    DebugPrintA("[Test] Loaded Assemblies and Images:\n");

    // 遍历所有程序集
    for (size_t i = 0; i < assemblyCount; i++)
    {
        Il2CppAssembly* assembly = assemblies[i];
        if (!assembly) continue;

        // 获取程序集对应的 image
        Il2CppImage* image = il2cpp_assembly_get_image(assembly);
        if (!image) continue;

        // 获取 image 名称
        const char* imageName = il2cpp_image_get_name(image);
        if (imageName) {
            DebugPrintA("[Test] Assembly: %s\n", imageName);
        }
    }
}

void TestPrintAllImageNamesWrapper()
{
    // 获取当前域（全局静态获取）
    CIl2CppDomain domain = CIl2CppDomain::get();
    if (domain.is_null()) {
        DebugPrintA("[Test] [ERROR] Failed to get il2cpp domain.\n");
        return;
    }

    // 获取域里的所有程序集
    std::vector<CIl2CppAssembly> assemblies = domain.assemblies();
    if (assemblies.empty()) {
        DebugPrintA("[Test] [ERROR] No assemblies found.\n");
        return;
    }

    DebugPrintA("[Test] Loaded Assemblies and Images:\n");

    // 遍历程序集
    for (const CIl2CppAssembly& assembly : assemblies)
    {
        if (assembly.is_null()) continue;

        // 获取程序集对应的 image
        CIl2CppImage image = assembly.get_image();
        if (image.is_null()) continue;

        // 获取 image 名称
        std::string imageName = image.name();
        if (!imageName.empty()) {
            DebugPrintA("[Test] %s\n", imageName.c_str());
        }
    }
}

void TestWrapper()
{
    const char* hello = "Hello il2cpp";

    try {
        CSystemString s = CSystemString::PtrToStringAnsi(hello);
        std::string cpp_str = s.AsString();

        DebugPrintA("[String] Converted string: %s\n", cpp_str.c_str());
    }
    catch (const std::exception& e) {
        DebugPrintA("[String] Exception: %s\n", e.what());
        return;
    }

    try {
        auto domain = CAppDomain::GetCurrentDomain();
        auto assemblies = domain.GetAssemblies();

        DebugPrintA("[Domain] Assembly count: %zu\n", assemblies.length());
    }
    catch (const std::exception& e) {
        DebugPrintA("[Domain] Exception: %s\n", e.what());
        return;
    }

    try {
        CRuntimeType stringType = CRuntimeType::FromName("System.String");

        DebugPrintA("[Type] System.String ptr: %p\n", stringType.raw_ptr());

        DebugPrintA("[Type] Name: %s\n", stringType.Name().AsString().c_str());

        DebugPrintA("[Type] FullName: %s\n", stringType.FullName().AsString().c_str());

        DebugPrintA("[Type] Namespace: %s\n", stringType.Namespace().AsString().c_str());

        DebugPrintA("[Type] IsEnum: %s\n", stringType.IsEnum() ? "true" : "false");

        DebugPrintA("[Type] IsGenericType: %s\n", stringType.IsGenericType() ? "true" : "false");

        DebugPrintA("[Type] IsValueType: %s\n", stringType.IsValueType() ? "true" : "false");

        auto baseType = stringType.BaseType();
        DebugPrintA("[Type] BaseType: %s\n", baseType.FullName().AsString().c_str());
    }
    catch (const std::exception& e) {
        DebugPrintA("[Type] Exception: %s\n", e.what());
        return;
    }

    try {
        CRuntimeType stringType = CRuntimeType::FromName("System.String");

        // BindingFlags:
        // Public | Static | FlattenHierarchy = 0x10 | 0x08 | 0x40 = 0x58
        auto field = stringType._GetField("Empty", 0x58);

        DebugPrintA("[Field] Name: %s\n", field->Name().AsString().c_str());

        DebugPrintA("[Field] DeclaringType: %s\n", field->DeclaringType().FullName().AsString().c_str());

        DebugPrintA("[Field] FieldType: %s\n", field->FieldType().FullName().AsString().c_str());

        DebugPrintA("[Field] IsLiteral: %s\n", field->IsLiteral() ? "true" : "false");

        DebugPrintA("[Field] MetadataToken: %d\n", field->MetadataToken());

        // static field → obj = nullptr
        auto emptyObj = field->GetValue(0);

        DebugPrintA("[Field] Empty value ptr: %p\n", emptyObj.raw_ptr());
    }
    catch (const std::exception& e) {
        DebugPrintA("[Field] Exception: %s\n", e.what());
        return;
    }

    try {
        CRuntimeType stringType = CRuntimeType::FromName("System.String");

        auto prop = stringType.GetProperty("Length");

        DebugPrintA("[Property] ptr: %p\n", prop->raw_ptr());

        DebugPrintA("[Property] Name: %s\n", prop->Name().AsString().c_str());

        DebugPrintA("[Property] DeclaringType: %s\n", prop->DeclaringType().FullName().AsString().c_str());

        DebugPrintA("[Property] PropertyType: %s\n",  prop->PropertyType().FullName().AsString().c_str());

        // 构造一个 string 实例测试 GetValue
        CSystemString testStr = CSystemString::PtrToStringAnsi("abcdef");

        auto lenObj = prop->GetValue(testStr.raw_ptr());

        DebugPrintA("[Property] Length value ptr: %p\n", lenObj.raw_ptr());
    }
    catch (const std::exception& e) {
        DebugPrintA("[Property] Exception: %s\n", e.what());
        return;
    }

    try {
        CSystemString helloStr = CSystemString::PtrToStringAnsi("Hello CSystemDynamic");

        CSystemDynamic dynObj(helloStr.raw_ptr());

        DebugPrintA("[Dynamic] ptr: %p\n", dynObj.raw_ptr());

        try {
            CSystemString dynStr = dynObj.ToString();
            DebugPrintA("[Dynamic] ToString: %s\n",  dynStr.AsString().c_str());
        }
        catch (const std::exception& e) {
            DebugPrintA("[Dynamic] Exception: %s\n", e.what());
        }
    }
    catch (const std::exception& e) {
        DebugPrintA("[Dynamic] Exception: %s\n", e.what());
    }

    DebugPrintA("[TestWrapper] All tests finished successfully.\n");
}

void yep() {
    auto domain = CAppDomain::GetCurrentDomain();
    auto assemblies = domain.GetAssemblies().to_vec<CMonoAssembly>();

    std::optional<CMonoAssembly> proto_assembly;
    std::optional<CMonoAssembly> excel_assembly;
    std::optional<CMonoAssembly> cmdid_assembly;

    for (size_t i = 0; i < assemblies.size(); ++i) {
        if (proto_assembly && excel_assembly && cmdid_assembly)
            break;

        CMonoAssembly mono_assembly(assemblies[i]);
        std::string assembly_name = mono_assembly.GetFullName().AsString();

        if (assembly_name.rfind("RPG.Network.Proto,", 0) == 0) { // starts_with
            proto_assembly = mono_assembly;
        }
        else if (assembly_name.rfind("RPG.GameCore.Config,", 0) == 0) {
            excel_assembly = mono_assembly;
        }
        else if (assembly_name.rfind("Assembly-CSharp,", 0) == 0) {
            cmdid_assembly = mono_assembly;
        }
    }

    DumpCs(".\\output\\dump.cs");

    if (cmdid_assembly) 
        CmdIdDump(*cmdid_assembly, ".\\output\\cmdid.json");
    else
        printf("[ERROR] Assembly-CSharp not foundn");

    if (proto_assembly)
        ProtoDump(*proto_assembly, ".\\output\\dump.proto");
    else
        printf("[ERROR] RPG.Network.Proto not found\n");
}

DWORD WINAPI MainThread(LPVOID) {
    SetUnhandledExceptionFilter(GlobalExceptionFilter);
    DebugPrintA("[INFO] CerydraDumper\n");
    DebugPrintA("[INFO] Waiting for GameAssembly.dll ...\n");

    while (!GetModuleHandle(L"GameAssembly.dll")) {
        Sleep(200);
    }

    DebugPrintA("[INFO] GameAssembly.dll loaded, Starting dump ...\n");

    int countdown = 15;
    for (int i = countdown; i > 0; --i) {
        DebugPrintA("\r[INFO] Wait for %d seconds before starting il2cpp dump ...  ", i);
        Sleep(1000);
    }
	DebugPrintA("\n");
    DebugPrintA("[INFO] Start il2cpp dump!\n");

    InitIl2CppFunctions();
    InitCache();
    TestPrintAllImageNamesWrapper();

    TestWrapper();
    yep();
 
//    Il2CppDomain* domain = nullptr;
//    Il2CppThread* thread = nullptr;
//    if (!AttachIl2Cpp(domain, thread))
//    {
//        return 1;
//    }
//
//#if DUMPCS_RENDER == 1
//    DumpCs(".\\output\\dump.cs");
//#elif DUMPCS_RENDER == 2
//    DumpCs2(".\\output\\dump.cs");
//#elif DUMPCS_RENDER == 3
//    DumpCs3(".\\output\\dump.cs");
//#else
//#error "Unknown DUMPCS_RENDER value"
//#endif
//
//    DumpJsonOutputToFile(".\\output\\script.json");
//	DumpMetaFile(".\\output\\global-metadata.dat");
//    GetCodeRegistration();
//    DumpProtos(".\\output\\dump.proto");

	DebugPrintA("[INFO] All done.\n");

    return 0;
}
