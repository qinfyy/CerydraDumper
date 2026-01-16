#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Il2CppFunctions.h"

namespace Il2CppApiWrapper {

    class Il2CppAssembly;
    class Il2CppImage;
    class Il2CppClass;
    class Il2CppType;

    class Il2CppDomain {
    public:
        uintptr_t ptr = 0;

        Il2CppDomain() : ptr(0) {}                    // nullptr ππ‘Ï
        explicit Il2CppDomain(uintptr_t p) : ptr(p) {}

        bool is_null() const { return ptr == 0; }

        std::vector<Il2CppAssembly> assemblies() const;
        Il2CppAssembly assembly_open(const std::string& name) const;

        static Il2CppDomain get() {
            ::Il2CppDomain* domain = ::il2cpp_domain_get();
            if (!domain) return Il2CppDomain(0);
            return Il2CppDomain(reinterpret_cast<uintptr_t>(domain));
        }
    };

    class Il2CppAssembly {
    public:
        uintptr_t ptr = 0;

        Il2CppAssembly() : ptr(0) {}
        explicit Il2CppAssembly(uintptr_t p) : ptr(p) {}

        bool is_null() const { return ptr == 0; }

        Il2CppImage get_image() const;
    };

    class Il2CppImage {
    public:
        uintptr_t ptr = 0;

        Il2CppImage() : ptr(0) {}
        explicit Il2CppImage(uintptr_t p) : ptr(p) {}

        bool is_null() const { return ptr == 0; }

        std::string name() const;
        size_t class_count() const;
        std::vector<Il2CppClass> classes() const;
    };

    class Il2CppClass {
    public:
        uintptr_t ptr = 0;

        Il2CppClass() : ptr(0) {}
        explicit Il2CppClass(uintptr_t p) : ptr(p) {}

        bool is_null() const { return ptr == 0; }

        std::string name() const;
        std::string namespace_name() const;
        ::Il2CppClass* get_parent() const;
        ::Il2CppType* byval_arg() const;
        std::vector<::MethodInfo*> methods() const;
        std::vector<::FieldInfo*> fields() const;
        int32_t get_flags() const;
        bool is_enum() const;
        bool is_value_type() const;

        ::MethodInfo* find_method_by_name(const std::string& name) const;
        ::MethodInfo* find_method(const std::string& name, const std::vector<std::string>& arg_types) const;
        ::MethodInfo* find_method_by_return_type(const std::string& return_type, const std::vector<std::string>& arg_types) const;
    };

    class Il2CppType {
    public:
        uintptr_t ptr = 0;

        Il2CppType() : ptr(0) {}
        explicit Il2CppType(uintptr_t p) : ptr(p) {}

        bool is_null() const { return ptr == 0; }

        std::string name() const;
        uint32_t get_attrs() const;
        bool is_by_ref() const;
        std::string formatted_name() const;
        ::Il2CppClass* get_class() const;
    };

    class Il2CppMethod {
    public:
        uintptr_t ptr = 0;

        Il2CppMethod() : ptr(0) {}
        explicit Il2CppMethod(uintptr_t p) : ptr(p) {}

        bool is_null() const { return ptr == 0; }

        ::MethodInfo* method_info() const;
        std::string name() const;
        ::Il2CppType* return_type() const;
        ::Il2CppClass* class_ptr() const;
        uintptr_t va() const;
        uintptr_t rva() const;
        bool is_valid() const;
        uint32_t param_count() const;
        ::Il2CppType* get_param(uint32_t i) const;
        std::string param_type_formatted(uint32_t i) const;
        std::string format_params() const;
    };

    class Il2CppField {
    public:
        uintptr_t ptr = 0;

        Il2CppField() : ptr(0) {}
        explicit Il2CppField(uintptr_t p) : ptr(p) {}

        bool is_null() const { return ptr == 0; }

        std::string name() const;
        int32_t get_flags() const;
        size_t get_offset() const;
        ::Il2CppType* get_type() const;
        ::Il2CppObject* get_value_object(::Il2CppObject* instance) const;
    };

    class Il2CppObject {
    public:
        uintptr_t ptr = 0;

        Il2CppObject() : ptr(0) {}
        explicit Il2CppObject(uintptr_t p) : ptr(p) {}

        bool is_null() const { return ptr == 0; }

        ::Il2CppClass* get_class() const;
        template<typename T>
        T unbox() const;
    };

} // namespace Il2CppApiWrapper
