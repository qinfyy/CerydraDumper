#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept> // 必须
#include "Il2CppFunctions.h"

// 提前声明
class CIl2CppAssembly;
class CIl2CppImage;
class CIl2CppClass;
class CIl2CppType;
class CIl2CppMethod;
class CIl2CppField;
class CIl2CppObject;

class CIl2CppDomain {
public:
    uintptr_t ptr = 0;

    CIl2CppDomain() : ptr(0) {}
    explicit CIl2CppDomain(uintptr_t p) : ptr(p) {}

    bool is_null() const { return ptr == 0; }

    std::vector<CIl2CppAssembly> assemblies() const;
    CIl2CppAssembly assembly_open(const std::string& name) const;

    static CIl2CppDomain get() {
        ::Il2CppDomain* domain = ::il2cpp_domain_get();
        if (!domain) return CIl2CppDomain(0);
        return CIl2CppDomain(reinterpret_cast<uintptr_t>(domain));
    }
};

class CIl2CppAssembly {
public:
    uintptr_t ptr = 0;

    CIl2CppAssembly() : ptr(0) {}
    explicit CIl2CppAssembly(uintptr_t p) : ptr(p) {}

    bool is_null() const { return ptr == 0; }

    CIl2CppImage get_image() const;
};

class CIl2CppImage {
public:
    uintptr_t ptr = 0;

    CIl2CppImage() : ptr(0) {}
    explicit CIl2CppImage(uintptr_t p) : ptr(p) {}

    bool is_null() const { return ptr == 0; }

    std::string name() const;
    size_t class_count() const;
    std::vector<CIl2CppClass> classes() const;
};

class CIl2CppClass {
public:
    uintptr_t ptr = 0;

    CIl2CppClass() : ptr(0) {}
    explicit CIl2CppClass(uintptr_t p) : ptr(p) {}

    bool is_null() const { return ptr == 0; }

    std::string name() const;
    std::string namespace_name() const;
    CIl2CppClass get_parent() const;
    CIl2CppType byval_arg() const;
    std::vector<CIl2CppMethod> methods() const;
    std::vector<CIl2CppField> fields() const;
    int32_t get_flags() const;
    bool is_enum() const;
    bool is_value_type() const;

    CIl2CppMethod find_method_by_name(const std::string& name) const;
    CIl2CppMethod find_method(const std::string& name, const std::vector<std::string>& arg_types) const;
    CIl2CppMethod find_method_by_return_type(const std::string& return_type, const std::vector<std::string>& arg_types) const;
};

class CIl2CppType {
public:
    uintptr_t ptr = 0;

    CIl2CppType() : ptr(0) {}
    explicit CIl2CppType(uintptr_t p) : ptr(p) {}

    bool is_null() const { return ptr == 0; }

    std::string name() const;
    uint32_t get_attrs() const;
    bool is_by_ref() const;
    std::string formatted_name() const;
    CIl2CppClass get_class() const;
};

class CIl2CppMethod {
public:
    uintptr_t ptr = 0;

    CIl2CppMethod() : ptr(0) {}
    explicit CIl2CppMethod(uintptr_t p) : ptr(p) {}

    bool is_null() const { return ptr == 0; }

    ::MethodInfo* method_info() const;
    std::string name() const;
    CIl2CppType return_type() const;
    CIl2CppClass class_ptr() const;
    uintptr_t va() const;
    uintptr_t rva() const;
    bool is_valid() const;
    uint32_t param_count() const;
    CIl2CppType get_param(uint32_t i) const;
    std::string param_type_formatted(uint32_t i) const;
    std::string format_params() const;
	int32_t get_flags() const;
};

class CIl2CppField {
public:
    uintptr_t ptr = 0;

    CIl2CppField() : ptr(0) {}
    explicit CIl2CppField(uintptr_t p) : ptr(p) {}

    bool is_null() const { return ptr == 0; }

    std::string name() const;

    int32_t get_flags() const;

    size_t get_offset() const;

    CIl2CppType get_type() const;

    CIl2CppObject get_value_object(const CIl2CppObject& instance) const;
};

class CIl2CppObject {
public:
    uintptr_t ptr = 0;

    static const CIl2CppObject NULL_OBJ;

    CIl2CppObject() : ptr(0) {}
    explicit CIl2CppObject(uintptr_t p) : ptr(p) {}

    bool is_null() const { return ptr == 0; }

    static CIl2CppObject from_uintptr(uintptr_t p) { return CIl2CppObject(p); }

    CIl2CppClass get_class() const;

    template<typename T>
    T unbox() const {
        if (is_null()) throw std::runtime_error("Attempt to unbox null object");
        return *(T*)(ptr + 16); // 偏移 +16
    }
};
