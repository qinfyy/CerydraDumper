#include "pch.h"
#include "Initializer.h"
#include "PrintHelper.h"
#include "Il2CppFunctions.h"
//#include "Il2CppDumper.h"
//#include "JsonGenerator.h"
//#include "MetaDumper.h"
//#include "ProtoDumper.h"
//
//#include "CSharpRender2.h"
//#include "CSharpRender3.h"

#include "Il2CppApiWrapper.h"

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
    Il2CppApiWrapper::Il2CppDomain domain = Il2CppApiWrapper::Il2CppDomain::get();
    if (domain.is_null()) {
        DebugPrintA("Failed to get IL2CPP domain.\n");
        return;
    }

    // 获取域里的所有程序集
    std::vector<Il2CppApiWrapper::Il2CppAssembly> assemblies = domain.assemblies();
    if (assemblies.empty()) {
        DebugPrintA("No assemblies found.\n");
        return;
    }

    DebugPrintA("Loaded Assemblies and Images:\n");

    // 遍历程序集
    for (const Il2CppApiWrapper::Il2CppAssembly& assembly : assemblies)
    {
        if (assembly.is_null()) continue;

        // 获取程序集对应的 image
        Il2CppApiWrapper::Il2CppImage image = assembly.get_image();
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

    Sleep(5000);
    InitIl2CppFunctions();
    TestPrintAllImageNames_Wrapper();
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
