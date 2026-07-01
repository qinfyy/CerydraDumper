#include "pch.h"
#include "Il2CppDumper.h"
#include "Il2CppRuntimeCache.h"
#include "PrintHelper.h"
#include <filesystem>
#include <fstream>
#include <il2cpp-tabledefs.h>
#include <sstream>

namespace
{
    std::string TrimCsType(const std::string& type)
    {
        std::string result;
        std::string token;

        auto flushToken = [&]() {
            if (token.empty()) {
                return;
            }

            const auto pos = token.find_last_of('.');
            result += pos != std::string::npos ? token.substr(pos + 1) : token;
            token.clear();
        };

        for (size_t i = 0; i < type.size(); ++i) {
            const char ch = type[i];
            if (ch == '<') {
                flushToken();
                result += '<';
            }
            else if (ch == '>') {
                flushToken();
                result += '>';
            }
            else if (ch == ',') {
                flushToken();
                result += ", ";
            }
            else if (ch == '[' && i + 1 < type.size() && type[i + 1] == ']') {
                flushToken();
                result += "[]";
                ++i;
            }
            else {
                token += ch;
            }
        }

        flushToken();
        return result;
    }

    std::string GetTypeName(const Cerydra::IL2CPP::Type* type)
    {
        if (!type) {
            return "void";
        }

        auto typeName = type->DisplayName();
        if (typeName.empty()) {
            return "object";
        }

        return TrimCsType(typeName);
    }

    std::string GetClassModifier(const Cerydra::IL2CPP::Class* klass)
    {
        std::ostringstream output;
        const auto flags = klass ? klass->flags : 0;

        if (flags & TYPE_ATTRIBUTE_SERIALIZABLE) {
            output << "[Serializable]\n";
        }

        switch (flags & TYPE_ATTRIBUTE_LAYOUT_MASK) {
        case TYPE_ATTRIBUTE_SEQUENTIAL_LAYOUT:
            output << "[StructLayout(LayoutKind.Sequential)]\n";
            break;
        case TYPE_ATTRIBUTE_EXPLICIT_LAYOUT:
            output << "[StructLayout(LayoutKind.Explicit)]\n";
            break;
        default:
            break;
        }

        switch (flags & TYPE_ATTRIBUTE_VISIBILITY_MASK) {
        case TYPE_ATTRIBUTE_PUBLIC:
        case TYPE_ATTRIBUTE_NESTED_PUBLIC:
            output << "public ";
            break;
        case TYPE_ATTRIBUTE_NOT_PUBLIC:
        case TYPE_ATTRIBUTE_NESTED_FAM_AND_ASSEM:
        case TYPE_ATTRIBUTE_NESTED_ASSEMBLY:
            output << "internal ";
            break;
        case TYPE_ATTRIBUTE_NESTED_PRIVATE:
            output << "private ";
            break;
        case TYPE_ATTRIBUTE_NESTED_FAMILY:
            output << "protected ";
            break;
        case TYPE_ATTRIBUTE_NESTED_FAM_OR_ASSEM:
            output << "protected internal ";
            break;
        default:
            break;
        }

        if ((flags & TYPE_ATTRIBUTE_ABSTRACT) && (flags & TYPE_ATTRIBUTE_SEALED)) {
            output << "static ";
        }
        else if (!(flags & TYPE_ATTRIBUTE_INTERFACE) && (flags & TYPE_ATTRIBUTE_ABSTRACT)) {
            output << "abstract ";
        }
        else if (klass && !klass->isValueType && !klass->isEnum && (flags & TYPE_ATTRIBUTE_SEALED)) {
            output << "sealed ";
        }

        if (klass && klass->isInterface) {
            output << "interface ";
        }
        else if (klass && klass->isEnum) {
            output << "enum ";
        }
        else if (klass && klass->isValueType) {
            output << "struct ";
        }
        else {
            output << "class ";
        }

        return output.str();
    }

    std::string GetFieldModifier(const Cerydra::IL2CPP::Field* field)
    {
        std::string result;
        const auto flags = field ? static_cast<uint16_t>(field->flags) : 0;

        switch (flags & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK) {
        case FIELD_ATTRIBUTE_PRIVATE:
            result += "private ";
            break;
        case FIELD_ATTRIBUTE_PUBLIC:
            result += "public ";
            break;
        case FIELD_ATTRIBUTE_FAMILY:
            result += "protected ";
            break;
        case FIELD_ATTRIBUTE_ASSEMBLY:
        case FIELD_ATTRIBUTE_FAM_AND_ASSEM:
            result += "internal ";
            break;
        case FIELD_ATTRIBUTE_FAM_OR_ASSEM:
            result += "protected internal ";
            break;
        default:
            break;
        }

        if (flags & FIELD_ATTRIBUTE_LITERAL) {
            result += "const ";
        }
        else {
            if (flags & FIELD_ATTRIBUTE_STATIC) {
                result += "static ";
            }
            if (flags & FIELD_ATTRIBUTE_INIT_ONLY) {
                result += "readonly ";
            }
        }

        return result;
    }

