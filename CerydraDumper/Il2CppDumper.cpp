#include "pch.h"
#include "Il2CppApiWrapper.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <il2cpp-tabledefs.h>
#include "PrintHelper.h"

std::string GetClassModifier(CIl2CppClass klass)
{
    std::ostringstream outPut;
    auto flags = klass.get_flags();

    if (flags & TYPE_ATTRIBUTE_SERIALIZABLE) {
        outPut << "[Serializable]\n";
    }

    auto layout = flags & TYPE_ATTRIBUTE_LAYOUT_MASK;
    switch (layout) {
    case TYPE_ATTRIBUTE_AUTO_LAYOUT:
        break;
    case TYPE_ATTRIBUTE_SEQUENTIAL_LAYOUT:
        outPut << "[StructLayout(LayoutKind.Sequential)]\n";
        break;
    case TYPE_ATTRIBUTE_EXPLICIT_LAYOUT:
        outPut << "[StructLayout(LayoutKind.Explicit)]\n";
        break;
    }

    auto is_valuetype = klass.is_value_type();
    bool is_enum = klass.is_enum();

    auto visibility = flags & TYPE_ATTRIBUTE_VISIBILITY_MASK;
    switch (visibility) {
    case TYPE_ATTRIBUTE_PUBLIC:
    case TYPE_ATTRIBUTE_NESTED_PUBLIC:
        outPut << "public ";
        break;
    case TYPE_ATTRIBUTE_NOT_PUBLIC:
    case TYPE_ATTRIBUTE_NESTED_FAM_AND_ASSEM:
    case TYPE_ATTRIBUTE_NESTED_ASSEMBLY:
        outPut << "internal ";
        break;
    case TYPE_ATTRIBUTE_NESTED_PRIVATE:
        outPut << "private ";
        break;
    case TYPE_ATTRIBUTE_NESTED_FAMILY:
        outPut << "protected ";
        break;
    case TYPE_ATTRIBUTE_NESTED_FAM_OR_ASSEM:
        outPut << "protected internal ";
        break;
    }

    if (flags & TYPE_ATTRIBUTE_ABSTRACT && flags & TYPE_ATTRIBUTE_SEALED) {
        outPut << "static ";
    }
    else if (!(flags & TYPE_ATTRIBUTE_INTERFACE) && flags & TYPE_ATTRIBUTE_ABSTRACT) {
        outPut << "abstract ";
    }
    else if (!is_valuetype && !is_enum && flags & TYPE_ATTRIBUTE_SEALED) {
        outPut << "sealed ";
    }

    if (flags & TYPE_ATTRIBUTE_INTERFACE) {
        outPut << "interface ";
    }
    else if (is_enum) {
        outPut << "enum ";
    }
    else if (is_valuetype) {
        outPut << "struct ";
    }
    else {
        outPut << "class ";
    }

    return outPut.str();
}

