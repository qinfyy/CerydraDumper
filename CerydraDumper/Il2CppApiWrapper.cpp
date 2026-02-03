#include "pch.h"
#include "Il2CppApiWrapper.h"
#include "Il2CppFunctions.h"
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>

//CIl2CppDomain
std::vector<CIl2CppAssembly> CIl2CppDomain::assemblies() const {
    size_t count = 0;
    Il2CppAssembly** arr = il2cpp_domain_get_assemblies(reinterpret_cast<Il2CppDomain*>(ptr), &count);

    std::vector<CIl2CppAssembly> result;
    result.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.emplace_back(reinterpret_cast<uintptr_t>(arr[i]));
    }

    return result;
}

CIl2CppAssembly CIl2CppDomain::assembly_open(const std::string& name) const {
    Il2CppAssembly* assembly = il2cpp_domain_assembly_open(reinterpret_cast<Il2CppDomain*>(ptr), const_cast<char*>(name.c_str()));
    if (!assembly)
        return CIl2CppAssembly(0);

    return CIl2CppAssembly(reinterpret_cast<uintptr_t>(assembly));
}

CIl2CppDomain CIl2CppDomain::get() {
    Il2CppDomain* domain = il2cpp_domain_get();
    if (!domain)
        return CIl2CppDomain(0);
    return CIl2CppDomain(reinterpret_cast<uintptr_t>(domain));
}

// Il2CppAssembly
CIl2CppImage CIl2CppAssembly::get_image() const {
    Il2CppImage* image = il2cpp_assembly_get_image(reinterpret_cast<Il2CppAssembly*>(ptr));

    if (!image)
        return CIl2CppImage(0);

    return CIl2CppImage(reinterpret_cast<uintptr_t>(image));
}

// Il2CppImage
std::string CIl2CppImage::name() const {
    return std::string(il2cpp_image_get_name(reinterpret_cast<Il2CppImage*>(ptr)));
}

size_t CIl2CppImage::class_count() const {
    return il2cpp_image_get_class_count(reinterpret_cast<Il2CppImage*>(ptr));
}

std::vector<CIl2CppClass> CIl2CppImage::classes() const {
    size_t count = class_count();
    std::vector<CIl2CppClass> out;
    out.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        Il2CppClass* cls = il2cpp_image_get_class(reinterpret_cast<Il2CppImage*>(ptr), i);
        if (!cls) {
            out.emplace_back(0);
        }
        else {
            out.emplace_back(reinterpret_cast<uintptr_t>(cls));
        }
    }

    return out;
}

// Il2CppClass
std::string CIl2CppClass::name() const {
    return std::string(il2cpp_class_get_name(reinterpret_cast<Il2CppClass*>(ptr)));
}

std::string CIl2CppClass::namespace_name() const {
    return std::string(il2cpp_class_get_namespace(reinterpret_cast<Il2CppClass*>(ptr)));
}

CIl2CppClass CIl2CppClass::get_parent() const {
    Il2CppClass* parent = il2cpp_class_get_parent(reinterpret_cast<Il2CppClass*>(ptr));
    return CIl2CppClass(reinterpret_cast<uintptr_t>(parent));
}

CIl2CppType CIl2CppClass::byval_arg() const {
    Il2CppType* t = reinterpret_cast<Il2CppType*>(ptr + 128);
    return CIl2CppType(reinterpret_cast<uintptr_t>(t));
}

std::vector<CIl2CppMethod> CIl2CppClass::methods() const {
    std::vector<CIl2CppMethod> out;
    void* iter = nullptr;

    while (true) {
        MethodInfo* method = il2cpp_class_get_methods(reinterpret_cast<Il2CppClass*>(ptr), &iter);
        if (!method)
            break;

        out.emplace_back(reinterpret_cast<uintptr_t>(method));
    }

    return out;
}

std::vector<CIl2CppField> CIl2CppClass::fields() const {
    std::vector<CIl2CppField> out;
    void* iter = nullptr;

    while (true) {
        FieldInfo* field = il2cpp_class_get_fields(reinterpret_cast<Il2CppClass*>(ptr), &iter);
        if (!field)
            break;

        out.emplace_back(reinterpret_cast<uintptr_t>(field));
    }

    return out;
}