    std::string GetMethodModifiers(uint16_t flags)
    {
        std::string result;

        switch (flags & METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK) {
        case METHOD_ATTRIBUTE_PRIVATE:
            result += "private ";
            break;
        case METHOD_ATTRIBUTE_PUBLIC:
            result += "public ";
            break;
        case METHOD_ATTRIBUTE_FAMILY:
            result += "protected ";
            break;
        case METHOD_ATTRIBUTE_ASSEM:
        case METHOD_ATTRIBUTE_FAM_AND_ASSEM:
            result += "internal ";
            break;
        case METHOD_ATTRIBUTE_FAM_OR_ASSEM:
            result += "protected internal ";
            break;
        default:
            break;
        }

        if (flags & METHOD_ATTRIBUTE_STATIC) result += "static ";
        if (flags & METHOD_ATTRIBUTE_ABSTRACT) {
            result += "abstract ";
            if ((flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_REUSE_SLOT) {
                result += "override ";
            }
        }
        else if (flags & METHOD_ATTRIBUTE_FINAL) {
            if ((flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_REUSE_SLOT) {
                result += "sealed override ";
            }
        }
        else if (flags & METHOD_ATTRIBUTE_VIRTUAL) {
            result += (flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_NEW_SLOT ? "virtual " : "override ";
        }
        if (flags & METHOD_ATTRIBUTE_PINVOKE_IMPL) result += "extern ";

        return result;
    }

    void DumpHeader(std::ostream& os)
    {
        uint32_t runningOffset = 0;
        const auto& assemblies = Il2CppRuntimeCache::Assemblies();
        for (size_t i = 0; i < assemblies.size(); ++i) {
            const auto* assembly = assemblies[i];
            if (!assembly || !assembly->image) {
                continue;
            }

            os << "// Image " << i << ": " << assembly->image->name
                << " - TypeDefIndexOffset: " << runningOffset << "\n";
            runningOffset += static_cast<uint32_t>(assembly->image->classes.size());
        }

        os << "\n";
    }

    void DumpFields(std::ostream& os, const Cerydra::IL2CPP::Class* klass)
    {
        os << "\t// Fields\n";
        for (const auto* field : klass->fields) {
            if (!field) {
                continue;
            }

            os << "\t" << GetFieldModifier(field)
                << GetTypeName(field->type) << " " << field->name
                << "; // 0x" << std::hex << field->offset << std::dec << "\n";
        }
        os << "\n";
    }

    void DumpMethods(std::ostream& os, const Cerydra::IL2CPP::Class* klass)
    {
        os << "\t// Methods\n\n";
        for (size_t i = 0; i < klass->methods.size(); ++i) {
            const auto* method = klass->methods[i];
            if (!method) {
                continue;
            }

            os << std::uppercase << "\t// RVA: 0x" << std::hex << method->Rva()
                << " VA: 0x" << method->Va() << std::dec << " // Slot: " << i << "\n";

            os << "\t" << GetMethodModifiers(static_cast<uint16_t>(method->flags));
            if (method->returnType && method->returnType->byRef) {
                os << "ref ";
            }

            os << GetTypeName(method->returnType) << " " << method->name << "(";
            for (size_t p = 0; p < method->args.size(); ++p) {
                auto* arg = method->args[p];
                os << GetTypeName(arg ? arg->type : nullptr) << " " << (arg ? arg->name : "arg" + std::to_string(p));
                if (p + 1 < method->args.size()) {
                    os << ", ";
                }
            }

            os << ") { }\n\n";
        }
    }

    void DumpClass(std::ostream& os, const Cerydra::IL2CPP::Class* klass, size_t typeIndex, const std::string& imageName)
    {
        if (!klass) {
            return;
        }

        DebugPrintA("[DumpCs] Dumping class: %s\n", klass->name.c_str());
        os << "// Assembly: " << imageName << "\n";
        os << "// Namespace: " << klass->namespaze << "\n";
        os << GetClassModifier(klass) << klass->name;

        if (!klass->isValueType && klass->parentClass && klass->parentClass->name != "Object") {
            os << " : " << klass->parentClass->name;
        }

        os << " // TypeDefIndex: " << typeIndex << "\n{\n";
        DumpFields(os, klass);
        DumpMethods(os, klass);
        os << "}\n\n";
    }

    void DumpClasses(std::ostream& os)
    {
        for (const auto* assembly : Il2CppRuntimeCache::Assemblies()) {
            if (!assembly || !assembly->image) {
                continue;
            }

            const auto& classes = assembly->image->classes;
            for (size_t i = 0; i < classes.size(); ++i) {
                DumpClass(os, classes[i], i, assembly->image->name);
            }
        }
    }
}

void DumpCs(const char* path)
{
    DebugPrintA("[DumpCs] Start dumping ...\n");

    std::filesystem::path filePath(path);
    const auto directory = filePath.parent_path();
    if (!directory.empty() && !std::filesystem::exists(directory)) {
        std::filesystem::create_directories(directory);
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        DebugPrintA("[ERROR] 打开文件失败: %s\n", path);
        return;
    }

    std::ostringstream output;
    output << "// Create by CerydraDumper\n\n";
    DumpHeader(output);
    DumpClasses(output);

    file << output.str();
    DebugPrintA("[DumpCs] Dump done.\n");
}
