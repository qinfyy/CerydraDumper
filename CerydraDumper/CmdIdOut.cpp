#include "pch.h"
#include "CmdIdOut.h"
#include <iostream>
#include "CSharpRuntime.h"
#include "Il2CppApiWrapper.h"
#include "CModuleManager.h"
#include "CBaseModule.h"
#include "CRspHandler.h"
#include <algorithm>
#include <fstream>
#include <filesystem>

void AddPacket(CMonoAssembly& mono_assembly)
{
    DebugPrintA("[AddPacket] Add packet handlers ...\n");

    auto types = mono_assembly.GetTypes(60 != 0);

    for (size_t i = 0; i < types.length(); ++i)
    {
        uintptr_t runtime_ptr = types.get<uintptr_t>(i);
        CRuntimeType runtime_type = CRuntimeType(runtime_ptr);

        if (runtime_type.IsInterface()) {
            continue;
        }

        if (runtime_type.Name() == "GlobalVars") {
            auto fields = runtime_type.GetFields(60);

            for (size_t j = 0; j < fields.length(); ++j)
            {
                CMonoField field = fields.get<CMonoField>(j);

                if (field.Name() == "s_ModuleManager") {
                    auto module_manager_obj = CModuleManager(field.GetValue(0).raw_ptr());

                    auto list_module = module_manager_obj.Modules().ToVector<CBaseModule>();
					//DebugPrintA("[AddPacket] Found modules count: %zu\n", list_module.size());

                    for (size_t k = 0; k < list_module.size(); ++k)
                    {
                        auto optModule = list_module.at(k);
                        std::string className = CIl2CppObject(optModule.raw_ptr()).get_class().name();
                        //DebugPrintA("[HandlerAddPacket] <AddPacketHandlersChild> %s\n", className.c_str());
                        try
                        {
                            optModule.AddPacketHandlersChild();
                        }
                        catch (const std::exception& ex)
                        {
                            std::string className = CIl2CppObject(optModule.raw_ptr()).get_class().name();
                            //DebugPrintA("[HandlerAddPacket] <AddPacketHandlersParent> %s\n", className.c_str());
                            try
                            {
                                optModule.AddPacketHandlersParent();
                            }
                            catch (...)
                            {
                                std::string className = CIl2CppObject(optModule.raw_ptr()).get_class().name();
                                DebugPrintA("[AddPacket] Failed to add packet handlers %s\n", className.c_str());
                            }
                        }
                    }
                    continue;
                }

            }
        }
    }
    DebugPrintA("[AddPacket] Finished add packet handlers\n");
}

std::string NormalizeName(const std::string& name)
{
    if (name.rfind("_OnCmd", 0) == 0)
        return "Cmd" + name.substr(6);

    if (name.rfind("_On", 0) == 0)
        return "Cmd" + name.substr(3);

    if (name.rfind("_Cmd", 0) == 0)
        return "Cmd" + name.substr(4);

    if (name.rfind("Cmd", 0) == 0)
        return name;

    return name;
}

static std::unordered_map<std::string, uintptr_t> CMDID;

