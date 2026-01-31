#include "pch.h"
#include "ProtoDumper.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "CSharpRuntime.h"
#include "PbUtil.h"
#include <optional>
#include "OriginalNameAttribute.h"
#include <regex>
#include <DbgHelp.h>
#include <iostream>
#include <filesystem>

std::string DumpCsharpType(CRuntimeType t) {
    std::ostringstream out;

    std::string runtime_type_name = GetRuntimeTypeName(t, false);
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
                                    if (key.starts_with("CMD")) {
                                        std::regex re(R"(CMD(.*?)_NONE)");
                                        std::smatch m;
                                        if (std::regex_match(key, m, re)) {
                                            std::string lower = m[1].str();
                                            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

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
                                }
                            }
                        }
                    }

                    if (attrs.empty() && t.IsEnum() && field.IsLiteral()) {
                        is_enum_type = true;
                        enum_key = t.Name().AsString() + "_" + field.Name().AsString();
                    }
                }

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
                    auto inner_fields = field.FieldType().GetFields(60);
                    auto count = CountOccurrences(full_name_field, "+");

                    if (count == 1) {
                        inner << "\toneof " << field.Name().AsString() << " {\n";
                        for (int j = 0; j < inner_fields.length(); j++) {
                            auto one_of_field_ptr = inner_fields.get<uintptr_t>(j);
                            CMonoField oneof_field(one_of_field_ptr);

                            if (oneof_field.IsLiteral()) {
                                auto int32_ptr = oneof_field.RawConstantValue();
                                std::string value = CSystemDynamic(int32_ptr).ToString().AsString();

                                if (!value.empty() && value != "0") {
                                    inner << "\t\tint32 " << oneof_field.Name().AsString() << " = " << value << ";\n";
                                }

                                skip_types[oneof_field.Name().AsString()] = true;
                            }
                        }
                        inner << "\t}\n";
                    }
                    else if (count == 2) {
                        inner << "\tenum " << field.FieldType().Name().AsString() << " {\n";
                        for (int j = 0; j < inner_fields.length(); j++) {
                            auto nested_field_ptr = inner_fields.get<uintptr_t>(j);
                            CMonoField nested_field(nested_field_ptr);
                            auto attrs = nested_field.GetCustomAttributes(true);
                            if (attrs.length() > 0) {
                                for (INT k = 0; k < attrs.length(); k++) {
                                    auto n_rt = attrs.get<CIl2CppObject>(k);
                                    if (n_rt.get_class().name() == "OriginalNameAttribute")
                                    {
                                        auto n_enum_key = OriginalNameAttribute(n_rt).GetName().AsString();
                                        if (nested_field.IsLiteral()) {
                                            auto int32_ptr =
                                                nested_field.RawConstantValue();
                                            auto value = CSystemDynamic(int32_ptr).ToString().AsString();
                                            if (!value.empty()) {
                                                if (!n_enum_key.empty()) {
                                                    inner << "\t\t" << n_enum_key << " = " << value << ";\n";
                                                    skip_types[n_enum_key] = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            if (attrs.length() == 0 && nested_field.IsLiteral()) {
                                auto n_enum_key = field.FieldType().Name().AsString() + "_" + nested_field.Name().AsString();
                                auto int32_ptr = nested_field.RawConstantValue();
                                auto value = CSystemDynamic(int32_ptr).ToString().AsString();
                                if (value != "") {
                                    if (!n_enum_key.empty()) {
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
            for (int j = 0; j < properties.length(); j++) {
                auto property_ptr = properties.get<uintptr_t>(j);
                auto property = CMonoProperty(property_ptr);
                auto d_property_name = property.DeclaringType().Name().AsString();
                if (d_property_name == t.Name().AsString()) {
                    auto it = field_ids.find(static_cast<int32_t>(j));
                    if (it != field_ids.end()) {
                        auto type_Name = GetRuntimeTypeName(property.PropertyType(), true);
                        if (type_Name.find('.') != std::string::npos) {
                            auto pos = type_Name.rfind('.');
                            if (pos != std::string::npos) {
                                type_Name = type_Name.substr(pos + 1);
                            }
                        }
                        for (auto& [k, v] : GetFieldTypeMap()) {
                            type_Name = ReplaceAll(type_Name, k, v);
                        }

                        if (type_Name.find("repeated") != std::string::npos) {
                            type_Name = ReplaceAll(type_Name, ">", "");
                        }

                        if (skip_types.count(property.Name().AsString())) {
                            std::string from = "int32 " + property.Name().AsString();
                            std::string to = type_Name + " " + property.Name().AsString();
                            // 我cnm的
                            std::string tmp = ReplaceAll(out.str(), from, to);
                            out.str("");
                            out << tmp;
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

void ProtoDump(CMonoAssembly mono_assembly, const char* path)
{
    DebugPrintA("[ProtoDump] Dumping proto ...\n");

    std::filesystem::path filePath(path);
    std::filesystem::path directory = filePath.parent_path();
    if (!std::filesystem::exists(directory))
        std::filesystem::create_directories(directory);

    std::ofstream ofs(path, std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) {
        DebugPrintA("[ERROR] Failed to open file: %s\n", path);
        return;
    }

    std::ostringstream final_str;

    final_str << "// Create by CerydraDumper\n\n";
    final_str << "syntax = \"proto3\";\n";

    auto types = mono_assembly.GetTypes(60 != 0);

    DebugPrintA("[ProtoDump] Enumerating runtime types ...\n");

    for (size_t i = 0; i < types.length(); ++i) {
        uintptr_t type_ptr = types.get<uintptr_t>(i);
        CRuntimeType runtime_type(type_ptr);

        if (runtime_type.IsGenericType())
            continue;

        std::string full_name = runtime_type.FullName().AsString();
        DebugPrintA(("[ProtoDump] Dumping: " + full_name + "\n").c_str());

        try {
            std::string output = DumpCsharpType(runtime_type);
            if (!output.empty()) {
                final_str << output;
            }
        }
        catch (const std::exception& e) {
            std::string err = "[ProtoDump] Exception in type: " + full_name +
                " | " + e.what() + "\n";
            DebugPrintA(err.c_str());
        }
        catch (...) {
            std::string err = "[ProtoDump] Unknown exception in type: " + full_name + "\n";
            DebugPrintA(err.c_str());
        }
    }

    ofs << final_str.str();
    ofs.close();

    DebugPrintA("[ProtoDump] Dump done.\n");
}
