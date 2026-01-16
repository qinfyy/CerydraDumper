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

//#include "Il2CppApi.h"

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

DWORD WINAPI MainThread(LPVOID) {
    DebugPrintA("[INFO] RuntimeDumper\n");
    DebugPrintA("[INFO] Waiting for GameAssembly.dll...\n");

    while (!GetModuleHandle(L"GameAssembly.dll")) {
        Sleep(200);
    }

    DebugPrintA("[INFO] GameAssembly.dll loaded, Starting dump ...\n");

    Sleep(5000);
    InitIl2CppFunctions();
    TestPrintAllImageNames();
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
