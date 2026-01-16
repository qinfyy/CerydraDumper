#pragma once
#include <cstdint>

uintptr_t GetUnityPlayerModuleBase();
uintptr_t GetApiBase();
void InitIl2CppFunctions();
uintptr_t GetGameAssemblyModuleBase();

// 占位类型
typedef void Il2CppDomain;
typedef void Il2CppClass;
typedef void Il2CppType;
typedef void Il2CppObject;
typedef void Il2CppAssembly;
typedef void Il2CppImage;
typedef void FieldInfo;
typedef void MethodInfo;

// function_ptr 模板
template<typename T>
class function_ptr {
public:
    function_ptr() = default;
    explicit function_ptr(void* address) : _fn(reinterpret_cast<T>(address)) {}

    inline bool valid() const { return _fn != nullptr; }
    inline operator bool() const { return valid(); }

    T get() const { return _fn; }

private:
    T _fn = nullptr;
};

// il2cpp_functions 类
class il2cpp_functions {
public:
    explicit il2cpp_functions();

    // Function pointers
    function_ptr<Il2CppImage* (*)(Il2CppAssembly* assembly)> assembly_get_image;
    function_ptr<FieldInfo* (*)(Il2CppClass* klass, void** iter)> class_get_fields;
    function_ptr<MethodInfo* (*)(Il2CppClass* klass, void** iter)> class_get_methods;
    function_ptr<const char* (*)(Il2CppClass* klass)> class_get_name;
    function_ptr<const char* (*)(Il2CppClass* klass)> class_get_namespace;
    function_ptr<Il2CppClass* (*)(Il2CppClass* klass)> class_get_parent;
    function_ptr<bool(*)(const Il2CppClass* klass)> class_is_valuetype;
    function_ptr<int32_t(*)(const Il2CppClass* klass)> class_get_flags;
    function_ptr<Il2CppClass* (*)(const Il2CppType* type)> class_from_type;
    function_ptr<bool(*)(const Il2CppClass* klass)> class_is_enum;

    function_ptr<Il2CppDomain* (*)()> domain_get;
    function_ptr<Il2CppAssembly** (*)(Il2CppDomain* domain, size_t* size)> domain_get_assemblies;

    function_ptr<int32_t(*)(FieldInfo* field)> field_get_flags;
    function_ptr<const char* (*)(FieldInfo* field)> field_get_name;
    function_ptr<size_t(*)(FieldInfo* field)> field_get_offset;
    function_ptr<Il2CppType* (*)(FieldInfo* field)> field_get_type;

    function_ptr<Il2CppType* (*)(const MethodInfo* method)> method_get_return_type;
    function_ptr<const char* (*)(const MethodInfo* method)> method_get_name;
    function_ptr<uint32_t(*)(const MethodInfo* method)> method_get_param_count;
    function_ptr<Il2CppType* (*)(const MethodInfo* method, uint32_t index)> method_get_param;

    function_ptr<const char* (*)(Il2CppType* type)> type_get_name;
    function_ptr<bool(*)(Il2CppType* type)> type_is_byref;
    function_ptr<uint32_t(*)(Il2CppType* type)> type_get_attrs;

    function_ptr<const char* (*)(Il2CppImage* image)> image_get_name;
    function_ptr<size_t(*)(Il2CppImage* image)> image_get_class_count;
    function_ptr<Il2CppClass* (*)(Il2CppImage* image, size_t index)> image_get_class;

    // Check if function is valid
    bool is_valid() const;

    Il2CppType* il2cpp_class_get_type(Il2CppClass* klass);

    uintptr_t il2cpp_method_get_relative_pointer(MethodInfo* method);

    // Wrappers for easier calling
    Il2CppImage* il2cpp_assembly_get_image(Il2CppAssembly* assembly);
    FieldInfo* il2cpp_class_get_fields(Il2CppClass* klass, void** iter);
    MethodInfo* il2cpp_class_get_methods(Il2CppClass* klass, void** iter);
    const char* il2cpp_class_get_name(Il2CppClass* klass);
    const char* il2cpp_class_get_namespace(Il2CppClass* klass);
    Il2CppClass* il2cpp_class_get_parent(Il2CppClass* klass);
    bool il2cpp_class_is_valuetype(const Il2CppClass* klass);
    int32_t il2cpp_class_get_flags(const Il2CppClass* klass);
    Il2CppClass* il2cpp_class_from_type(const Il2CppType* type);
    bool il2cpp_class_is_enum(const Il2CppClass* klass);

    Il2CppDomain* il2cpp_domain_get();
    Il2CppAssembly** il2cpp_domain_get_assemblies(Il2CppDomain* domain, size_t* size);

    int32_t il2cpp_field_get_flags(FieldInfo* field);
    const char* il2cpp_field_get_name(FieldInfo* field);
    size_t il2cpp_field_get_offset(FieldInfo* field);
    Il2CppType* il2cpp_field_get_type(FieldInfo* field);

    Il2CppType* il2cpp_method_get_return_type(const MethodInfo* method);
    const char* il2cpp_method_get_name(const MethodInfo* method);
    uint32_t il2cpp_method_get_param_count(const MethodInfo* method);
    Il2CppType* il2cpp_method_get_param(const MethodInfo* method, uint32_t index);

    const char* il2cpp_type_get_name(Il2CppType* type);
    bool il2cpp_type_is_byref(Il2CppType* type);
    uint32_t il2cpp_type_get_attrs(Il2CppType* type);

    const char* il2cpp_image_get_name(Il2CppImage* image);
    size_t il2cpp_image_get_class_count(Il2CppImage* image);
    Il2CppClass* il2cpp_image_get_class(Il2CppImage* image, size_t index);

private:
    void** _table;

    template<typename T>
    function_ptr<T> resolve(size_t index);
};
