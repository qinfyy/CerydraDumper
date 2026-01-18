#include "pch.h"
#include "Initializer.h"
#include "PrintHelper.h"
#include "Il2CppFunctions.h"
#include "Il2CppDumper.h"
#include "Il2CppCache.h"
#include "CAppDomain.h"
#include "CMonoAssembly.h"
//
//#include "CSharpRender2.h"
//#include "CSharpRender3.h"

#include "Il2CppApiWrapper.h"
#include "CSystemString.h"
#include <iostream>
#include "CCSharpRuntime.h"

#define DUMPCS_RENDER 1


void TestPrintAllImageNames()
{

    // 获取当前域
    Il2CppDomain* domain = il2cpp_domain_get();
    if (!domain) {
        DebugPrintA("Failed to get IL2CPP domain.\n");
        return;
    }

    // 获取域里的程序集
    size_t assemblyCount = 0;
    Il2CppAssembly** assemblies = il2cpp_domain_get_assemblies(domain, &assemblyCount);
    if (!assemblies || assemblyCount == 0) {
        DebugPrintA("No assemblies found.\n");
        return;
    }

    DebugPrintA("Loaded Assemblies and Images:\n");

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
            DebugPrintA("%s\n", imageName);
        }
    }
}

void TestPrintAllImageNames_Wrapper()
{
    // 获取当前域（全局静态获取）
    CIl2CppDomain domain = CIl2CppDomain::get();
    if (domain.is_null()) {
        DebugPrintA("Failed to get IL2CPP domain.\n");
        return;
    }

    // 获取域里的所有程序集
    std::vector<CIl2CppAssembly> assemblies = domain.assemblies();
    if (assemblies.empty()) {
        DebugPrintA("No assemblies found.\n");
        return;
    }

    DebugPrintA("Loaded Assemblies and Images:\n");

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
            DebugPrintA("%s\n", imageName.c_str());
        }
    }
}

void TestWrapper()
{
    const char* hello = "Hello IL2CPP";

    // =========================
    // 1️⃣ 测试 System.String 封装
    // =========================
    try {
        CSystemString s = CSystemString::PtrToStringAnsi(hello);
        std::string cpp_str = s.AsString();

        printf("[String] Converted string: %s\n", cpp_str.c_str());
    }
    catch (const std::exception& e) {
        std::cerr << "[String] Exception: " << e.what() << std::endl;
        return;
    }

    // =========================
    // 2️⃣ 测试 AppDomain / Assembly（基础）
    // =========================
    try {
        auto domain = CAppDomain::GetCurrentDomain();
        auto assemblies = domain.GetAssemblies();

        std::cout << "[Domain] Assembly count: "
            << assemblies.length() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "[Domain] Exception: " << e.what() << std::endl;
        return;
    }

    // =========================
    // 3️⃣ 测试 CRuntimeType
    // =========================
    try {
        CRuntimeType stringType = CRuntimeType::FromName("System.String");

        std::cout << "[Type] System.String ptr: "
            << stringType.raw_ptr() << std::endl;

        std::cout << "[Type] Name: "
            << stringType.Name().AsString() << std::endl;

        std::cout << "[Type] FullName: "
            << stringType.FullName().AsString() << std::endl;

        std::cout << "[Type] Namespace: "
            << stringType.Namespace().AsString() << std::endl;

        std::cout << "[Type] IsEnum: "
            << stringType.IsEnum() << std::endl;

        std::cout << "[Type] IsGenericType: "
            << stringType.IsGenericType() << std::endl;

        std::cout << "[Type] IsValueType: "
            << stringType.IsValueType() << std::endl;

        auto baseType = stringType.BaseType();
        std::cout << "[Type] BaseType: "
            << baseType.FullName().AsString() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "[Type] Exception: " << e.what() << std::endl;
        return;
    }

    // =========================
    // 4️⃣ 测试 CMonoField（System.String.Empty）
    // =========================
    try {
        CRuntimeType stringType = CRuntimeType::FromName("System.String");

        // BindingFlags:
        // Public | Static | FlattenHierarchy = 0x10 | 0x08 | 0x40 = 0x58
        auto field = stringType.GetField(
            CSystemString::PtrToStringAnsi("Empty"),
            0x58
        );

        std::cout << "[Field] Name: "
            << field->Name().AsString() << std::endl;

        std::cout << "[Field] DeclaringType: "
            << field->DeclaringType().FullName().AsString() << std::endl;

        std::cout << "[Field] FieldType: "
            << field->FieldType().FullName().AsString() << std::endl;

        std::cout << "[Field] IsLiteral: "
            << field->IsLiteral() << std::endl;

        std::cout << "[Field] MetadataToken: "
            << field->MetadataToken() << std::endl;

        // static field → obj = nullptr
        auto emptyObj = field->GetValue(0);

        std::cout << "[Field] Empty value ptr: "
            << emptyObj.raw_ptr() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "[Field] Exception: " << e.what() << std::endl;
        return;
    }

    // =========================
    // 5️⃣ 测试 CMonoProperty（System.String.Length）
    // =========================
    try {
        CRuntimeType stringType = CRuntimeType::FromName("System.String");

        auto prop = stringType.GetProperty(
            CSystemString::PtrToStringAnsi("Length")
        );

        std::cout << "[Property] ptr: "
            << prop->raw_ptr() << std::endl;

        std::cout << "[Property] Name: "
            << prop->Name().AsString() << std::endl;

        std::cout << "[Property] DeclaringType: "
            << prop->DeclaringType().FullName().AsString() << std::endl;

        std::cout << "[Property] PropertyType: "
            << prop->PropertyType().FullName().AsString() << std::endl;

        // 构造一个 string 实例测试 GetValue
        CSystemString testStr = CSystemString::PtrToStringAnsi("abcdef");

        auto lenObj = prop->GetValue(testStr.raw_ptr());

        std::cout << "[Property] Length value ptr: "
            << lenObj.raw_ptr() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "[Property] Exception: " << e.what() << std::endl;
        return;
    }

    try {
        CSystemString helloStr = CSystemString::PtrToStringAnsi("Hello CSystemDynamic");

        CSystemDynamic dynObj(helloStr.raw_ptr());

        std::cout << "[Dynamic] ptr: " << dynObj.raw_ptr() << std::endl;

        try {
            CSystemString dynStr = dynObj.ToString();
            std::cout << "[Dynamic] ToString: " << dynStr.AsString() << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "[Dynamic] Exception: " << e.what() << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[Dynamic] Exception: " << e.what() << std::endl;
    }

    std::cout << "\n[TestWrapper] All tests finished successfully.\n";
}


DWORD WINAPI MainThread(LPVOID) {
    DebugPrintA("[INFO] RuntimeDumper\n");
    DebugPrintA("[INFO] Waiting for GameAssembly.dll...\n");

    while (!GetModuleHandle(L"GameAssembly.dll")) {
        Sleep(200);
    }

    DebugPrintA("[INFO] GameAssembly.dll loaded, Starting dump ...\n");

    Sleep(10000);
    InitIl2CppFunctions();
    InitCache();
    TestPrintAllImageNames_Wrapper();

    TestWrapper();

    //DumpCs(".\\output\\dump.cs");
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
