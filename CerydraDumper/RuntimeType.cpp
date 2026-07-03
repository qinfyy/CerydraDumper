#include "pch.h"
#include "RuntimeType.h"
#include "MonoAssembly.h"

namespace Cerydra::CSharp
{
    Cerydra::IL2CPP::Class* RuntimeType::RuntimeTypeClass()
    {
        static auto* klass = RequireClass("System.RuntimeType");
        return klass;
    }

    Cerydra::IL2CPP::Class* RuntimeType::TypeClass()
    {
        static auto* klass = RequireClass("System.Type");
        return klass;
    }

    RuntimeType* RuntimeType::FromClass(Cerydra::IL2CPP::Class* klass)
    {
        return klass ? klass->GetTypeObject() : nullptr;
    }

    RuntimeType* RuntimeType::FromName(const std::string& name)
    {
        auto* klass = Il2CppRuntimeCache::GetClass(name);
        if (!klass) {
            throw std::runtime_error("找不到类: " + name);
        }

        return FromClass(klass);
    }

    Cerydra::IL2CPP::Method* RuntimeType::GetBaseTypeMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "get_BaseType", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::IsGenericTypeMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "get_IsGenericType", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::IsEnumMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "get_IsEnum", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetArrayRankMethod() { static auto* method = RequireMethod(TypeClass(), "GetArrayRank", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetReflectedTypeMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "get_ReflectedType", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetElementTypeMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "GetElementType", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetNamespaceMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "get_Namespace", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetNameMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "get_Name", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetFullNameMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "get_FullName", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetTypeHandleMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "get_TypeHandle", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetGenericArgumentsMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "GetGenericArguments", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetFieldsMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "GetFields", { "System.Reflection.BindingFlags" }); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetPropertiesMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "GetProperties", { "System.Reflection.BindingFlags" }); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::IsByRefMethod() { static auto* method = RequireMethod(TypeClass(), "get_IsByRef", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::IsArrayMethod() { static auto* method = RequireMethod(TypeClass(), "get_IsArray", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::IsValueTypeMethod() { static auto* method = RequireMethod(TypeClass(), "get_IsValueType", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::IsPointerMethod() { static auto* method = RequireMethod(TypeClass(), "get_IsPointer", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetTypeMethod() { static auto* method = RequireMethod(TypeClass(), "GetType", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetPropertyMethod() { static auto* method = RequireMethod(TypeClass(), "GetProperty", { "string" }); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::IsInterfaceMethod() { static auto* method = RequireMethod(TypeClass(), "get_IsInterface", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetConstructorMethod() { static auto* method = RequireMethod(TypeClass(), "GetConstructor", { "System.Type[]" }); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::IsAssignableFromMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "IsAssignableFrom", { "System.Type" }); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetInterfacesMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "GetInterfaces", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetDeclaringTypeMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "get_DeclaringType", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetAssemblyMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "get_Assembly", {}); return method; }
    Cerydra::IL2CPP::Method* RuntimeType::GetFieldMethod() { static auto* method = RequireMethod(RuntimeTypeClass(), "GetField", { "string", "System.Reflection.BindingFlags" }); return method; }

    RuntimeType* RuntimeType::GetBaseType() const { return InvokeInstance<RuntimeType*>(this, GetBaseTypeMethod()); }
    bool RuntimeType::IsGenericType() const { return InvokeInstance<bool>(this, IsGenericTypeMethod()); }
    bool RuntimeType::IsEnum() const { return InvokeInstance<bool>(this, IsEnumMethod()); }
    int RuntimeType::GetArrayRank() const { return InvokeInstance<int>(this, GetArrayRankMethod()); }
    RuntimeType* RuntimeType::GetReflectedType() const { return InvokeInstance<RuntimeType*>(this, GetReflectedTypeMethod()); }
    RuntimeType* RuntimeType::GetElementType() const { return InvokeInstance<RuntimeType*>(this, GetElementTypeMethod()); }
    SystemString* RuntimeType::GetNamespace() const { return InvokeInstance<SystemString*>(this, GetNamespaceMethod()); }
    SystemString* RuntimeType::GetName() const { return InvokeInstance<SystemString*>(this, GetNameMethod()); }
    SystemString* RuntimeType::GetFullName() const { return InvokeInstance<SystemString*>(this, GetFullNameMethod()); }
    Object* RuntimeType::GetTypeHandle() const { return InvokeInstance<Object*>(this, GetTypeHandleMethod()); }
    Array<RuntimeType*>* RuntimeType::GetGenericArguments() const { return InvokeInstance<Array<RuntimeType*>*>(this, GetGenericArgumentsMethod()); }
    Array<MonoField*>* RuntimeType::GetFields(int bindingFlags) const { return InvokeInstance<Array<MonoField*>*, int>(this, GetFieldsMethod(), bindingFlags); }
    Array<MonoProperty*>* RuntimeType::GetProperties(int bindingFlags) const { return InvokeInstance<Array<MonoProperty*>*, int>(this, GetPropertiesMethod(), bindingFlags); }
    bool RuntimeType::IsByRef() const { return InvokeInstance<bool>(this, IsByRefMethod()); }
    bool RuntimeType::IsArray() const { return InvokeInstance<bool>(this, IsArrayMethod()); }
    bool RuntimeType::IsValueType() const { return InvokeInstance<bool>(this, IsValueTypeMethod()); }
    bool RuntimeType::IsPointer() const { return InvokeInstance<bool>(this, IsPointerMethod()); }
    Object* RuntimeType::GetType() const { return InvokeInstance<Object*>(this, GetTypeMethod()); }
    bool RuntimeType::IsInterface() const { return InvokeInstance<bool>(this, IsInterfaceMethod()); }
    Object* RuntimeType::GetConstructor(Array<RuntimeType*>* types) const { return InvokeInstance<Object*>(this, GetConstructorMethod(), types); }
    bool RuntimeType::IsAssignableFrom(RuntimeType* type) const { return InvokeInstance<bool>(this, IsAssignableFromMethod(), type); }
    Array<RuntimeType*>* RuntimeType::GetInterfaces() const { return InvokeInstance<Array<RuntimeType*>*>(this, GetInterfacesMethod()); }
    RuntimeType* RuntimeType::GetDeclaringType() const { return InvokeInstance<RuntimeType*>(this, GetDeclaringTypeMethod()); }

    MonoProperty* RuntimeType::GetProperty(SystemString* name) const
    {
        return InvokeInstance<MonoProperty*>(this, GetPropertyMethod(), name);
    }

    SystemString* RuntimeType::GetAssemblyName() const
    {
        auto* assembly = InvokeInstance<MonoAssembly*>(this, GetAssemblyMethod());
        return assembly ? assembly->GetName() : nullptr;
    }

    MonoField* RuntimeType::GetFieldObject(SystemString* name, int bindingFlags) const
    {
        return InvokeInstance<MonoField*, SystemString*, int>(this, GetFieldMethod(), name, bindingFlags);
    }

    Cerydra::IL2CPP::Field* RuntimeType::GetField(const char* name) const
    {
        auto* klass = GetMetaClass();
        while (klass) {
            if (auto* field = klass->GetField(name)) {
                return field;
            }
            klass = klass->parentClass;
        }

        throw std::runtime_error("找不到字段: " + std::string(name));
    }

    Cerydra::IL2CPP::Type* RuntimeType::GetMetaType() const
    {
        const auto typePtr = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(this) + 16);
        return Il2CppRuntimeCache::GetTypeByAddress(typePtr);
    }

    Cerydra::IL2CPP::Class* RuntimeType::GetMetaClass() const
    {
        auto* type = GetMetaType();
        return type ? type->klass : nullptr;
    }

    Cerydra::IL2CPP::Class* RuntimeFieldHandle::StaticClass() { static auto* klass = RequireClass("System.RuntimeFieldHandle"); return klass; }
    Cerydra::IL2CPP::Method* RuntimeFieldHandle::GetValueMethod() { static auto* method = RequireMethod(StaticClass(), "get_Value", {}); return method; }
    uintptr_t RuntimeFieldHandle::GetValue() const { return InvokeInstance<uintptr_t>(this, GetValueMethod()); }

    Cerydra::IL2CPP::Class* MonoField::RuntimeFieldInfoClass() { static auto* klass = RequireClass("System.Reflection.RuntimeFieldInfo"); return klass; }
    Cerydra::IL2CPP::Class* MonoField::FieldInfoClass() { static auto* klass = RequireClass("System.Reflection.FieldInfo"); return klass; }
    Cerydra::IL2CPP::Method* MonoField::GetDeclaringTypeMethod() { static auto* method = RequireMethod(RuntimeFieldInfoClass(), "get_DeclaringType", {}); return method; }
    Cerydra::IL2CPP::Method* MonoField::GetFieldTypeMethod() { static auto* method = RequireMethod(RuntimeFieldInfoClass(), "get_FieldType", {}); return method; }
    Cerydra::IL2CPP::Method* MonoField::GetNameMethod() { static auto* method = RequireMethod(RuntimeFieldInfoClass(), "get_Name", {}); return method; }
    Cerydra::IL2CPP::Method* MonoField::GetFieldHandleMethod() { static auto* method = RequireMethod(RuntimeFieldInfoClass(), "get_FieldHandle", {}); return method; }
    Cerydra::IL2CPP::Method* MonoField::GetRawConstantValueMethod() { static auto* method = RequireMethod(RuntimeFieldInfoClass(), "GetRawConstantValue", {}); return method; }
    Cerydra::IL2CPP::Method* MonoField::GetCustomAttributesMethod() { static auto* method = RequireMethod(RuntimeFieldInfoClass(), "GetCustomAttributes", { "bool" }); return method; }
    Cerydra::IL2CPP::Method* MonoField::GetValueMethod() { static auto* method = RequireMethod(RuntimeFieldInfoClass(), "GetValue", { "object" }); return method; }
    Cerydra::IL2CPP::Method* MonoField::IsLiteralMethod() { static auto* method = RequireMethod(FieldInfoClass(), "get_IsLiteral", {}); return method; }
    Cerydra::IL2CPP::Method* MonoField::GetMetadataTokenMethod() { static auto* method = RequireMethod(RuntimeFieldInfoClass(), "get_MetadataToken", {}); return method; }
    Cerydra::IL2CPP::Method* MonoField::SetValueMethod() { static auto* method = RequireMethod(FieldInfoClass(), "SetValue", { "object", "object" }); return method; }

    RuntimeType* MonoField::GetDeclaringType() const { return InvokeInstance<RuntimeType*>(this, GetDeclaringTypeMethod()); }
    RuntimeType* MonoField::GetFieldType() const { return InvokeInstance<RuntimeType*>(this, GetFieldTypeMethod()); }
    SystemString* MonoField::GetName() const { return InvokeInstance<SystemString*>(this, GetNameMethod()); }
    RuntimeFieldHandle* MonoField::GetFieldHandle() const { return InvokeInstance<RuntimeFieldHandle*>(this, GetFieldHandleMethod()); }
    Object* MonoField::GetRawConstantValue() const { return InvokeInstance<Object*>(this, GetRawConstantValueMethod()); }
    Array<Object*>* MonoField::GetCustomAttributes(bool inherit) const { return InvokeInstance<Array<Object*>*, bool>(this, GetCustomAttributesMethod(), inherit); }
    Object* MonoField::GetValue(Object* obj) const { return InvokeInstance<Object*>(this, GetValueMethod(), obj); }
    bool MonoField::IsLiteral() const { return InvokeInstance<bool>(this, IsLiteralMethod()); }
    int32_t MonoField::GetMetadataToken() const { return InvokeInstance<int32_t>(this, GetMetadataTokenMethod()); }
    void MonoField::SetValue(Object* obj, Object* value) const { InvokeInstance<void>(this, SetValueMethod(), obj, value); }

    Cerydra::IL2CPP::Field* MonoField::GetMetaField() const
    {
        const auto fieldPtr = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(this) + 24);
        return Il2CppRuntimeCache::GetFieldByAddress(fieldPtr);
    }

    Cerydra::IL2CPP::Class* MonoProperty::RuntimePropertyInfoClass() { static auto* klass = RequireClass("System.Reflection.RuntimePropertyInfo"); return klass; }
    Cerydra::IL2CPP::Class* MonoProperty::PropertyInfoClass() { static auto* klass = RequireClass("System.Reflection.PropertyInfo"); return klass; }
    Cerydra::IL2CPP::Method* MonoProperty::GetDeclaringTypeMethod() { static auto* method = RequireMethod(RuntimePropertyInfoClass(), "get_DeclaringType", {}); return method; }
    Cerydra::IL2CPP::Method* MonoProperty::GetPropertyTypeMethod() { static auto* method = RequireMethod(RuntimePropertyInfoClass(), "get_PropertyType", {}); return method; }
    Cerydra::IL2CPP::Method* MonoProperty::GetNameMethod() { static auto* method = RequireMethod(RuntimePropertyInfoClass(), "get_Name", {}); return method; }
    Cerydra::IL2CPP::Method* MonoProperty::SetValueMethod() { static auto* method = RequireMethod(PropertyInfoClass(), "SetValue", { "object", "object" }); return method; }
    Cerydra::IL2CPP::Method* MonoProperty::GetValueMethod() { static auto* method = RequireMethod(PropertyInfoClass(), "GetValue", { "object" }); return method; }
    Cerydra::IL2CPP::Method* MonoProperty::GetValueWithArgsMethod() { static auto* method = RequireMethod(PropertyInfoClass(), "GetValue", { "object", "System.Object[]" }); return method; }

    RuntimeType* MonoProperty::GetDeclaringType() const { return InvokeInstance<RuntimeType*>(this, GetDeclaringTypeMethod()); }
    RuntimeType* MonoProperty::GetPropertyType() const { return InvokeInstance<RuntimeType*>(this, GetPropertyTypeMethod()); }
    SystemString* MonoProperty::GetName() const { return InvokeInstance<SystemString*>(this, GetNameMethod()); }
    void MonoProperty::SetValue(Object* obj, Object* value) const { InvokeInstance<void>(this, SetValueMethod(), obj, value); }
    Object* MonoProperty::GetValue(Object* obj) const { return InvokeInstance<Object*>(this, GetValueMethod(), obj); }
    Object* MonoProperty::GetValue(Object* obj, Array<Object*>* args) const { return InvokeInstance<Object*>(this, GetValueWithArgsMethod(), obj, args); }

    Cerydra::IL2CPP::Class* SystemObject::StaticClass() { static auto* klass = RequireClass("System.Object"); return klass; }
    Cerydra::IL2CPP::Method* SystemObject::GetTypeMethod() { static auto* method = RequireMethod(StaticClass(), "GetType", {}); return method; }
    RuntimeType* SystemObject::GetType() const { return InvokeInstance<RuntimeType*>(this, GetTypeMethod()); }

    Cerydra::IL2CPP::Class* SystemInt32::StaticClass() { static auto* klass = RequireClass("System.Int32"); return klass; }
    Cerydra::IL2CPP::Method* SystemInt32::ToStringMethod() { static auto* method = RequireMethod(StaticClass(), "ToString", {}); return method; }
    SystemString* SystemInt32::ToString() const { return InvokeInstance<SystemString*>(this, ToStringMethod()); }

    Cerydra::IL2CPP::Class* SystemInt64::StaticClass() { static auto* klass = RequireClass("System.Int64"); return klass; }
    Cerydra::IL2CPP::Method* SystemInt64::ToStringMethod() { static auto* method = RequireMethod(StaticClass(), "ToString", {}); return method; }
    SystemInt64* SystemInt64::FromAddress(uintptr_t ptr) { return reinterpret_cast<SystemInt64*>(ptr); }
    SystemString* SystemInt64::ToString() const { return InvokeInstance<SystemString*>(this, ToStringMethod()); }

    SystemString* SystemDynamic::ToString() const
    {
        return InvokeDynamic<SystemString*>(this, "ToString", {});
    }
}
