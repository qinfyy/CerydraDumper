#include "pch.h"
#include "RuntimeType.h"
#include "MonoAssembly.h"

namespace Cerydra::CSharp
{
    Cerydra::Il2Cpp::Class* RuntimeType::RuntimeTypeClass()
    {
        static auto* klass = RequireClass("System.RuntimeType");
        return klass;
    }

    Cerydra::Il2Cpp::Class* RuntimeType::TypeClass()
    {
        static auto* klass = RequireClass("System.Type");
        return klass;
    }

    RuntimeType* RuntimeType::FromClass(Cerydra::Il2Cpp::Class* klass)
    {
        return klass ? klass->GetTypeObject() : nullptr;
    }

    RuntimeType* RuntimeType::FromName(const std::string& name)
    {
        auto* klass = Cerydra::Il2Cpp::FindClass(name);
        if (!klass) {
            throw std::runtime_error("找不到类: " + name);
        }

        return FromClass(klass);
    }

    RuntimeType* RuntimeType::GetBaseType() const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "get_BaseType", {});
        return InvokeInstance<RuntimeType*>(this, method);
    }
    bool RuntimeType::IsGenericType() const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "get_IsGenericType", {});
        return InvokeInstance<bool>(this, method);
    }
    bool RuntimeType::IsEnum() const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "get_IsEnum", {});
        return InvokeInstance<bool>(this, method);
    }
    int RuntimeType::GetArrayRank() const
    {
        static auto* method = RequireMethod(TypeClass(), "GetArrayRank", {});
        return InvokeInstance<int>(this, method);
    }
    RuntimeType* RuntimeType::GetReflectedType() const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "get_ReflectedType", {});
        return InvokeInstance<RuntimeType*>(this, method);
    }
    RuntimeType* RuntimeType::GetElementType() const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "GetElementType", {});
        return InvokeInstance<RuntimeType*>(this, method);
    }
    SystemString* RuntimeType::GetNamespace() const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "get_Namespace", {});
        return InvokeInstance<SystemString*>(this, method);
    }
    SystemString* RuntimeType::GetName() const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "get_Name", {});
        return InvokeInstance<SystemString*>(this, method);
    }
    SystemString* RuntimeType::GetFullName() const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "get_FullName", {});
        return InvokeInstance<SystemString*>(this, method);
    }
    Object* RuntimeType::GetTypeHandle() const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "get_TypeHandle", {});
        return InvokeInstance<Object*>(this, method);
    }
    Array<RuntimeType*>* RuntimeType::GetGenericArguments() const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "GetGenericArguments", {});
        return InvokeInstance<Array<RuntimeType*>*>(this, method);
    }
    Array<MonoField*>* RuntimeType::GetFields(BindingFlags bindingFlags) const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "GetFields", { "System.Reflection.BindingFlags" });
        return InvokeInstance<Array<MonoField*>*, int>(this, method, static_cast<int>(bindingFlags));
    }
    Array<MonoProperty*>* RuntimeType::GetProperties(BindingFlags bindingFlags) const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "GetProperties", { "System.Reflection.BindingFlags" });
        return InvokeInstance<Array<MonoProperty*>*, int>(this, method, static_cast<int>(bindingFlags));
    }
    bool RuntimeType::IsByRef() const
    {
        static auto* method = RequireMethod(TypeClass(), "get_IsByRef", {});
        return InvokeInstance<bool>(this, method);
    }
    bool RuntimeType::IsArray() const
    {
        static auto* method = RequireMethod(TypeClass(), "get_IsArray", {});
        return InvokeInstance<bool>(this, method);
    }
    bool RuntimeType::IsValueType() const
    {
        static auto* method = RequireMethod(TypeClass(), "get_IsValueType", {});
        return InvokeInstance<bool>(this, method);
    }
    bool RuntimeType::IsPointer() const
    {
        static auto* method = RequireMethod(TypeClass(), "get_IsPointer", {});
        return InvokeInstance<bool>(this, method);
    }
    Object* RuntimeType::GetType() const
    {
        static auto* method = RequireMethod(TypeClass(), "GetType", {});
        return InvokeInstance<Object*>(this, method);
    }
    bool RuntimeType::IsInterface() const
    {
        static auto* method = RequireMethod(TypeClass(), "get_IsInterface", {});
        return InvokeInstance<bool>(this, method);
    }
    Object* RuntimeType::GetConstructor(Array<RuntimeType*>* types) const
    {
        static auto* method = RequireMethod(TypeClass(), "GetConstructor", { "System.Type[]" });
        return InvokeInstance<Object*>(this, method, types);
    }
    bool RuntimeType::IsAssignableFrom(RuntimeType* type) const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "IsAssignableFrom", { "System.Type" });
        return InvokeInstance<bool>(this, method, type);
    }
    Array<RuntimeType*>* RuntimeType::GetInterfaces() const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "GetInterfaces", {});
        return InvokeInstance<Array<RuntimeType*>*>(this, method);
    }
    RuntimeType* RuntimeType::GetDeclaringType() const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "get_DeclaringType", {});
        return InvokeInstance<RuntimeType*>(this, method);
    }

    MonoProperty* RuntimeType::GetProperty(SystemString* name) const
    {
        static auto* method = RequireMethod(TypeClass(), "GetProperty", { "string" });
        return InvokeInstance<MonoProperty*>(this, method, name);
    }

    SystemString* RuntimeType::GetAssemblyName() const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "get_Assembly", {});
        auto* assembly = InvokeInstance<MonoAssembly*>(this, method);
        return assembly ? assembly->GetName() : nullptr;
    }

    MonoField* RuntimeType::GetFieldObject(SystemString* name, BindingFlags bindingFlags) const
    {
        static auto* method = RequireMethod(RuntimeTypeClass(), "GetField", { "string", "System.Reflection.BindingFlags" });
        return InvokeInstance<MonoField*, SystemString*, int>(this, method, name, static_cast<int>(bindingFlags));
    }

    Cerydra::Il2Cpp::Field* RuntimeType::GetField(const char* name) const
    {
        auto* klass = GetMetaClass();
        while (klass) {
            if (auto* field = klass->GetField(name)) {
                return field;
            }
            if (!klass->address) {
                break;
            }

            auto* parent = il2cpp_class_get_parent(reinterpret_cast<Il2CppClass*>(klass->address));
            klass = Cerydra::Il2Cpp::FindClassByAddress(reinterpret_cast<uintptr_t>(parent));
        }

        throw std::runtime_error("找不到字段: " + std::string(name));
    }

    Cerydra::Il2Cpp::Type* RuntimeType::GetMetaType() const
    {
        const auto typePtr = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(this) + 16);
        return Cerydra::Il2Cpp::FindTypeByAddress(typePtr);
    }

    Cerydra::Il2Cpp::Class* RuntimeType::GetMetaClass() const
    {
        auto* type = GetMetaType();
        if (!type) {
            return nullptr;
        }
        if (type->klass) {
            return type->klass;
        }
        if (!type->address) {
            return nullptr;
        }

        auto* nativeClass = il2cpp_class_from_type(reinterpret_cast<const Il2CppType*>(type->address));
        return Cerydra::Il2Cpp::FindClassByAddress(reinterpret_cast<uintptr_t>(nativeClass));
    }

    Cerydra::Il2Cpp::Class* RuntimeFieldHandle::StaticClass()
    {
        static auto* klass = RequireClass("System.RuntimeFieldHandle");
        return klass;
    }
    uintptr_t RuntimeFieldHandle::GetValue() const
    {
        static auto* method = RequireMethod(StaticClass(), "get_Value", {});
        return InvokeInstance<uintptr_t>(this, method);
    }

    Cerydra::Il2Cpp::Class* MonoField::RuntimeFieldInfoClass()
    {
        static auto* klass = RequireClass("System.Reflection.RuntimeFieldInfo");
        return klass;
    }
    Cerydra::Il2Cpp::Class* MonoField::FieldInfoClass()
    {
        static auto* klass = RequireClass("System.Reflection.FieldInfo");
        return klass;
    }

    RuntimeType* MonoField::GetDeclaringType() const
    {
        static auto* method = RequireMethod(RuntimeFieldInfoClass(), "get_DeclaringType", {});
        return InvokeInstance<RuntimeType*>(this, method);
    }
    RuntimeType* MonoField::GetFieldType() const
    {
        static auto* method = RequireMethod(RuntimeFieldInfoClass(), "get_FieldType", {});
        return InvokeInstance<RuntimeType*>(this, method);
    }
    SystemString* MonoField::GetName() const
    {
        static auto* method = RequireMethod(RuntimeFieldInfoClass(), "get_Name", {});
        return InvokeInstance<SystemString*>(this, method);
    }
    RuntimeFieldHandle* MonoField::GetFieldHandle() const
    {
        static auto* method = RequireMethod(RuntimeFieldInfoClass(), "get_FieldHandle", {});
        return InvokeInstance<RuntimeFieldHandle*>(this, method);
    }
    Object* MonoField::GetRawConstantValue() const
    {
        static auto* method = RequireMethod(RuntimeFieldInfoClass(), "GetRawConstantValue", {});
        return InvokeInstance<Object*>(this, method);
    }
    Array<Object*>* MonoField::GetCustomAttributes(bool inherit) const
    {
        static auto* method = RequireMethod(RuntimeFieldInfoClass(), "GetCustomAttributes", { "bool" });
        return InvokeInstance<Array<Object*>*, bool>(this, method, inherit);
    }
    Object* MonoField::GetValue(Object* obj) const
    {
        static auto* method = RequireMethod(RuntimeFieldInfoClass(), "GetValue", { "object" });
        return InvokeInstance<Object*>(this, method, obj);
    }
    bool MonoField::IsLiteral() const
    {
        static auto* method = RequireMethod(FieldInfoClass(), "get_IsLiteral", {});
        return InvokeInstance<bool>(this, method);
    }
    int32_t MonoField::GetMetadataToken() const
    {
        static auto* method = RequireMethod(RuntimeFieldInfoClass(), "get_MetadataToken", {});
        return InvokeInstance<int32_t>(this, method);
    }
    void MonoField::SetValue(Object* obj, Object* value) const
    {
        static auto* method = RequireMethod(FieldInfoClass(), "SetValue", { "object", "object" });
        InvokeInstance<void>(this, method, obj, value);
    }

    Cerydra::Il2Cpp::Field* MonoField::GetMetaField() const
    {
        const auto fieldPtr = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(this) + 24);
        return Cerydra::Il2Cpp::FindFieldByAddress(fieldPtr);
    }

    Cerydra::Il2Cpp::Class* MonoProperty::RuntimePropertyInfoClass()
    {
        static auto* klass = RequireClass("System.Reflection.RuntimePropertyInfo");
        return klass;
    }
    Cerydra::Il2Cpp::Class* MonoProperty::PropertyInfoClass()
    {
        static auto* klass = RequireClass("System.Reflection.PropertyInfo");
        return klass;
    }

    RuntimeType* MonoProperty::GetDeclaringType() const
    {
        static auto* method = RequireMethod(RuntimePropertyInfoClass(), "get_DeclaringType", {});
        return InvokeInstance<RuntimeType*>(this, method);
    }
    RuntimeType* MonoProperty::GetPropertyType() const
    {
        static auto* method = RequireMethod(RuntimePropertyInfoClass(), "get_PropertyType", {});
        return InvokeInstance<RuntimeType*>(this, method);
    }
    SystemString* MonoProperty::GetName() const
    {
        static auto* method = RequireMethod(RuntimePropertyInfoClass(), "get_Name", {});
        return InvokeInstance<SystemString*>(this, method);
    }
    void MonoProperty::SetValue(Object* obj, Object* value) const
    {
        static auto* method = RequireMethod(PropertyInfoClass(), "SetValue", { "object", "object" });
        InvokeInstance<void>(this, method, obj, value);
    }
    Object* MonoProperty::GetValue(Object* obj) const
    {
        static auto* method = RequireMethod(PropertyInfoClass(), "GetValue", { "object" });
        return InvokeInstance<Object*>(this, method, obj);
    }
    Object* MonoProperty::GetValue(Object* obj, Array<Object*>* args) const
    {
        static auto* method = RequireMethod(PropertyInfoClass(), "GetValue", { "object", "System.Object[]" });
        return InvokeInstance<Object*>(this, method, obj, args);
    }

    Cerydra::Il2Cpp::Class* SystemObject::StaticClass()
    {
        static auto* klass = RequireClass("System.Object");
        return klass;
    }
    RuntimeType* SystemObject::GetType() const
    {
        static auto* method = RequireMethod(StaticClass(), "GetType", {});
        return InvokeInstance<RuntimeType*>(this, method);
    }

    Cerydra::Il2Cpp::Class* SystemInt32::StaticClass()
    {
        static auto* klass = RequireClass("System.Int32");
        return klass;
    }
    SystemString* SystemInt32::ToString() const
    {
        static auto* method = RequireMethod(StaticClass(), "ToString", {});
        return InvokeInstance<SystemString*>(this, method);
    }

    Cerydra::Il2Cpp::Class* SystemInt64::StaticClass()
    {
        static auto* klass = RequireClass("System.Int64");
        return klass;
    }
    SystemInt64* SystemInt64::FromAddress(uintptr_t ptr)
    {
        return reinterpret_cast<SystemInt64*>(ptr);
    }
    SystemString* SystemInt64::ToString() const
    {
        static auto* method = RequireMethod(StaticClass(), "ToString", {});
        return InvokeInstance<SystemString*>(this, method);
    }

    SystemString* SystemDynamic::ToString() const
    {
        return InvokeDynamic<SystemString*>(this, "ToString", {});
    }
}
