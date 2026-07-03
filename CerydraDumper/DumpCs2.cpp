#include "pch.h"
#include "DumpCs2.h"
#include "Il2CppRuntimeCache.h"
#include "PrintHelper.h"
#include <filesystem>
#include <fstream>
#include <il2cpp-tabledefs.h>
#include <sstream>

namespace
{
    std::string GetTypeName(const Cerydra::IL2CPP::Type* type)
    {
        if (!type) {
            return "void";
        }

        auto name = type->DisplayName();
        return name.empty() ? "object" : name;
    }

    bool IsFieldStatic(const Cerydra::IL2CPP::Field* field)
    {
        return field && (field->flags & FIELD_ATTRIBUTE_STATIC);
    }

    bool IsMethodStatic(const Cerydra::IL2CPP::Method* method)
    {
        return method && (method->flags & METHOD_ATTRIBUTE_STATIC);
    }

    std::string ToBinary32(uint32_t value)
    {
        std::string result = "0b";
        for (int i = 31; i >= 0; --i) {
            result += ((value >> i) & 1) ? '1' : '0';
        }
        return result;
    }

    void RenderClass(std::ostringstream& os, const Cerydra::IL2CPP::Class* klass)
    {
        if (!klass) {
            return;
        }

        DebugPrintA("[DumpCs2] Dumping class: %s\n", klass->name.c_str());

        os << "namespace: " << klass->namespaze << "\n";
        os << "Assembly: " << (klass->image ? klass->image->name : "") << "\n";
        os << "class " << klass->name;
        if (klass->parentClass) {
            os << " : " << klass->parentClass->name;
        }
        os << " {\n\n";

        for (const auto* field : klass->fields) {
            if (!field) {
                continue;
            }

            os << "\t0x" << std::hex << field->offset << std::dec << " | ";
            if (IsFieldStatic(field)) {
                os << "static ";
            }
            os << GetTypeName(field->type) << " " << field->name;
            const auto literal = field->LiteralValue();
            if (!literal.empty()) {
                os << " = " << literal;
            }
            os << ";\n";
        }

        os << "\n";

        for (const auto* method : klass->methods) {
            if (!method) {
                continue;
            }

            os << "\t[Flags: " << ToBinary32(method->flags)
                << "] [ParamsCount: " << method->args.size()
                << "] |RVA: 0x" << std::uppercase << std::hex << method->Rva() << std::dec << "|\n";

            os << "\t";
            if (IsMethodStatic(method)) {
                os << "static ";
            }
            os << GetTypeName(method->returnType) << " " << method->name << "(";
            for (size_t i = 0; i < method->args.size(); ++i) {
                auto* arg = method->args[i];
                os << method->ParamModifier(i)
                    << GetTypeName(arg ? arg->type : nullptr) << " "
                    << (arg ? arg->DisplayName(i) : "arg" + std::to_string(i + 1));
                if (i + 1 < method->args.size()) {
                    os << ", ";
                }
            }
            os << ");\n\n";
        }

        os << "}\n\n";
    }

    void Render(std::ostringstream& os)
    {
        os << "// Create by CerydraDumper\n\n";

        for (const auto* assembly : Il2CppRuntimeCache::Assemblies()) {
            if (!assembly || !assembly->image) {
                continue;
            }

            for (const auto* klass : assembly->image->classes) {
                RenderClass(os, klass);
            }
        }
    }
}

void DumpCs2(const char* path)
{
    const std::filesystem::path filePath(path);
    const auto directory = filePath.parent_path();
    if (!directory.empty() && !std::filesystem::exists(directory)) {
        std::filesystem::create_directories(directory);
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        DebugPrintA("[ERROR] 打开文件失败: %s\n", path);
        return;
    }

    DebugPrintA("[DumpCs2] dumping...\n");
    std::ostringstream output;
    Render(output);
    file << output.str();
    DebugPrintA("[DumpCs2] dump done!\n");
}