int32_t CIl2CppClass::get_flags() const {
    return il2cpp_class_get_flags(reinterpret_cast<Il2CppClass*>(ptr));
}

bool CIl2CppClass::is_enum() const {
    return il2cpp_class_is_enum(reinterpret_cast<Il2CppClass*>(ptr));
}

bool CIl2CppClass::is_value_type() const {
    return il2cpp_class_is_valuetype(reinterpret_cast<Il2CppClass*>(ptr));
}

CIl2CppMethod CIl2CppClass::find_method_by_name(const std::string& name) const {
    auto ms = methods();
    for (auto& m : ms) {
        if (m.name() == name)
            return m;
    }

    return CIl2CppMethod(0);
}

CIl2CppMethod CIl2CppClass::find_method(const std::string& name, const std::vector<std::string>& arg_types) const {
    auto ms = methods();
    for (auto& m : ms) {
        if (m.name() != name)
            continue;

        size_t count = m.param_count();
        if (count != arg_types.size())
            continue;

        bool fail = false;
        for (size_t i = 0; i < count; ++i) {
            CIl2CppType t = m.get_param(i);
            std::string tn = t.formatted_name();
            if (tn != arg_types[i]) {
                fail = true;
                break; 
            }
        }

        if (!fail)
            return m;
    }

    return CIl2CppMethod(0);
}

// 按返回类型 + 参数类型查找方法，返回 CIl2CppMethod
CIl2CppMethod CIl2CppClass::find_method_by_return_type(const std::string& return_type, const std::vector<std::string>& arg_types) const {
    auto ms = methods();

    for (auto& m : ms) {
        CIl2CppType rt = m.return_type();
        std::string rname = rt.formatted_name();
        if (rname.find(return_type) != 0)
            continue;

        size_t count = m.param_count();
        if (count != arg_types.size())
            continue;

        bool fail = false;
        for (size_t i = 0; i < count; ++i) {
            CIl2CppType t = m.get_param(i);
            std::string tn = t.formatted_name();
            if (tn != arg_types[i]) { 
                fail = true;
                break;
            }
        }

        if (!fail)
            return m;
    }

    return CIl2CppMethod(0);
}

// Il2CppType
std::string CIl2CppType::name() const {
	const char* type_name = il2cpp_type_get_name(reinterpret_cast<Il2CppType*>(ptr));
    return std::string(type_name);
}

uint32_t CIl2CppType::get_attrs() const {
    return il2cpp_type_get_attrs(reinterpret_cast<Il2CppType*>(ptr));
}

bool CIl2CppType::is_by_ref() const {
    return il2cpp_type_is_byref(reinterpret_cast<Il2CppType*>(ptr));
}

std::string CIl2CppType::formatted_name() const {
    std::string n = name();
    if (n == "System.Int32") return "int";
    if (n == "System.UInt32") return "uint";
    if (n == "System.Int16") return "short";
    if (n == "System.UInt16") return "ushort";
    if (n == "System.Int64") return "long";
    if (n == "System.UInt64") return "ulong";
    if (n == "System.Byte") return "byte";
    if (n == "System.SByte") return "sbyte";
    if (n == "System.Boolean") return "bool";
    if (n == "System.Single") return "float";
    if (n == "System.Double") return "double";
    if (n == "System.String") return "string";
    if (n == "System.Char") return "char";
    if (n == "System.Object") return "object";
    if (n == "System.Void") return "void";
    if (n == "System.Decimal") return "decimal";
    if (n == "System.DateTime") return "DateTime";
    return n;
}

CIl2CppClass CIl2CppType::get_class() const {
    Il2CppClass* cls = il2cpp_class_from_type(reinterpret_cast<Il2CppType*>(ptr));
    return CIl2CppClass(reinterpret_cast<uintptr_t>(cls));
}

// Il2CppMethod
MethodInfo* CIl2CppMethod::method_info() const {
    return reinterpret_cast<MethodInfo*>(ptr);
}

std::string CIl2CppMethod::name() const {
    return std::string(il2cpp_method_get_name(reinterpret_cast<MethodInfo*>(ptr)));
}

CIl2CppType CIl2CppMethod::return_type() const {
    Il2CppType* t = il2cpp_method_get_return_type(reinterpret_cast<MethodInfo*>(ptr));
    return CIl2CppType(reinterpret_cast<uintptr_t>(t));
}

