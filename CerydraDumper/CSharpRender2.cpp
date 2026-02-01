#include "pch.h"
#include "CSharpRender2.h"
#include <sstream>
#include <fstream>
#include <filesystem>
#include <il2cpp-tabledefs.h>
#include "Il2CppFunctions.h"
#include "PrintHelper.h"
#include "Util.h"
#include "Il2CppApiWrapper.h"

static std::string GetTypeName(const CIl2CppType& type) {
    if (type.is_null())
        return "void";

    std::string typeName = type.formatted_name();
    if (typeName.empty())
        return "object";

    return typeName;
}

static bool IsFieldStatic(const CIl2CppField field) {
    return field.get_flags() & FIELD_ATTRIBUTE_STATIC;
}

static bool IsMethodStatic(const CIl2CppMethod method) {
    return (method.get_flags() & METHOD_ATTRIBUTE_STATIC) != 0;
}

static std::string ToBinary32(uint32_t value) {
    std::string result = "0b";
    for (int i = 31; i >= 0; --i) {
        result += ((value >> i) & 1) ? '1' : '0';
    }
    return result;
}

static void RenderDumper(std::ostringstream& os) {
    os << "// Create by CerydraDumper\n\n";

    CIl2CppDomain domain = CIl2CppDomain::get();
    if (domain.is_null()) return;

    auto assemblies = domain.assemblies();

    for (size_t i = 0; i < assemblies.size(); i++) {
		auto assembly = assemblies[i];
        CIl2CppImage image = assembly.get_image();

        if (image.is_null())
            continue;

        auto classes = image.classes();
        for (size_t c = 0; c < classes.size(); c++) {
            CIl2CppClass klass = classes[c];
            if (klass.is_null()) continue;

			std::string ns = klass.namespace_name();
            std::string img = image.name();
            std::string cls = klass.name();

            DebugPrintW(L"[DumpCs2] Dumping class: %ls\n", Utf8ToUtf16(cls).c_str());

            os << "namespace: " << ns << "\n";
            os << "Assembly: " << img << "\n";
            os << "class " << cls;
			auto parent = klass.get_parent();
            if (!parent.is_null()) {
                os << " : " << parent.name();
            }
            os << " {\n\n";

            auto fields = klass.fields();

            for (auto f = 0; f < fields.size(); f++) {
				auto field = fields[f];
				if (field.is_null()) 
                    continue;

                os << "\t0x" << std::hex << field.get_offset() << std::dec << " | ";
                if (IsFieldStatic(field)) {
                    os << "static ";
                }

                std::string typeName = GetTypeName(field.get_type());
                os << typeName << " " << field.name() << ";\n";
            }

            os << "\n";

			auto methods = klass.methods();
            for (auto m = 0; m < methods.size(); m++) {
				auto method = methods[m];

                uintptr_t rva = method.rva();

                int paramCount = method.param_count();

                os << "\t[Flags: " << ToBinary32(method.get_flags()) << "] [ParamsCount: "
                    << paramCount << "] |RVA: 0x"
                    << std::uppercase << std::hex << rva << std::dec << "|\n";

                os << "\t";
                if (IsMethodStatic(method)) os << "static ";
                std::string retType = GetTypeName(method.return_type());
                os << retType << " " << method.name() << "(";

                for (int p = 0; p < paramCount; ++p) {
                    std::string paramType = method.param_type_formatted(p);
                    std::string paramName = "arg" + std::to_string(p);

                    os << paramType << " " << paramName;
                    if (p + 1 < paramCount)
                        os << ", ";
                }

                os << ");\n\n";
            }

            os << "}\n\n";
        }
    }
}

void DumpCs2(const char* path) {
    std::filesystem::path filePath(path);
    std::filesystem::path directory = filePath.parent_path();
    if (!std::filesystem::exists(directory)) {
        std::filesystem::create_directories(directory);
    }

    std::ofstream file(path);
    if (file.is_open()) {
        DebugPrintA("[DumpCs2] dumping...\n");

        std::ostringstream ss;
        RenderDumper(ss);

        file << ss.str();
        file.close();
        DebugPrintA("[DumpCs2] dump done!\n");
    }
    else
    {
        DebugPrintA("[ERROR] Failed to open file for writing: %s\n", path);
    }
}
