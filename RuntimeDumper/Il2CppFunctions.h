#pragma once
#include <cstdint>

uintptr_t GetUnityPlayerModuleBase();

uintptr_t GetApiBase();

uintptr_t GetGameAssemblyModuleBase();

void InitIl2CppFunctions();

typedef void Il2CppDomain;
typedef void Il2CppClass;
typedef void Il2CppType;
typedef void Il2CppObject;
typedef void Il2CppAssembly;
typedef void Il2CppImage;
typedef void FieldInfo;

typedef struct MethodInfo {
    void* invoker_method;
    void* method_pointer;
    uint8_t _pad[0x20];
    uint16_t flags;
} MethodInfo;

#define IL2CPP_API(i, name, ret, params, args) \
__declspec(noinline) inline ret name params { \
    using FuncType = ret(__fastcall*) params; \
    uintptr_t addr = GetApiBase() + 8 * (i); \
    FuncType fn = reinterpret_cast<FuncType>(*reinterpret_cast<uintptr_t*>(addr)); \
    return fn args; \
}

IL2CPP_API(22, il2cpp_assembly_get_image, Il2CppImage*, (Il2CppAssembly* assembly), (assembly))
IL2CPP_API(31, il2cpp_class_get_fields, FieldInfo*, (Il2CppClass* klass, void** iter), (klass, iter))
IL2CPP_API(33, il2cpp_class_get_interface, uintptr_t, (Il2CppClass* klass), (klass))
IL2CPP_API(35, il2cpp_class_get_methods, MethodInfo*, (Il2CppClass* klass, void** iter), (klass, iter))
IL2CPP_API(37, il2cpp_class_get_name, const char*, (Il2CppClass* klass), (klass))
IL2CPP_API(39, il2cpp_class_get_namespace, const char*, (Il2CppClass* klass), (klass))
IL2CPP_API(40, il2cpp_class_get_parent, Il2CppClass*, (Il2CppClass* klass), (klass))
IL2CPP_API(43, il2cpp_class_is_valuetype, bool, (Il2CppClass* klass), (klass))
IL2CPP_API(45, il2cpp_class_get_flags, int32_t, (Il2CppClass* klass), (klass))
IL2CPP_API(49, il2cpp_class_from_type, Il2CppClass*, (const Il2CppType* type), (type))
IL2CPP_API(53, il2cpp_class_is_enum, bool, (Il2CppClass* klass), (klass))
IL2CPP_API(63, il2cpp_domain_get, Il2CppDomain*, (), ())
IL2CPP_API(64, il2cpp_domain_assembly_open, Il2CppAssembly*, (Il2CppDomain* domain, char* name), (domain, name))
IL2CPP_API(65, il2cpp_domain_get_assemblies, Il2CppAssembly**, (Il2CppDomain* domain, size_t* size), (domain, size))
IL2CPP_API(72, il2cpp_field_get_flags, int32_t, (FieldInfo* field), (field))
IL2CPP_API(73, il2cpp_field_get_name, const char*, (FieldInfo* field), (field))
IL2CPP_API(75, il2cpp_field_get_offset, size_t, (FieldInfo* field), (field))
IL2CPP_API(76, il2cpp_field_get_type, Il2CppType*, (FieldInfo* field), (field))
IL2CPP_API(77, il2cpp_field_get_value_object, Il2CppObject*, (FieldInfo* field, Il2CppObject* obj), (field, obj));
IL2CPP_API(116, il2cpp_method_get_return_type, Il2CppType*, (const MethodInfo* method), (method))
IL2CPP_API(117, il2cpp_method_get_name, const char*, (const MethodInfo* method), (method))
IL2CPP_API(123, il2cpp_method_get_param_count, uint32_t, (const MethodInfo* method), (method))
IL2CPP_API(124, il2cpp_method_get_param, Il2CppType*, (const MethodInfo* method, uint32_t index), (method, index))
IL2CPP_API(161, il2cpp_type_get_name, const char*, (Il2CppType* type), (type))
IL2CPP_API(162, il2cpp_type_is_byref, bool, (Il2CppType* type), (type))
IL2CPP_API(163, il2cpp_type_get_attrs, uint32_t, (Il2CppType* type), (type))
IL2CPP_API(168, il2cpp_image_get_name, const char*, (Il2CppImage* image), (image))
IL2CPP_API(169, il2cpp_image_get_class_count, size_t, (Il2CppImage* image), (image))
IL2CPP_API(170, il2cpp_image_get_class, Il2CppClass*, (Il2CppImage* image, size_t index), (image, index))