std::string GetMethodModifiers(uint16_t flags)
{
    std::string str;

    uint16_t access = flags & METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK;
    switch (access)
    {
    case METHOD_ATTRIBUTE_PRIVATE:
        str += "private ";
        break;
    case METHOD_ATTRIBUTE_PUBLIC:
        str += "public ";
        break;
    case METHOD_ATTRIBUTE_FAMILY:
        str += "protected ";
        break;
    case METHOD_ATTRIBUTE_ASSEM:
    case METHOD_ATTRIBUTE_FAM_AND_ASSEM:
        str += "internal ";
        break;
    case METHOD_ATTRIBUTE_FAM_OR_ASSEM:
        str += "protected internal ";
        break;
    }

    if (flags & METHOD_ATTRIBUTE_STATIC)
        str += "static ";

    if (flags & METHOD_ATTRIBUTE_ABSTRACT)
    {
        str += "abstract ";
        if ((flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_REUSE_SLOT)
            str += "override ";
    }
    else if (flags & METHOD_ATTRIBUTE_FINAL)
    {
        if ((flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_REUSE_SLOT)
            str += "sealed override ";
    }
    else if (flags & METHOD_ATTRIBUTE_VIRTUAL)
    {
        if ((flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_NEW_SLOT)
            str += "virtual ";
        else
            str += "override ";
    }

    if (flags & METHOD_ATTRIBUTE_PINVOKE_IMPL)
        str += "extern ";

    return str;
}

std::string GetFieldModifier(const CIl2CppField field)
{
    std::string rets = "";
	uint32_t flags = field.get_flags();
    uint16_t attrs = flags;
    auto access = attrs & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK;
    switch (access) {
    case FIELD_ATTRIBUTE_PRIVATE:
        rets += "private ";
        break;
    case FIELD_ATTRIBUTE_PUBLIC:
        rets += "public ";
        break;
    case FIELD_ATTRIBUTE_FAMILY:
        rets += "protected ";
        break;
    case FIELD_ATTRIBUTE_ASSEMBLY:
    case FIELD_ATTRIBUTE_FAM_AND_ASSEM:
        rets += "internal ";
        break;
    case FIELD_ATTRIBUTE_FAM_OR_ASSEM:
        rets += "protected internal ";
        break;
    }

    if (attrs & FIELD_ATTRIBUTE_LITERAL) {
        rets += "const ";
    }
    else
    {
        if (attrs & FIELD_ATTRIBUTE_STATIC) {
            rets += "static ";
        }
        if (attrs & FIELD_ATTRIBUTE_INIT_ONLY) {
            rets += "readonly ";
        }
    }

    return rets;
}

std::string TrimCsType(const std::string& type)
{
    std::string result;
    std::string token;
    int genericDepth = 0;

    auto FlushToken = [&]()
        {
            if (token.empty())
                return;

            size_t pos = token.find_last_of('.');
            if (pos != std::string::npos)
                result += token.substr(pos + 1);
            else
                result += token;

            token.clear();
        };

    for (size_t i = 0; i < type.size(); i++)
    {
        char c = type[i];

        if (c == '<')
        {
            FlushToken();
            result += '<';
            genericDepth++;
        }
        else if (c == '>')
        {
            FlushToken();
            result += '>';
            genericDepth--;
        }
        else if (c == ',')
        {
            FlushToken();
            result += ", ";
        }
        else if (c == '[' && i + 1 < type.size() && type[i + 1] == ']')
        {
            FlushToken();
            result += "[]";
            i++;
        }
        else
        {
            token += c;
        }
    }

    FlushToken();
    return result;
}

std::string GetTypeName(const CIl2CppType& type) {
    if (type.is_null())
        return "void";

    std::string typeName = type.formatted_name();
    if (typeName.empty())
        return "object";

    return TrimCsType(typeName);
}

void DumpCsHeader(std::ostream& os)
{
    CIl2CppDomain domain = CIl2CppDomain::get();
    if (domain.is_null())
        return;

    auto assemblies = domain.assemblies();
    uint32_t runningOffset = 0;

    for (size_t i = 0; i < assemblies.size(); i++) {
        CIl2CppImage image = assemblies[i].get_image();
        if (image.is_null())
            continue;

        size_t classCount = image.class_count();

        os << "// Image " << i << ": " << image.name()
            << " - TypeDefIndexOffset: " << runningOffset << "\n";

        runningOffset += static_cast<uint32_t>(classCount);
    }

    os << "\n";
}

void DumpFields(std::ostream& os, CIl2CppClass klass)
{
    void* iter = nullptr;
    const FieldInfo* field;

    os << "\t// Fields\n";

	auto fields = klass.fields();

    for (auto& field : fields) {
		CIl2CppType type = field.get_type();
		std::string tname = GetTypeName(type);
		uint32_t offset = field.get_offset();

        std::string literal;
        //if (field->type->attrs & FIELD_ATTRIBUTE_LITERAL) {
        //    literal = DumpLiteralValue(const_cast<FieldInfo*>(field));
        //}

		os << "\t" << GetFieldModifier(field) << tname << " " << field.name();

        if (!literal.empty()) {
            os << " = " << literal;
        }

        os << "; // 0x" << std::hex << offset << std::dec << "\n";
    }

    os << "\n";
}

void GetMethodParamModifier(std::ostream& os, const CIl2CppType type, uint32_t attrs) {
    if (type.is_by_ref()) {
        if (attrs & PARAM_ATTRIBUTE_OUT && !(attrs & PARAM_ATTRIBUTE_IN)) {
            os << "out ";
        }
        else if (attrs & PARAM_ATTRIBUTE_IN && !(attrs & PARAM_ATTRIBUTE_OUT)) {
            os << "in ";
        }
        else {
            os << "ref ";
        }
    }
    else {
        if (attrs & PARAM_ATTRIBUTE_IN) {
            os << "[In] ";
        }
        if (attrs & PARAM_ATTRIBUTE_OUT) {
            os << "[Out] ";
        }
    }
}

void DumpMethods(std::ostream& os, CIl2CppClass klass) {
    void* iter = nullptr;
    const MethodInfo* method;

    os << "\t// Methods\n\n";

    std::vector<CIl2CppMethod> methods = klass.methods();

    for (size_t i = 0; i < methods.size(); ++i) {
        CIl2CppMethod& method = methods[i];
        uintptr_t va = (uintptr_t)method.va();
        uintptr_t rva = method.rva();

        os << std::uppercase << "\t// RVA: 0x" << std::hex << rva << " VA: 0x" << va << std::dec << " // Slot: " << i << "\n";

        auto returnType = method.return_type();
        auto flags = method.get_flags();
        std::string ret = GetTypeName(returnType);
        std::string modifier = GetMethodModifiers(flags);
        std::string methodName = method.name();

        os << "\t" << modifier;
        if (returnType.is_by_ref()) {
            os << "ref ";
        }

        os << ret << " " << methodName << "(";

        auto param_count = method.param_count();
        for (int i = 0; i < param_count; ++i) {
            CIl2CppType p = method.get_param(i);
            auto attrs = p.get_attrs();
            //std::string pname = il2cpp_method_get_param_name(method.method_info(), i);
            GetMethodParamModifier(os, p, attrs);

            os << GetTypeName(p) << " " << "arg" << i;

            if (i + 1 < param_count)
                os << ", ";
        }

        os << ") { }\n\n";
    }
}

void DumpClass(std::ostream& os, const CIl2CppClass& klass, size_t tdi, std::string imgName)
{
    DebugPrintA("[DumpCs] Dumping class: %s\n", klass.name().c_str());

    std::string name = klass.name();
    std::string nsp = klass.namespace_name();
	std::string modifier = GetClassModifier(klass);

    os << "// Assembly: " << imgName << "\n";
    os << "// Namespace: " << nsp << "\n";

    std::vector<std::string> extends;

    CIl2CppClass parent = klass.get_parent();
    bool is_valuetype = klass.is_value_type();

    if (!is_valuetype && !parent.is_null()) {
		//DebugPrintA("[DumpCs] is_valuetype %d\n", is_valuetype);
  //      DebugPrintA("[DumpCs] parent: %s\n", parent.name().c_str());
        CIl2CppType parent_type = parent.byval_arg();

        if (parent_type.formatted_name() != "object") {
            if (!parent.name().empty()) {
                extends.emplace_back(parent.name());
            }
        }
    }

    //if (il2cpp_class_get_interfaces) {
    //    void* iter = nullptr;
    //    while (auto itf = il2cpp_class_get_interfaces(klass, &iter)) {
    //        extends.emplace_back(itf->name);
    //    }
    //}
    //else {
    //    // Fallback method if il2cpp_class_get_interfaces is not available
    //    if (klass->interfaces_count > 0 && klass->implementedInterfaces) {
    //        for (uint16_t i = 0; i < klass->interfaces_count; ++i) {
    //            Il2CppClass* interfaceClass = klass->implementedInterfaces[i];
    //            if (interfaceClass && interfaceClass->name) {
    //                extends.emplace_back(interfaceClass->name);
    //            }
    //        }
    //    }
    //}

    os << GetClassModifier(klass) << name;

    if (!extends.empty()) {
        os << " : " << extends[0];
        for (size_t i = 1; i < extends.size(); ++i) {
            os << ", " << extends[i];
        }
    }

    os << " // TypeDefIndex: " << tdi << "\n" << "{\n";

    DumpFields(os, klass);

    DumpMethods(os, klass);

    os << "}\n\n";
}

void DumpClasses(std::ostream& os) {
    CIl2CppDomain domain = CIl2CppDomain::get();
    if (domain.is_null())
        return;

    auto assemblies = domain.assemblies();
    for (auto& assembly : assemblies) {
        CIl2CppImage image = assembly.get_image();
        auto classes = image.classes();
        for (size_t i = 0; i < classes.size(); i++) {
            DumpClass(os, classes[i], i, image.name());
        }
    }
}

void DumpCs(const char* path) {
    DebugPrintA("[DumpCs] Start dumping ...\n");

    std::filesystem::path filePath(path);
    std::filesystem::path directory = filePath.parent_path();
    if (!std::filesystem::exists(directory))
        std::filesystem::create_directories(directory);

    std::ofstream file(path);
    if (!file.is_open()) {
        DebugPrintA("[ERROR] Failed to open file: %s\n", path);
        return;
    }

    std::ostringstream ss;
	ss << "// Create by CerydraDumper\n\n";

    DumpCsHeader(ss);
    DumpClasses(ss);

    file << ss.str();
    file.close();

    DebugPrintA("[DumpCs] Dump done.\n");
}