void DumpRespAndNotify(CMonoAssembly& mono_assembly, const char* path)
{
    DebugPrintA("[DumpRespAndNotify] Start Dump response and notify\n");

    auto types = mono_assembly.GetTypes(60 != 0);
    auto typesVec = types.to_vec<uintptr_t>();

    //DebugPrintA("[DumpRespAndNotify] 获取到类型数量: %zu\n", typesVec.size());

    for (uintptr_t runtime_ptr : typesVec)
    {
        if (!runtime_ptr)
            continue;

        CRuntimeType runtime_type(runtime_ptr);

        if (runtime_type.IsInterface())
            continue;

        if (runtime_type.Name().AsString() != "NotifyManager")
            continue;

        //DebugPrintA("[DumpRespAndNotify] 找到 NotifyManager 类型\n");

        auto fields = runtime_type.GetFields(60);
        auto fieldsVec = fields.to_vec<uintptr_t>();

        //DebugPrintA("[DumpRespAndNotify] NotifyManager 字段数量: %zu\n", fields.length());

        for (uintptr_t field_ptr : fieldsVec)
        {
            if (!field_ptr)
                continue;

            CMonoField field(field_ptr);

            if (field.Name() != "_RspHandlers")
                continue;

            //DebugPrintA("[DumpRespAndNotify]  找到字段 _RspHandlers\n");

            CIl2CppObject arr_obj = field.GetValue(0);
			uintptr_t arr_ptr = arr_obj.raw_ptr();
            if (arr_obj.is_null())
            {
                //DebugPrintA("[DumpRespAndNotify] [ERROR] _RspHandlers 为 null\n");
                continue;
            }

            //DebugPrintA("[DumpRespAndNotify] _RspHandlers 字段 %p\n", arr_ptr);
            CNativeArray<uintptr_t>* res_array = reinterpret_cast<CNativeArray<uintptr_t>*>(arr_ptr);

            //DebugPrintA("[DumpRespAndNotify] CNativeArray 数组长度: %d, CNativeArray 指针 %p\n", res_array->max_length, res_array);

            for (int k = 0; k < res_array->max_length; ++k)
            {
                uintptr_t dict_ptr = res_array->vector[k];
                CNativeDictionary<uintptr_t, uintptr_t>* res_dict = reinterpret_cast<CNativeDictionary<uintptr_t, uintptr_t>*>(dict_ptr);

                CNativeArray<CEntry<uintptr_t, uintptr_t>>* entries_array = res_dict->entries;
                if (!entries_array) continue;

                //DebugPrintA("[DumpRespAndNotify] Dictionary指针 %p, 处理 Dictionary[%d], entries指针 %p，entries 数量: %d\n", dict_ptr, k, res_dict->entries, res_dict->count);

                for (int l = 0; l < res_dict->count; ++l)
                {
                    auto& entry = res_dict->entries->vector[l];

                    std::vector<CRspHandler> handlers = CNativeList(entry.value).ToVector<CRspHandler>();

                    if (handlers.empty())
                        continue;

                    std::string selected;
                    std::string first_rsp_name;

                    for (auto& handler : handlers)
                    {
                        uintptr_t method_addr = *(reinterpret_cast<uintptr_t*>(handler.raw_ptr() + 0x28));

                        if (!method_addr)
                            continue;

                        CIl2CppMethod method(method_addr);
                        std::string raw_name = method.name();
                        std::string normalized = NormalizeName(raw_name);
                        //DebugPrintA("[DumpRespAndNotify] Handler 方法名: %s\n", raw_name.c_str());

                        bool all_upper = std::all_of(
                            raw_name.begin(), raw_name.end(),
                            [](char c) { return std::isupper(static_cast<unsigned char>(c)); });

                        if (normalized.rfind("Cmd", 0) == 0 && !all_upper)
                        {
                            selected = normalized;
                            break;
                        }

                        if (first_rsp_name.empty())
                            first_rsp_name = normalized;
                    }

                    std::string final_name;
                    if (!selected.empty())
                    {
                        final_name = selected;
                    }
                    else if (!first_rsp_name.empty())
                    {
                        if (first_rsp_name.rfind("Cmd", 0) != 0)
                            final_name = "Cmd" + first_rsp_name;
                        else
                            final_name = first_rsp_name;
                    }
                    else
                    {
                        final_name = "CmdUnknown";
                    }

                    CMDID[final_name] = entry.key;
					//DebugPrintA("[DumpRespAndNotify] %s -> %llu\n", final_name.c_str(), entry.key);
                }
            }
        }
    }

    std::filesystem::path filePath(path);
    std::filesystem::path directory = filePath.parent_path();
    if (!std::filesystem::exists(directory))
        std::filesystem::create_directories(directory);

    std::ofstream file(path);

    if (!file.is_open())
    {
        DebugPrintA("[ERROR] Failed to open file: %s\n", path);
    }
    else
    {
        file << "{\n";
        bool first = true;
        for (const auto& [name, key] : CMDID)
        {
            if (!first)
                file << ",\n";
            file << "  \"" << name << "\": " << key;
            first = false;
        }
        file << "\n}\n";
        file.close();

        DebugPrintA("[DumpRespAndNotify] cmdid written successfully, %zu items\n", CMDID.size());
    }

    DebugPrintA("[DumpRespAndNotify] Dump response complete\n");
}

void CmdIdDump(CMonoAssembly& mono_assembly, const char* path) {
    AddPacket(mono_assembly);
    DumpRespAndNotify(mono_assembly, path);
}
