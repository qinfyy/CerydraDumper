#include "pch.h"
#include "Il2CppApiWrapper.h"
#include "Il2CppFunctions.h"
#include <cstring>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>

namespace Il2CppApiWrapper {

    std::vector<Il2CppAssembly> Il2CppDomain::assemblies() const {
        size_t count = 0;
        ::Il2CppAssembly** arr = ::il2cpp_domain_get_assemblies(reinterpret_cast<::Il2CppDomain*>(ptr), &count);

        std::vector<Il2CppAssembly> result;
        result.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            result.emplace_back(reinterpret_cast<uintptr_t>(arr[i]));
        }
        return result;
    }

    Il2CppAssembly Il2CppDomain::assembly_open(const std::string& name) const {
        ::Il2CppAssembly* assembly = ::il2cpp_domain_assembly_open(reinterpret_cast<::Il2CppDomain*>(ptr), const_cast<char*>(name.c_str()));
        if (!assembly) return Il2CppAssembly((uintptr_t)nullptr); // 返回空对象
        return Il2CppAssembly(reinterpret_cast<uintptr_t>(assembly)); // 封装到你的类
    }

    // ---------------------- Il2CppAssembly ----------------------
    Il2CppImage Il2CppAssembly::get_image() const {
        ::Il2CppImage* image = ::il2cpp_assembly_get_image(
            reinterpret_cast<::Il2CppAssembly*>(ptr)
        );

        if (!image) return Il2CppImage(0);  // 返回空对象
        return Il2CppImage(reinterpret_cast<uintptr_t>(image));
    }


    // ---------------------- Il2CppImage ----------------------
    std::string Il2CppImage::name() const {
        return std::string(::il2cpp_image_get_name(reinterpret_cast<::Il2CppImage*>(ptr)));
    }

    size_t Il2CppImage::class_count() const {
        return ::il2cpp_image_get_class_count(reinterpret_cast<::Il2CppImage*>(ptr));
    }

    std::vector<Il2CppClass> Il2CppImage::classes() const {
        size_t count = class_count();
        std::vector<Il2CppClass> out;
        out.reserve(count);

        for (size_t i = 0; i < count; ++i) {
            ::Il2CppClass* cls = ::il2cpp_image_get_class(reinterpret_cast<::Il2CppImage*>(ptr), i);
            if (!cls) {
                out.emplace_back(0); // 空对象
            }
            else {
                out.emplace_back(reinterpret_cast<uintptr_t>(cls)); // 封装成 Wrapper
            }
        }

        return out;
    }

    // ---------------------- Il2CppClass ----------------------
    std::string Il2CppClass::name() const {
        return std::string(::il2cpp_class_get_name(reinterpret_cast<::Il2CppClass*>(ptr)));
    }

    std::string Il2CppClass::namespace_name() const {
        return std::string(::il2cpp_class_get_namespace(reinterpret_cast<::Il2CppClass*>(ptr)));
    }

    ::Il2CppClass* Il2CppClass::get_parent() const {
        return ::il2cpp_class_get_parent(reinterpret_cast<::Il2CppClass*>(ptr));
    }

    ::Il2CppType* Il2CppClass::byval_arg() const {
        return reinterpret_cast<::Il2CppType*>(ptr + 128);
    }

    std::vector<::MethodInfo*> Il2CppClass::methods() const {
        std::vector<::MethodInfo*> out;
        void* iter = nullptr;
        while (true) {
            ::MethodInfo* method = ::il2cpp_class_get_methods(reinterpret_cast<::Il2CppClass*>(ptr), &iter);
            if (!method) break;
            out.push_back(method);
        }
        return out;
    }

    std::vector<::FieldInfo*> Il2CppClass::fields() const {
        std::vector<::FieldInfo*> out;
        void* iter = nullptr;
        while (true) {
            ::FieldInfo* field = ::il2cpp_class_get_fields(reinterpret_cast<::Il2CppClass*>(ptr), &iter);
            if (!field) break;
            out.push_back(field);
        }
        return out;
    }

    int32_t Il2CppClass::get_flags() const {
        return ::il2cpp_class_get_flags(reinterpret_cast<::Il2CppClass*>(ptr));
    }

    bool Il2CppClass::is_enum() const {
        return ::il2cpp_class_is_enum(reinterpret_cast<::Il2CppClass*>(ptr));
    }

    bool Il2CppClass::is_value_type() const {
        return ::il2cpp_class_is_valuetype(reinterpret_cast<::Il2CppClass*>(ptr));
    }

    ::MethodInfo* Il2CppClass::find_method_by_name(const std::string& name) const {
        auto ms = methods();
        for (auto m : ms) {
            if (std::string(::il2cpp_method_get_name(m)) == name) return m;
        }
        return nullptr;
    }

    ::MethodInfo* Il2CppClass::find_method(const std::string& name, const std::vector<std::string>& arg_types) const {
        auto ms = methods();
        for (auto m : ms) {
            if (std::string(::il2cpp_method_get_name(m)) != name) continue;
            size_t count = ::il2cpp_method_get_param_count(m);
            if (count != arg_types.size()) continue;
            bool fail = false;
            for (size_t i = 0; i < count; ++i) {
                ::Il2CppType* t = ::il2cpp_method_get_param(m, i);
                std::string tn = ::il2cpp_type_get_name(t);
                if (tn != arg_types[i]) { fail = true; break; }
            }
            if (!fail) return m;
        }
        return nullptr;
    }

    ::MethodInfo* Il2CppClass::find_method_by_return_type(const std::string& return_type, const std::vector<std::string>& arg_types) const {
        auto ms = methods();
        for (auto m : ms) {
            ::Il2CppType* rt = ::il2cpp_method_get_return_type(m);
            std::string rname = ::il2cpp_type_get_name(rt);
            if (rname.find(return_type) != 0) continue;
            size_t count = ::il2cpp_method_get_param_count(m);
            if (count != arg_types.size()) continue;
            bool fail = false;
            for (size_t i = 0; i < count; ++i) {
                ::Il2CppType* t = ::il2cpp_method_get_param(m, i);
                std::string tn = ::il2cpp_type_get_name(t);
                if (tn != arg_types[i]) { fail = true; break; }
            }
            if (!fail) return m;
        }
        return nullptr;
    }

    // ---------------------- Il2CppType ----------------------
    std::string Il2CppType::name() const {
        return std::string(::il2cpp_type_get_name(reinterpret_cast<::Il2CppType*>(ptr)));
    }

    uint32_t Il2CppType::get_attrs() const {
        return ::il2cpp_type_get_attrs(reinterpret_cast<::Il2CppType*>(ptr));
    }

    bool Il2CppType::is_by_ref() const {
        return ::il2cpp_type_is_byref(reinterpret_cast<::Il2CppType*>(ptr));
    }

    std::string Il2CppType::formatted_name() const {
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

    ::Il2CppClass* Il2CppType::get_class() const {
        return ::il2cpp_class_from_type(reinterpret_cast<::Il2CppType*>(ptr));
    }

    // ---------------------- Il2CppMethod ----------------------
    ::MethodInfo* Il2CppMethod::method_info() const {
        return reinterpret_cast<::MethodInfo*>(ptr);
    }

    std::string Il2CppMethod::name() const {
        return std::string(::il2cpp_method_get_name(reinterpret_cast<::MethodInfo*>(ptr)));
    }

    ::Il2CppType* Il2CppMethod::return_type() const {
        return ::il2cpp_method_get_return_type(reinterpret_cast<::MethodInfo*>(ptr));
    }

    ::Il2CppClass* Il2CppMethod::class_ptr() const {
        return reinterpret_cast<::Il2CppClass*>(*(uintptr_t*)ptr);
    }

    uintptr_t Il2CppMethod::va() const {
        return *(uintptr_t*)(ptr + 8);
    }

    uintptr_t Il2CppMethod::rva() const {
        uintptr_t _va = va();
        if (_va == 0) return 0;
        return _va - GetGameAssemblyModuleBase();
    }

    bool Il2CppMethod::is_valid() const {
        auto info = method_info();
        return info && info->method_pointer != nullptr;
    }

    uint32_t Il2CppMethod::param_count() const {
        return ::il2cpp_method_get_param_count(reinterpret_cast<::MethodInfo*>(ptr));
    }

    ::Il2CppType* Il2CppMethod::get_param(uint32_t i) const {
        return ::il2cpp_method_get_param(reinterpret_cast<::MethodInfo*>(ptr), i);
    }

    std::string Il2CppMethod::param_type_formatted(uint32_t i) const {
        ::Il2CppType* t = get_param(i);
        std::string name = ::il2cpp_type_get_name(t);
        // 类型映射和 Rust 保持一致
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

    std::string Il2CppMethod::format_params() const {
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

    // ---------------------- Il2CppObject ----------------------
    //constexpr Il2CppObject Il2CppObject::NULL_OBJ;

    //bool Il2CppObject::is_null() const {
    //    return ptr == 0;
    //}

    ::Il2CppClass* Il2CppObject::get_class() const {
        return reinterpret_cast<::Il2CppClass*>(*(uintptr_t*)ptr);
    }

    template<typename T>
    T Il2CppObject::unbox() const {
        return *(T*)(ptr + 16);
    }

} // namespace Il2CppWrapper
