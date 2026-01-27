#include "pch.h"
#include "Pb.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "CCSharpRuntime.h"
#include "PbUtil.h"
#include <optional>
#include "OriginalNameAttribute.h"
#include <regex>
#include <DbgHelp.h>
#include <iostream>

std::string dump_csharp_type(CRuntimeType t) {
    std::ostringstream out;

    std::string runtime_type_name = get_runtime_type_name(t, false);
    if (runtime_type_name.ends_with("Attribute")) {
        return "";
    }

    if (runtime_type_name.find('.') != std::string::npos) {
		return "";
    }

    std::optional<std::string> cmd_name;
    std::unordered_map<std::string, bool> skip_types;

    if (runtime_type_name.find('<') == std::string::npos) {
        auto fields = t.GetFields(60);

        std::ostringstream inner;
        auto is_enum_type = t.IsEnum();
        std::unordered_map<int, std::string> field_ids;
        auto idx = 0;
        if (fields.length() > 0) {
            for (size_t i = 0; i < fields.length(); ++i) {
                auto field = fields.get<CMonoField>(i);
				auto fieldType = field.FieldType();
                auto full_name_field = fieldType.FullName().AsString();

                auto d_field_name = field.DeclaringType().Name().AsString();
                std::optional<std::string> enum_key;
                if (d_field_name == t.Name().AsString()) {
                    auto attrs = field.GetCustomAttributes(true);
                    if (attrs.length() > 0) {
                        for (int j = 0; j < attrs.length(); j++) {
                            auto n_rt_p = attrs.get<uintptr_t>(j);
							auto n_rt = CIl2CppObject(n_rt_p);

                            auto name_rt = n_rt.get_class().name();
                            if (name_rt == "OriginalNameAttribute") {
                                is_enum_type = true;

                                enum_key = OriginalNameAttribute(n_rt.raw_ptr()).GetName().AsString();

                                if (cmd_name.has_value()) {
                                    const std::string& key = enum_key.value();
                                    //if let Some(key) = &enum_key {
                                    if (key.starts_with("CMD")) {
                                        std::regex re(R"(CMD(.*?)_NONE)");
                                        std::smatch m;
                                        if (std::regex_match(key, m, re)) {
                                            std::string lower = m[1].str();
                                            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

                                            // 等价于 Rust 的 r"_(\w)" + replace_all
                                            for (size_t i = 0; i + 1 < lower.size(); ) {
                                                if (lower[i] == '_' && std::isalnum((unsigned char)lower[i + 1])) {
                                                    lower[i + 1] = std::toupper((unsigned char)lower[i + 1]);
                                                    lower.erase(i, 1);
                                                }
                                                else {
                                                    ++i;
                                                }
                                            }

                                            cmd_name = "Cmd" + lower;
                                        }
                                    }
                                    else if (key.starts_with("Cmd")) {
                                        std::regex re(R"(Cmd(.*?)None)");
                                        std::smatch m;
                                        if (std::regex_match(key, m, re)) {
                                            cmd_name = "Cmd" + m[1].str();
                                        }
                                    }
                                    //}
                                }
                            }
                        }
                    }

                    //if attrs.len() == 0 && t.get_is_enum() ? && field.get_is_literal() ? {
                    //    is_enum_type = true;
                    //    enum_key = Some(
                    //        (t.get_name() ? .as_str() + "_" + field.get_name() ? .as_str()).to_string(),
                    //        );
                    //}

                    if (attrs.empty() && t.IsEnum() && field.IsLiteral()) {
                        is_enum_type = true;
                        enum_key = t.Name().AsString() + "_" + field.Name().AsString();
                    }
                }

                //if field.get_is_literal() ? {
                //    let int32_ptr = field.get_raw_constant_value() ? ;
                //    let n_int32 = SystemDynamic::new(int32_ptr);
                //    let value = n_int32.get_string() ? .as_str().to_string();
                //    field_ids.insert(idx, value.clone());
                //    idx += 1;
                //    if let Some(enum_key) = &enum_key{
                //        inner.push_str(&format!("\t{} = {};\n", enum_key, value.clone()));
                //    }
                //}

                if (field.IsLiteral()) {
                    uintptr_t value_ptr = field.RawConstantValue();
                    auto n_int32 = CSystemDynamic(value_ptr);
                    std::string value = n_int32.ToString().AsString();

                    field_ids[idx++] = value;

                    if (enum_key.has_value()) {
                        inner << "\t" << *enum_key << " = " << value << ";\n";
                    }
                }
                else if (full_name_field.find(d_field_name) != std::string::npos) {
                //else if contains(&full_name_field, &d_field_name) {
                    auto inner_fields = field.FieldType().GetFields(60);
                    auto count = count_occurrences(full_name_field, "+");

                    if (count == 1) {
                        //inner.push_str(&format!("\toneof {} {{\n", field.get_name() ? .as_str()));
                        inner << "\toneof " << field.Name().AsString() << " {\n";
                        //for j in 0..inner_fields.len() {
                        for (int j = 0; j < inner_fields.length(); j++) {
                            auto one_of_field_ptr = inner_fields.get<uintptr_t>(j);
                            CMonoField oneof_field(one_of_field_ptr);

                            //if one_of_field.get_is_literal() ? {
                            if (oneof_field.IsLiteral()) {
                                auto int32_ptr = oneof_field.RawConstantValue();
                                //auto value = SystemDynamic::new(int32_ptr)
                                //    .get_string() ?
                                //    .as_str()
                                //    .to_string();
                                std::string value = CSystemDynamic(oneof_field.RawConstantValue()).ToString().AsString();
                                //if value != "" && value != "0" {
                                //    inner.push_str("\t\tint32 ");
                                //    inner.push_str(&one_of_field.get_name() ? .as_str());
                                //    inner.push_str(&format!(" = {};", value));
                                //    inner.push('\n');
                                //}
                                if (!value.empty() && value != "0") {
                                    inner << "\t\tint32 " << oneof_field.Name().AsString() << " = " << value << ";\n";
                                }
                                /*skip_types
                                    .insert(one_of_field.get_name() ? .as_str().to_string(), true);*/
                                skip_types[oneof_field.Name().AsString()] = true;
                            }
                        }
                        inner << "\t}\n";
                    }
                    else if (count == 2) {
                        //inner.push_str(&format!(
                        //    "\tenum {} {{\n",
                        //    field.get_field_type() ? .get_name() ? .as_str()
                        //    ));
                        inner << "\tenum " << field.FieldType().Name().AsString() << " {\n";
                        //for j in 0..inner_fields.len() {
                        for (int j = 0; j < inner_fields.length(); j++) {
                            auto nested_field_ptr = inner_fields.get<uintptr_t>(j);
							CMonoField nested_field(nested_field_ptr);
                            auto attrs = nested_field.GetCustomAttributes(true);
                            if (attrs.length() > 0) {
                                //for k in 0..attrs.len() {
                                for (INT k = 0; k < attrs.length(); k++) {
                                    auto n_rt = attrs.get<CIl2CppObject>(k);
                                    if (n_rt.get_class().name() == "OriginalNameAttribute")
                                    {
                                        auto n_enum_key = OriginalNameAttribute(n_rt).GetName().AsString();
                                        if (nested_field.IsLiteral() ) {
                                            auto int32_ptr =
                                                nested_field.RawConstantValue();
                                            auto value = CSystemDynamic(int32_ptr).ToString().AsString();
                                            if (!value.empty()) {
                                                //if let Some(key) = n_enum_key{
                                                if (!n_enum_key.empty()) {
                                                    //inner.push_str("\t\t");
                                                    //inner.push_str(&key);
                                                    //inner.push_str(&format!(" = {};", value));
                                                    //inner.push('\n');
                                                    //skip_types.insert(key, true);
                                                    inner << "\t\t" << n_enum_key << " = " << value << ";\n";
                                                    skip_types[n_enum_key] = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            if (attrs.length() == 0 && nested_field.IsLiteral() ) {
                                //auto n_enum_key = Some(
                                //    (field.get_field_type() ? .get_name() ? .as_str()
                                //        + "_"
                                //        + nested_field.get_name() ? .as_str())
                                //    .to_string(),
                                //    );
								auto n_enum_key = field.FieldType().Name().AsString() + "_" + nested_field.Name().AsString();
                                auto int32_ptr = nested_field.RawConstantValue();
                                auto value = CSystemDynamic(int32_ptr).ToString().AsString();
                                if (value != "") {
                                    if (!n_enum_key.empty()){
                                        //inner.push_str("\t\t");
                                        //inner.push_str(&key);
                                        //inner.push_str(&format!(" = {};", value));
                                        //inner.push('\n');
                                        //skip_types.insert(key, true);
                                        inner << "\t\t" << n_enum_key << " = " << value << ";\n";
                                        skip_types[n_enum_key] = true;
                                    }
                                }
                            }
                        }
                        inner << "\t}\n";
                    }
                }
            }
        }

        out << "\n";

        //if let Some(cmd_name) = &cmd_name{
        //    out.push_str(&format!("// {}\n", runtime_type_name));
        //    out.push_str(&format!("enum {}", cmd_name));
        //}
        //else if is_enum_type {
        //    out.push_str(&format!("enum {}", runtime_type_name));
        //}
        //else {
        //    out.push_str(&format!("message {}", runtime_type_name));
        //}


        if (cmd_name.has_value()) {
            out << "// " << runtime_type_name << "\n";
            out << "enum " << *cmd_name;
        }
        else if (is_enum_type) {
            out << "enum " << runtime_type_name;
        }
        else {
            out << "message " << runtime_type_name;
        }

        out << " {\n";
        out << inner.str();

        auto properties = t.GetProperties(28);
        if (properties.length() > 0) {
            //for j in 0..properties.len() {
				for (int j = 0; j < properties.length(); j++) {
                auto property_ptr = properties.get<uintptr_t>(j);
                auto property = CMonoProperty(property_ptr);
                auto d_property_name = property.DeclaringType().Name().AsString();
                if (d_property_name == t.Name().AsString()) {
                    auto it = field_ids.find(static_cast<int32_t>(j));
                    if (it != field_ids.end()) {
                        auto type_Name = get_runtime_type_name(property.PropertyType(), true);
                        //if type_Name.contains('.') {
                        //    if let Some(capture) = typename.rsplit('.').next() {
                        //        typename = capture.to_string();
                        //    }
                        //}
                        if (type_Name.find('.') != std::string::npos) {
                            auto pos = type_Name.rfind('.');
                            if (pos != std::string::npos) {
                                type_Name = type_Name.substr(pos + 1);
                            }
                        }
                        //for (pattern, replacement) in& get_field_type_map() {
                        //    typename = typename.replace(pattern, replacement);
                        //}
                        for (auto& [k, v] : get_field_type_map())
                            type_Name = replace_all(type_Name, k, v);

                        //if type_Name.contains("repeated") {
                        //    type_Name = type_Name.replace(">", "");
                        //}


                        if (type_Name.find("repeated") != std::string::npos)
                            type_Name = replace_all(type_Name, ">", "");

                        //if skip_types.contains_key(&property.get_name() ? .as_str().to_string()) {
                        //    let pattern = format!("int32 {}", property.get_name() ? .as_str());
                        //    let replacement =
                        //        format!("{} {}", typename, property.get_name() ? .as_str());
                        //    out = out.replace(&pattern, &replacement);
                        //}
                        //else if field_id != "0" {
                        //    out += &format!(
                        //        "\t{} {} = {};\n",
                        //        typename,
                        //        property.get_name() ? .as_str(),
                        //        field_id
                        //        );
                        //}

                        if (skip_types.count(property.Name().AsString())) {
                            std::string from = "int32 " + property.Name().AsString();
                            std::string to = type_Name + " " + property.Name().AsString();
                            out.str(replace_all(out.str(), from, to));
                        }
                        else if (it->second != "0") {
                            out << "\t" << type_Name
                                << " " << property.Name().AsString()
                                << " = " << it->second << ";\n";
                        }
                    }
                }
            }
        }

        out << "}\n";
    }

    return out.str();
}

static void WriteFullDump(EXCEPTION_POINTERS* ep)
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

static LONG WINAPI MyVehHandler(PEXCEPTION_POINTERS ep)
{
    std::cerr << "[VEH] Caught exception at address: 0x"
        << std::hex << ep->ExceptionRecord->ExceptionAddress << std::dec << std::endl;
	WriteFullDump(ep);

    return EXCEPTION_CONTINUE_SEARCH;
}


void proto_dump(CMonoAssembly mono_assembly)
{
    DebugPrintA("[ProtoDump] Dumping raw proto...\n");

    std::ofstream ofs("dump.proto", std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) {
        DebugPrintA("[ProtoDump] Failed to open dump.proto\n");
        return;
    }

    std::ostringstream final_str;

	final_str << "// Create by Cerydra Dumper.\n";
    final_str << "syntax = \"proto3\";\n";

    auto types = mono_assembly.GetTypes(60 != 0);

    DebugPrintA("[ProtoDump] Enumerating runtime types...\n");

    for (size_t i = 0; i < types.length(); ++i) {
        uintptr_t type_ptr = types.get<uintptr_t>(i);
        CRuntimeType runtime_type(type_ptr);

        // 跳过泛型类型（和 Rust 一致）
        if (runtime_type.IsGenericType())
            continue;

        std::string full_name = runtime_type.FullName().AsString();
        DebugPrintA(("[ProtoDump] Processing: " + full_name + "\n").c_str());

        try {
            //PVOID handler = AddVectoredExceptionHandler(1, MyVehHandler);
            //if (!handler)
            //    std::cerr << "Failed to add VEH handler!" << std::endl;

            std::string output = dump_csharp_type(runtime_type);
            if (!output.empty()) {
                final_str << output << "\n";

                // 每个 proto 打印
                DebugPrintA("[ProtoDump] Class: %s\n");
                DebugPrintA(output.c_str());
                DebugPrintA("\n");
            }
        }
        catch (const std::exception& e) {
            std::string err = "[ProtoDump] Exception in type: " + full_name +
                " | " + e.what() + "\n";
            DebugPrintA(err.c_str());
        }
        //catch (...) {
        //    std::string err = "[ProtoDump] Unknown exception in type: " + full_name + "\n";
        //    DebugPrintA(err.c_str());
        //}
    }

    ofs << final_str.str();
    ofs.close();

    DebugPrintA("[ProtoDump] Finished dumping raw proto.\n");
}
