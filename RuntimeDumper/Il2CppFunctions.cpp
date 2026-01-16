#include "pch.h"
#include "Il2CppFunctions.h"
#include <iostream>
#include "Memory.h"
#include "PrintHelper.h"

uintptr_t GetUnityPlayerModuleBase()
{
    HMODULE mod = GetModuleHandleA("UnityPlayer.dll");
    return reinterpret_cast<uintptr_t>(mod);
}

uintptr_t API_BASE_PTR;

uintptr_t GetApiBase()
{
    return API_BASE_PTR;
}

uintptr_t ExtractQwordTarget(uintptr_t instruction_address) {
    int32_t relative_offset = *reinterpret_cast<int32_t*>(instruction_address + 3);
    uintptr_t next_instruction = instruction_address + 7;
    uintptr_t target_address = next_instruction + relative_offset;
    return target_address;
}

void InitIl2CppFunctions()
{
    HMODULE hUnityPlayer = GetModuleHandleA("UnityPlayer.dll");
    if (!hUnityPlayer) {
        MessageBoxA(NULL, "UnityPlayer.dll not found!", "Error", MB_OK | MB_ICONERROR);
        ExitProcess(1);
        return;
    }

    uintptr_t target = Scan(hUnityPlayer, "48 8B 05 ? ? ? ? 48 8D 0D ? ? ? ? FF D0");
    DebugPrintA("[INFO] Target: %p\n", (void*)target);

    uintptr_t il2cppFunctionsTable;
    if (target != 0) {
        il2cppFunctionsTable = ExtractQwordTarget(target);
        DebugPrintA("[INFO] il2cpp functions table: %p\n", (void*)il2cppFunctionsTable);
        API_BASE_PTR = il2cppFunctionsTable;
    }
    else {
        MessageBoxA(NULL, "Failed to find il2cpp!", "Error", MB_OK | MB_ICONERROR);
        ExitProcess(1);
        return;
    }
}

uintptr_t GetGameAssemblyModuleBase()
{
    HMODULE mod = GetModuleHandleA("GameAssembly.dll");
    return reinterpret_cast<uintptr_t>(mod);
}

size_t class_byval;
std::once_flag class_byval_once;

