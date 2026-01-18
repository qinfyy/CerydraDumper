#include "pch.h"
#include "Initializer.h"
#include "PrintHelper.h"
#include "Il2CppFunctions.h"
#include "Il2CppDumper.h"
#include "Il2CppCache.h"
#include "CAppDomain.h"
#include "MonoAssembly.h"
//
//#include "CSharpRender2.h"
//#include "CSharpRender3.h"

#include "Il2CppApiWrapper.h"
#include "SystemString.h"
#include <iostream>

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

    const char* hello = "Hello IL2CPP";

    try {
        // 调用静态方法创建 SystemString
        CSystemString s = CSystemString::PtrToStringAnsi(hello);

        std::string cpp_str = s.AsString();

        printf("Converted string: %s\n", cpp_str.c_str());

		auto csDom = CAppDomain::GetCurrentDomain();
        auto csasms = csDom.GetAssemblies();
        if (csasms.is_null()) {
            DebugPrintA("No assemblies found.\n");
            return 0;
		}

		std::cout << "Assembly Image len: " << csasms.length() << "\n";

        for (auto& csasm : csasms.to_vec<CMonoAssembly>()) {
            auto csimg = csasm.GetFullName();
			DebugPrintA("Assembly Image Name: %s\n", csimg.AsString().c_str());
        }
    }
    catch (const std::runtime_error& e) {
        std::cout << "捕获到 runtime_error: " << e.what() << "\n";
    }
    catch (...) {
        std::cout << "捕获到其他异常\n";
    }


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
