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

class CIl2CppWrapBase {
protected:
    uintptr_t ptr = 0;

public:
    CIl2CppWrapBase() : ptr(0) {}
    explicit CIl2CppWrapBase(uintptr_t p) : ptr(p) {}

    bool is_null() const { return ptr == 0; }

    operator void* () const { return reinterpret_cast<void*>(ptr); }
    operator uintptr_t() const { return ptr; }

    uintptr_t raw_ptr() const { return ptr; }
};

class CIl2CppDomain : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    std::vector<CIl2CppAssembly> assemblies() const;
    CIl2CppAssembly assembly_open(const std::string& name) const;

    static CIl2CppDomain get();
};

class CIl2CppAssembly : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CIl2CppImage get_image() const;
};

class CIl2CppImage : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    std::string name() const;
    size_t class_count() const;
    std::vector<CIl2CppClass> classes() const;
};

class CIl2CppClass : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

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

class CIl2CppType : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    std::string name() const;
    uint32_t get_attrs() const;
    bool is_by_ref() const;
    std::string formatted_name() const;
    CIl2CppClass get_class() const;
};

class CIl2CppMethod : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

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

class CIl2CppField : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    std::string name() const;

    int32_t get_flags() const;

    size_t get_offset() const;

    CIl2CppType get_type() const;

    CIl2CppObject get_value_object(const CIl2CppObject& instance) const;
};

class CIl2CppObject : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    static const CIl2CppObject NULL_OBJ;

    static CIl2CppObject from_uintptr(uintptr_t p) { return CIl2CppObject(p); }

    CIl2CppClass get_class() const;

    template<typename T>
    T unbox() const {
        if (is_null()) throw std::runtime_error("Attempt to unbox null object");
        return *(T*)(ptr + 16); // 偏移 +16
    }
};


class CIl2CppArray : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    // ---- basic fields ----

    inline CIl2CppClass klass() const;
    inline uintptr_t monitor() const;
    inline uintptr_t bounds() const;

    // +0x18 length
    __forceinline size_t length() const {
        return *reinterpret_cast<const size_t*>(ptr + 0x18);
    }

    inline bool empty() const { return length() == 0; }

    // ---- raw data ----
private:
    inline uintptr_t first_item_ptr() const
    {
        // data starts at +0x20
        return ptr + 0x20;
    }

public:
    // ---- element access (template, header-only) ----

    template<typename T>
    inline const T& get(size_t index) const
    {
        static_assert(!std::is_void_v<T>, "T must not be void");
        return *reinterpret_cast<const T*>(
            first_item_ptr() + index * sizeof(T)
            );
    }

    template<typename T>
    inline T& get_mut(size_t index)
    {
        static_assert(!std::is_void_v<T>, "T must not be void");
        return *reinterpret_cast<T*>(first_item_ptr() + index * sizeof(T));
    }

    template<typename T>
    inline std::vector<T> to_vec() const
    {
        static_assert(std::is_copy_constructible_v<T>,
            "T must be copyable");

        const T* begin = reinterpret_cast<const T*>(first_item_ptr());
        return std::vector<T>(begin, begin + length());
    }

    template<typename T>
    inline std::vector<T> to_vec_sized(size_t size) const
    {
        static_assert(std::is_copy_constructible_v<T>,
            "T must be copyable");

        const T* begin = reinterpret_cast<const T*>(first_item_ptr());
        return std::vector<T>(begin, begin + size);
    }
};