size_t brute_class_byval(il2cpp_functions* funcs) {
    Il2CppDomain* domain = funcs->il2cpp_domain_get();
    size_t assemblyCount = 0;
    Il2CppAssembly** assemblies = funcs->il2cpp_domain_get_assemblies(domain, &assemblyCount);

    if (assemblyCount == 0) return 0;

    Il2CppImage* image = funcs->il2cpp_assembly_get_image(assemblies[0]);
    Il2CppClass* klass = funcs->il2cpp_image_get_class(image, 0);
    DebugPrintA("%llX\n", klass);
    size_t offset = 0;
    for (offset = 0; offset < 256; offset += sizeof(void*)) {
        __try {
            uintptr_t ptr = ((uintptr_t)klass + offset);
            Il2CppType* type = (Il2CppType*)ptr;
            const char* typeName = funcs->il2cpp_type_get_name(type);
            if (typeName != nullptr) {
                return offset;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    return 0;
}

size_t get_class_byval_offset(il2cpp_functions* funcs) {
    std::call_once(class_byval_once, [&]() {
        class_byval = brute_class_byval(funcs);

        if (class_byval == 0) {
            printf("[!] Failed to locate Il2CppClass::byval_arg\n");
        }
        else {
            printf("[+] Il2CppClass::byval_arg offset = 0x%zX\n", class_byval);
        }
        });

    return class_byval;
}

template<typename T>
function_ptr<T> il2cpp_functions::resolve(size_t index) {
    void* addr = const_cast<void*>(_table[index]);
    return addr ? function_ptr<T>(addr) : function_ptr<T>();
}

il2cpp_functions::il2cpp_functions() {
    _table = reinterpret_cast<void**>(GetApiBase());

    assembly_get_image = resolve<Il2CppImage * (*)(Il2CppAssembly*)>(22);
    class_get_methods = resolve<MethodInfo * (*)(Il2CppClass*, void**)>(35);
    class_get_name = resolve<const char* (*)(Il2CppClass*)>(37);
    class_get_namespace = resolve<const char* (*)(Il2CppClass*)>(39);
    domain_get = resolve<Il2CppDomain * (*)()>(63);
    domain_get_assemblies = resolve<Il2CppAssembly * *(*)(Il2CppDomain*, size_t*)>(65);

    method_get_name = resolve<const char* (*)(const MethodInfo*)>(117);
    image_get_class_count = resolve<size_t(*)(Il2CppImage*)>(169);
    image_get_class = resolve<Il2CppClass * (*)(Il2CppImage*, size_t)>(170);

    class_get_fields = resolve<FieldInfo * (*)(Il2CppClass*, void**)>(31);
    class_get_parent = resolve<Il2CppClass * (*)(Il2CppClass*)>(40);
    class_is_valuetype = resolve<bool(*)(const Il2CppClass*)>(43);
    class_get_flags = resolve<int32_t(*)(const Il2CppClass*)>(45);
    class_from_type = resolve<Il2CppClass * (*)(const Il2CppType*)>(49);
    class_is_enum = resolve<bool(*)(const Il2CppClass*)>(53);

    field_get_flags = resolve<int32_t(*)(FieldInfo*)>(72);
    field_get_name = resolve<const char* (*)(FieldInfo*)>(73);
    field_get_offset = resolve<size_t(*)(FieldInfo*)>(75);
    field_get_type = resolve<Il2CppType * (*)(FieldInfo*)>(76);

    method_get_return_type = resolve<Il2CppType * (*)(const MethodInfo*)>(116);
    method_get_param_count = resolve<uint32_t(*)(const MethodInfo*)>(123);
    method_get_param = resolve<Il2CppType * (*)(const MethodInfo*, uint32_t)>(124);

    type_get_name = resolve<const char* (*)(Il2CppType*)>(161);
    type_is_byref = resolve<bool(*)(Il2CppType*)>(162);
    type_get_attrs = resolve<uint32_t(*)(Il2CppType*)>(163);

    image_get_name = resolve<const char* (*)(Il2CppImage*)>(168);
}

bool il2cpp_functions::is_valid() const {
    return assembly_get_image.valid() && class_get_methods.valid();
}

Il2CppType* il2cpp_functions::il2cpp_class_get_type(Il2CppClass* klass) {
    return (Il2CppType*)((uintptr_t)klass + get_class_byval_offset(this));
}

uintptr_t il2cpp_functions::il2cpp_method_get_relative_pointer(MethodInfo* method) {
    auto ptr = *reinterpret_cast<uintptr_t*>((uintptr_t)method + 0x08);
    if (ptr == 0) return 0;
    return ptr - GetGameAssemblyModuleBase();
}

// Wrappers
Il2CppImage* il2cpp_functions::il2cpp_assembly_get_image(Il2CppAssembly* assembly) {
    return assembly_get_image.valid() ? assembly_get_image.get()(assembly) : nullptr;
}

FieldInfo* il2cpp_functions::il2cpp_class_get_fields(Il2CppClass* klass, void** iter) {
    return class_get_fields.valid() ? class_get_fields.get()(klass, iter) : nullptr;
}

MethodInfo* il2cpp_functions::il2cpp_class_get_methods(Il2CppClass* klass, void** iter) {
    return class_get_methods.valid() ? class_get_methods.get()(klass, iter) : nullptr;
}

const char* il2cpp_functions::il2cpp_class_get_name(Il2CppClass* klass) {
    return class_get_name.valid() ? class_get_name.get()(klass) : nullptr;
}

const char* il2cpp_functions::il2cpp_class_get_namespace(Il2CppClass* klass) {
    return class_get_namespace.valid() ? class_get_namespace.get()(klass) : nullptr;
}

Il2CppClass* il2cpp_functions::il2cpp_class_get_parent(Il2CppClass* klass) {
    return class_get_parent.valid() ? class_get_parent.get()(klass) : nullptr;
}

bool il2cpp_functions::il2cpp_class_is_valuetype(const Il2CppClass* klass) {
    return class_is_valuetype.valid() && class_is_valuetype.get()(klass);
}

int32_t il2cpp_functions::il2cpp_class_get_flags(const Il2CppClass* klass) {
    return class_get_flags.valid() ? class_get_flags.get()(klass) : 0;
}

Il2CppClass* il2cpp_functions::il2cpp_class_from_type(const Il2CppType* type) {
    return class_from_type.valid() ? class_from_type.get()(type) : nullptr;
}

bool il2cpp_functions::il2cpp_class_is_enum(const Il2CppClass* klass) {
    return class_is_enum.valid() && class_is_enum.get()(klass);
}

Il2CppDomain* il2cpp_functions::il2cpp_domain_get() {
    return domain_get.valid() ? domain_get.get()() : nullptr;
}

Il2CppAssembly** il2cpp_functions::il2cpp_domain_get_assemblies(Il2CppDomain* domain, size_t* size) {
    return domain_get_assemblies.valid() ? domain_get_assemblies.get()(domain, size) : nullptr;
}

int32_t il2cpp_functions::il2cpp_field_get_flags(FieldInfo* field) {
    return field_get_flags.valid() ? field_get_flags.get()(field) : 0;
}

const char* il2cpp_functions::il2cpp_field_get_name(FieldInfo* field) {
    return field_get_name.valid() ? field_get_name.get()(field) : nullptr;
}

size_t il2cpp_functions::il2cpp_field_get_offset(FieldInfo* field) {
    return field_get_offset.valid() ? field_get_offset.get()(field) : 0;
}

Il2CppType* il2cpp_functions::il2cpp_field_get_type(FieldInfo* field) {
    return field_get_type.valid() ? field_get_type.get()(field) : nullptr;
}

Il2CppType* il2cpp_functions::il2cpp_method_get_return_type(const MethodInfo* method) {
    return method_get_return_type.valid() ? method_get_return_type.get()(method) : nullptr;
}

const char* il2cpp_functions::il2cpp_method_get_name(const MethodInfo* method) {
    return method_get_name.valid() ? method_get_name.get()(method) : nullptr;
}

uint32_t il2cpp_functions::il2cpp_method_get_param_count(const MethodInfo* method) {
    return method_get_param_count.valid() ? method_get_param_count.get()(method) : 0;
}

Il2CppType* il2cpp_functions::il2cpp_method_get_param(const MethodInfo* method, uint32_t index) {
    return method_get_param.valid() ? method_get_param.get()(method, index) : nullptr;
}

const char* il2cpp_functions::il2cpp_type_get_name(Il2CppType* type) {
    return type_get_name.valid() ? type_get_name.get()(type) : nullptr;
}

bool il2cpp_functions::il2cpp_type_is_byref(Il2CppType* type) {
    return type_is_byref.valid() && type_is_byref.get()(type);
}

uint32_t il2cpp_functions::il2cpp_type_get_attrs(Il2CppType* type) {
    return type_get_attrs.valid() ? type_get_attrs.get()(type) : 0;
}

const char* il2cpp_functions::il2cpp_image_get_name(Il2CppImage* image) {
    return image_get_name.valid() ? image_get_name.get()(image) : nullptr;
}

size_t il2cpp_functions::il2cpp_image_get_class_count(Il2CppImage* image) {
    return image_get_class_count.valid() ? image_get_class_count.get()(image) : 0;
}

Il2CppClass* il2cpp_functions::il2cpp_image_get_class(Il2CppImage* image, size_t index) {
    return image_get_class.valid() ? image_get_class.get()(image, index) : nullptr;
}
