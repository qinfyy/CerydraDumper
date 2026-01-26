#include "pch.h"
#include "CmdIdOut.h"
#include <iostream>
#include "CCSharpRuntime.h"
#include "Il2CppApiWrapper.h"
#include "ModuleManager.h"
#include "CBaseModule.h"
#include "CRspHandler.h"
#include <algorithm>
#include <fstream>

void HandlerAddPacket(CMonoAssembly& mono_assembly)
{
    std::cout << "[AddPacket] Add packet handlers...\n";

    auto types = mono_assembly.GetTypes(60 != 0);

    for (size_t i = 0; i < types.length(); ++i)
    {
        uintptr_t runtime_ptr = types.get<uintptr_t>(i);
        CRuntimeType runtime_type = CRuntimeType(runtime_ptr);

        // if runtime_type.get_is_interface()?
        if (runtime_type.IsInterface()) {
            continue;
        }

        // if runtime_type.get_name()? == "GlobalVars"
        if (runtime_type.Name() == "GlobalVars") {
            auto fields = runtime_type.GetFields(60);

            for (size_t j = 0; j < fields.length(); ++j)
            {
                CMonoField field = fields.get<CMonoField>(j);

                if (field.Name() == "s_ModuleManager") {
                    // static field → obj = nullptr / 0
                    auto module_manager_obj = CModuleManager(field.GetValue(0).raw_ptr());

                    // modules()?.to_vec::<BaseModule>()
                    auto list_module = module_manager_obj.Modules().ToVector<CBaseModule>();

                    for (size_t k = 0; k < list_module.size(); ++k)
                    {
                        auto optModule = list_module.at(k);
                        std::string className = CIl2CppObject(optModule.raw_ptr()).get_class().name();
                        DebugPrintA("[HandlerAddPacket] <AddPacketHandlersChild> %s\n", className.c_str());
                        try
                        {
                            optModule.AddPacketHandlersChild();
                        }
                        catch (...)
                        {
                            std::string className = CIl2CppObject(optModule.raw_ptr()).get_class().name();
                            DebugPrintA("[HandlerAddPacket] <AddPacketHandlersParent> %s\n", className.c_str());
                            try
                            {
                                optModule.AddPacketHandlersParent();
                            }
                            catch (...)
                            {
                                std::string className = CIl2CppObject(optModule.raw_ptr()).get_class().name();
                                DebugPrintA("❌ Failed to add packet handlers %s\n", className.c_str());
                            }
                        }
                    }
                    continue;
                }

            }
        }
    }
    std::cout << "[AddPacket] Finished add packet handlers...\n";
}

std::string NormalizeName(const std::string& name)
{
    if (name.rfind("_OnCmd", 0) == 0) // starts_with("_OnCmd")
        return "Cmd" + name.substr(6);

    if (name.rfind("_On", 0) == 0) // starts_with("_On")
        return "Cmd" + name.substr(3);

    if (name.rfind("_Cmd", 0) == 0) // starts_with("_Cmd")
        return "Cmd" + name.substr(4);

    if (name.rfind("Cmd", 0) == 0) // starts_with("Cmd")
        return name;

    return name; // 原样返回
}

// 全局 CMDID
static std::unordered_map<std::string, uintptr_t> CMDID;

void HandlerDumpRespAndNotify(CMonoAssembly& mono_assembly)
{
    DebugPrintA("[HandlerDumpRespAndNotify] 开始 Dump response 并构建 CMDID\n");

    auto types = mono_assembly.GetTypes(60 != 0);
    auto typesVec = types.to_vec<uintptr_t>();

    DebugPrintA("[HandlerDumpRespAndNotify] 获取到类型数量: %zu\n", typesVec.size());

    for (uintptr_t runtime_ptr : typesVec)
    {
        if (!runtime_ptr)
            continue;

        CRuntimeType runtime_type(runtime_ptr);

        if (runtime_type.IsInterface())
            continue;

        if (runtime_type.Name().AsString() != "NotifyManager")
            continue;

        DebugPrintA("[HandlerDumpRespAndNotify] 找到 NotifyManager 类型\n");

        auto fields = runtime_type.GetFields(60);
        auto fieldsVec = fields.to_vec<uintptr_t>();

        DebugPrintA("[HandlerDumpRespAndNotify] NotifyManager 字段数量: %zu\n", fields.length());

        for (uintptr_t field_ptr : fieldsVec)
        {
            if (!field_ptr)
                continue;

            CMonoField field(field_ptr);

            if (field.Name() != "_RspHandlers")
                continue;

            DebugPrintA("[HandlerDumpRespAndNotify]  找到字段 _RspHandlers\n");

            CIl2CppObject arr_obj = field.GetValue(0);
			uintptr_t arr_ptr = arr_obj.raw_ptr();
            if (arr_obj.is_null())
            {
                DebugPrintA("[HandlerDumpRespAndNotify] [ERROR] _RspHandlers 为 null\n");
                continue;
            }

            DebugPrintA("[HandlerDumpRespAndNotify] _RspHandlers 字段 %p\n", arr_ptr);
            CNativeArray<uintptr_t>* res_array = reinterpret_cast<CNativeArray<uintptr_t>*>(arr_ptr);

            DebugPrintA("[HandlerDumpRespAndNotify] CNativeArray 数组长度: %d, CNativeArray 指针 %p\n", res_array->max_length, res_array);

            for (int k = 0; k < res_array->max_length; ++k)
            {
                uintptr_t dict_ptr = res_array->vector[k];
                CNativeDictionary<uintptr_t, uintptr_t>* res_dict = reinterpret_cast<CNativeDictionary<uintptr_t, uintptr_t>*>(dict_ptr);

                CNativeArray<CEntry<uintptr_t, uintptr_t>>* entries_array = res_dict->entries;
                if (!entries_array) continue;

                DebugPrintA("[HandlerDumpRespAndNotify] Dictionary指针 %p, 处理 Dictionary[%d], entries指针 %p，entries 数量: %d\n", dict_ptr, k, res_dict->entries, res_dict->count);

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
                        DebugPrintA("[HandlerDumpRespAndNotify] Handler 方法名: %s\n", raw_name.c_str());

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
                }
            }
        }
    }

    std::ofstream file("cmdid.json");
    if (!file.is_open())
    {
        DebugPrintA("[HandlerDumpRespAndNotify] [ERROR] 无法写入 cmdid.json\n");
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

        DebugPrintA("[HandlerDumpRespAndNotify] cmdid.json 写入成功，共 %zu 条\n", CMDID.size());
    }

    DebugPrintA("[HandlerDumpRespAndNotify] Dump response 完成\n");
}

void CmdIdDump(CMonoAssembly& mono_assembly) {
    HandlerAddPacket(mono_assembly);
    HandlerDumpRespAndNotify(mono_assembly);
}
