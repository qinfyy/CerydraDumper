#include "pch.h"
//#include "Il2CppApi.h"
//#include "Il2CppFunctions.h"
//#include <vector>
//#include <cstring>
//
//#define IL2CPP_API(i, name, ret, params, args) \
//ret name params { \
//    using Func = ret(__fastcall*)params; \
//    auto fn = reinterpret_cast<Func>(*reinterpret_cast<uintptr_t*>(GetApiBase() + i * 8)); \
//    return fn args; \
//}
//
//// API 实现
//IL2CPP_API(21, il2cpp_get_corlib, Il2CppImage, (), ())
//IL2CPP_API(22, il2cpp_assembly_get_image, Il2CppImage, (Il2CppAssembly assembly), (assembly))
//IL2CPP_API(31, il2cpp_class_get_fields, Il2CppField, (Il2CppClass klass, void** iter), (klass, iter))
//IL2CPP_API(33, il2cpp_class_get_interface, uintptr_t, (Il2CppClass klass), (klass))
//IL2CPP_API(35, il2cpp_class_get_methods, Il2CppMethod, (Il2CppClass klass, void** iter), (klass, iter))
//IL2CPP_API(37, il2cpp_class_get_name, const char*, (Il2CppClass klass), (klass))
//IL2CPP_API(39, il2cpp_class_get_namespace, const char*, (Il2CppClass klass), (klass))
//IL2CPP_API(40, il2cpp_class_get_parent, Il2CppClass, (Il2CppClass klass), (klass))
//IL2CPP_API(43, il2cpp_class_is_value_type, bool, (Il2CppClass klass), (klass))
//IL2CPP_API(45, il2cpp_class_get_flags, int32_t, (Il2CppClass klass), (klass))
//IL2CPP_API(49, il2cpp_class_from_type, Il2CppClass, (Il2CppType type), (type))
//IL2CPP_API(53, il2cpp_class_is_enum, bool, (Il2CppClass klass), (klass))
//IL2CPP_API(63, il2cpp_domain_get, Il2CppDomain, (), ())
//IL2CPP_API(64, il2cpp_domain_assembly_open, Il2CppAssembly, (Il2CppDomain domain, const char* name), (domain, name))
//IL2CPP_API(65, il2cpp_domain_get_assemblies, Il2CppAssembly*, (Il2CppDomain domain, size_t* size), (domain, size))
//IL2CPP_API(72, il2cpp_field_get_flags, int32_t, (Il2CppField field), (field))
//IL2CPP_API(73, il2cpp_field_get_name, const char*, (Il2CppField field), (field))
//IL2CPP_API(75, il2cpp_field_get_offset, size_t, (Il2CppField field), (field))
//IL2CPP_API(76, il2cpp_field_get_type, Il2CppType, (Il2CppField field), (field))
//IL2CPP_API(77, il2cpp_field_get_value_object, Il2CppObject, (Il2CppField field, Il2CppObject obj), (field, obj))
//IL2CPP_API(116, il2cpp_method_get_return_type, Il2CppType, (Il2CppMethod method), (method))
//IL2CPP_API(117, il2cpp_method_get_name, const char*, (Il2CppMethod method), (method))
//IL2CPP_API(123, il2cpp_method_get_param_count, uint32_t, (Il2CppMethod method), (method))
//IL2CPP_API(124, il2cpp_method_get_param, Il2CppType, (Il2CppMethod method, uint32_t index), (method, index))
//IL2CPP_API(127, il2cpp_object_get_class, Il2CppClass, (Il2CppObject obj), (obj))
//IL2CPP_API(129, il2cpp_object_get_virtual_method, Il2CppMethod, (Il2CppObject obj, Il2CppMethod method), (obj, method))
//IL2CPP_API(130, il2cpp_object_new, Il2CppObject, (Il2CppClass klass), (klass))
//IL2CPP_API(131, il2cpp_object_unbox, Il2CppObject, (Il2CppObject obj), (obj))
//IL2CPP_API(154, il2cpp_thread_attach, uintptr_t, (Il2CppDomain domain), (domain))
//IL2CPP_API(161, il2cpp_type_get_name, const char*, (Il2CppType type), (type))
//IL2CPP_API(162, il2cpp_type_is_by_ref, bool, (Il2CppType type), (type))
//IL2CPP_API(163, il2cpp_type_get_attrs, uint32_t, (Il2CppType type), (type))
//IL2CPP_API(168, il2cpp_image_get_name, const char*, (Il2CppImage image), (image))
//IL2CPP_API(169, il2cpp_image_get_class_count, size_t, (Il2CppImage image), (image))
//IL2CPP_API(170, il2cpp_image_get_class, Il2CppClass, (Il2CppImage image, size_t index), (image, index))
//
//// Il2CppDomain
//Il2CppDomain::Il2CppDomain(uintptr_t p) : ptr(p) {
//}
//Il2CppDomain Il2CppDomain::Get() {
//    return il2cpp_domain_get();
//}
//std::vector<Il2CppAssembly> Il2CppDomain::assemblies() const {
//    size_t count = 0;
//    auto arr = il2cpp_domain_get_assemblies(*this, &count);
//    std::vector<Il2CppAssembly> result;
//    for (size_t i = 0; i < count; i++) result.emplace_back(arr[i]);
//    return result;
//}
//Il2CppAssembly Il2CppDomain::assembly_open(const char* name) const {
//    return il2cpp_domain_assembly_open(*this, name);
//}
//Il2CppDomain::operator uintptr_t() const {
//    return ptr;
//}
//
//// Il2CppAssembly
//Il2CppAssembly::Il2CppAssembly(uintptr_t p) : ptr(p) {
//}
//Il2CppImage Il2CppAssembly::get_image() const {
//    return il2cpp_assembly_get_image(*this);
//}
//Il2CppAssembly::operator uintptr_t() const {
//    return ptr;
//}
//
//// Il2CppImage
//Il2CppImage::Il2CppImage(uintptr_t p) : ptr(p) {
//}
//const char* Il2CppImage::name() const {
//    return il2cpp_image_get_name(*this);
//}
//size_t Il2CppImage::class_count() const {
//    return il2cpp_image_get_class_count(*this);
//}
//std::vector<Il2CppClass> Il2CppImage::classes() const {
//    std::vector<Il2CppClass> result;
//    auto count = class_count();
//    for (size_t i = 0; i < count; i++) result.emplace_back(il2cpp_image_get_class(*this, i));
//    return result;
//}
//Il2CppImage::operator uintptr_t() const {
//    return ptr;
//}
//
//// Il2CppClass
//Il2CppClass::Il2CppClass(uintptr_t p) : ptr(p) {
//}
//const char* Il2CppClass::name() const {
//    return il2cpp_class_get_name(*this);
//}
//const char* Il2CppClass::namespaze() const {
//    return il2cpp_class_get_namespace(*this);
//}
//Il2CppClass Il2CppClass::get_parent() const {
//    return il2cpp_class_get_parent(*this);
//}
//Il2CppType Il2CppClass::byval_arg() const {
//    return Il2CppType(ptr + 128);
//}
//std::vector<Il2CppMethod> Il2CppClass::methods() const {
//    void* iter = nullptr;
//    std::vector<Il2CppMethod> result;
//    while (true) {
//        auto m = il2cpp_class_get_methods(*this, &iter);
//        if (!m.ptr) break;
//        result.push_back(m);
//    }
//    return result;
//}
//std::vector<Il2CppField> Il2CppClass::fields() const {
//    void* iter = nullptr;
//    std::vector<Il2CppField> result;
//    while (true) {
//        auto f = il2cpp_class_get_fields(*this, &iter);
//        if (!f.ptr) break;
//        result.push_back(f);
//    }
//    return result;
//}
//int32_t Il2CppClass::get_flags() const {
//    return il2cpp_class_get_flags(*this);
//}
//bool Il2CppClass::is_enum() const {
//    return il2cpp_class_is_enum(*this);
//}
//bool Il2CppClass::is_value_type() const {
//    return il2cpp_class_is_value_type(*this);
//}
//Il2CppMethod Il2CppClass::find_method_by_name(const char* name) const {
//    for (auto& m : methods()) {
//        if (std::strcmp(m.name(), name) == 0) return m;
//    }
//    return Il2CppMethod(0);
//}
//Il2CppClass::operator uintptr_t() const {
//    return ptr;
//}
//
//// Il2CppType
//Il2CppType::Il2CppType(uintptr_t p) : ptr(p) {
//}
//const char* Il2CppType::name() const {
//    return il2cpp_type_get_name(*this);
//}
//uint32_t Il2CppType::get_attrs() const {
//    return il2cpp_type_get_attrs(*this);
//}
//bool Il2CppType::is_by_ref() const {
//    return il2cpp_type_is_by_ref(*this);
//}
//Il2CppClass Il2CppType::get_class() const {
//    return il2cpp_class_from_type(*this);
//}
//Il2CppType::operator uintptr_t() const {
//    return ptr;
//}
//
//// Il2CppMethod
//Il2CppMethod::Il2CppMethod(uintptr_t p) : ptr(p) {
//}
//const char* Il2CppMethod::name() const {
//    return il2cpp_method_get_name(*this);
//}
//Il2CppType Il2CppMethod::return_type() const {
//    return il2cpp_method_get_return_type(*this);
//}
//uint32_t Il2CppMethod::param_count() const {
//    return il2cpp_method_get_param_count(*this);
//}
//Il2CppType Il2CppMethod::param(uint32_t i) const {
//    return il2cpp_method_get_param(*this, i);
//}
//uintptr_t Il2CppMethod::va() const {
//    return *reinterpret_cast<uintptr_t*>(ptr + 8);
//}
//uintptr_t Il2CppMethod::rva() const {
//    return va() ? va() - GetGameAssemblyModuleBase() : 0;
//}
//MethodInfo* Il2CppMethod::info() const {
//    return reinterpret_cast<MethodInfo*>(ptr);
//}
//bool Il2CppMethod::is_valid() const {
//    return info()->method_pointer != nullptr;
//}
//Il2CppMethod::operator uintptr_t() const {
//    return ptr;
//}
//
//// Il2CppField
//Il2CppField::Il2CppField(uintptr_t p) : ptr(p) {
//}
//const char* Il2CppField::name() const {
//    return il2cpp_field_get_name(*this);
//}
//int32_t Il2CppField::get_flags() const {
//    return il2cpp_field_get_flags(*this);
//}
//size_t Il2CppField::get_offset() const {
//    return il2cpp_field_get_offset(*this);
//}
//Il2CppType Il2CppField::get_type() const {
//    return il2cpp_field_get_type(*this);
//}
//Il2CppObject Il2CppField::get_value_object(Il2CppObject instance) const {
//    return il2cpp_field_get_value_object(*this, instance);
//}
//Il2CppField::operator uintptr_t() const {
//    return ptr;
//}
//
//// Il2CppObject
//const Il2CppObject Il2CppObject::NULL_OBJ = Il2CppObject(0);
//
//// 构造函数
//Il2CppObject::Il2CppObject(uintptr_t p) : ptr(p) {
//}
//
//// 判断是否为空
//bool Il2CppObject::is_null() const {
//    return ptr == NULL;
//}
//
//// 获取对象的类
//Il2CppClass Il2CppObject::get_class() const {
//    if (is_null()) return Il2CppClass(0);
//    return Il2CppClass(*reinterpret_cast<uintptr_t*>(ptr));
//}