CIl2CppClass CIl2CppMethod::class_ptr() const {
    Il2CppClass* cls = reinterpret_cast<Il2CppClass*>(*(uintptr_t*)ptr);
    return CIl2CppClass(reinterpret_cast<uintptr_t>(cls));
}

uintptr_t CIl2CppMethod::va() const {
    return *(uintptr_t*)(ptr + 8);
}

uintptr_t CIl2CppMethod::rva() const {
    uintptr_t _va = va();
    if (_va == 0)
        return 0;

    return _va - GetGameAssemblyModuleBase();
}

bool CIl2CppMethod::is_valid() const {
    auto info = method_info();
    return info && info->method_pointer != nullptr;
}

uint32_t CIl2CppMethod::param_count() const {
    return il2cpp_method_get_param_count(reinterpret_cast<MethodInfo*>(ptr));
}

CIl2CppType CIl2CppMethod::get_param(uint32_t i) const {
    Il2CppType* t = il2cpp_method_get_param(reinterpret_cast<MethodInfo*>(ptr), i);
    return CIl2CppType(reinterpret_cast<uintptr_t>(t));
}

std::string CIl2CppMethod::param_type_formatted(uint32_t i) const {
    CIl2CppType t = get_param(i);
    std::string name = t.name();

    if (name == "System.Int32") return "int";
    if (name == "System.UInt32") return "uint";
    if (name == "System.Int16") return "short";
    if (name == "System.UInt16") return "ushort";
    if (name == "System.Int64") return "long";
    if (name == "System.UInt64") return "ulong";
    if (name == "System.Byte") return "byte";
    if (name == "System.SByte") return "sbyte";
    if (name == "System.Boolean") return "bool";
    if (name == "System.Single") return "float";
    if (name == "System.Double") return "double";
    if (name == "System.String") return "string";
    if (name == "System.Char") return "char";
    if (name == "System.Object") return "object";
    if (name == "System.Void") return "void";
    if (name == "System.Decimal") return "decimal";
    if (name == "System.DateTime") return "DateTime";

    return name;
}

std::string CIl2CppMethod::format_params() const {
    std::ostringstream out;
    uint32_t count = param_count();
    out << name() << "(";
    for (uint32_t i = 0; i < count; ++i) {
        out << param_type_formatted(i);
        if (i + 1 < count) out << ",";
    }

    out << ")";
    return out.str();
}

int32_t CIl2CppMethod::get_flags() const {
    if (is_null())
        return 0;

    return method_info()->flags;
}

// CIl2CppField
std::string CIl2CppField::name() const {
    if (is_null())
        return "";

    const char* n = il2cpp_field_get_name(reinterpret_cast<FieldInfo*>(ptr));
    return n ? std::string(n) : std::string();
}

int32_t CIl2CppField::get_flags() const {
    if (is_null())
        return 0;

    return il2cpp_field_get_flags(reinterpret_cast<FieldInfo*>(ptr));
}

size_t CIl2CppField::get_offset() const {
    if (is_null())
        return 0;

    return il2cpp_field_get_offset(reinterpret_cast<FieldInfo*>(ptr));
}

CIl2CppType CIl2CppField::get_type() const {
    if (is_null())
        return CIl2CppType(0);

    Il2CppType* t = il2cpp_field_get_type(reinterpret_cast<FieldInfo*>(ptr));
    return CIl2CppType(reinterpret_cast<uintptr_t>(t));
}

CIl2CppObject CIl2CppField::get_value_object(const CIl2CppObject& instance) const {
    if (is_null() || instance.is_null()) return CIl2CppObject(0);
    Il2CppObject* value = il2cpp_field_get_value_object(reinterpret_cast<FieldInfo*>(ptr), reinterpret_cast<Il2CppObject*>(instance.raw_ptr()));
    
    if (!value)
        return CIl2CppObject(0);

    return CIl2CppObject(reinterpret_cast<uintptr_t>(value));
}

// Il2CppObject
const CIl2CppObject CIl2CppObject::NULL_OBJ = CIl2CppObject(0);

CIl2CppClass CIl2CppObject::get_class() const {
    if (is_null())
        return CIl2CppClass(0);

    // 首 8 字节存放类指针
    uintptr_t cls_ptr = *(uintptr_t*)ptr;
    return CIl2CppClass(cls_ptr);
}
