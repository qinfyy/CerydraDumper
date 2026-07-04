#pragma once

#include "CSharpInvoke.h"
#include "SystemString.h"
#include <string>

namespace Cerydra::CSharp
{
    class MonoAssembly;
    class MonoField;
    class MonoProperty;

    class RuntimeType : public Object
    {
    public:
        static Cerydra::Il2Cpp::Class* RuntimeTypeClass();
        static Cerydra::Il2Cpp::Class* TypeClass();

        static RuntimeType* FromClass(Cerydra::Il2Cpp::Class* klass);
        static RuntimeType* FromName(const std::string& name);

        RuntimeType* GetBaseType() const;
        bool IsGenericType() const;
        bool IsEnum() const;
        int GetArrayRank() const;
        RuntimeType* GetReflectedType() const;
        RuntimeType* GetElementType() const;
        SystemString* GetNamespace() const;
        SystemString* GetName() const;
        SystemString* GetFullName() const;
        Object* GetTypeHandle() const;
        Array<RuntimeType*>* GetGenericArguments() const;
        Array<MonoField*>* GetFields(int bindingFlags) const;
        Array<MonoProperty*>* GetProperties(int bindingFlags) const;
        bool IsByRef() const;
        bool IsArray() const;
        bool IsValueType() const;
        bool IsPointer() const;
        Object* GetType() const;
        MonoProperty* GetProperty(SystemString* name) const;
        bool IsInterface() const;
        Object* GetConstructor(Array<RuntimeType*>* types) const;
        bool IsAssignableFrom(RuntimeType* type) const;
        Array<RuntimeType*>* GetInterfaces() const;
        RuntimeType* GetDeclaringType() const;
        SystemString* GetAssemblyName() const;
        MonoField* GetFieldObject(SystemString* name, int bindingFlags) const;

        Cerydra::Il2Cpp::Field* GetField(const char* name) const;
        Cerydra::Il2Cpp::Type* GetMetaType() const;
        Cerydra::Il2Cpp::Class* GetMetaClass() const;

    private:
        static Cerydra::Il2Cpp::Method* GetBaseTypeMethod();
        static Cerydra::Il2Cpp::Method* IsGenericTypeMethod();
        static Cerydra::Il2Cpp::Method* IsEnumMethod();
        static Cerydra::Il2Cpp::Method* GetArrayRankMethod();
        static Cerydra::Il2Cpp::Method* GetReflectedTypeMethod();
        static Cerydra::Il2Cpp::Method* GetElementTypeMethod();
        static Cerydra::Il2Cpp::Method* GetNamespaceMethod();
        static Cerydra::Il2Cpp::Method* GetNameMethod();
        static Cerydra::Il2Cpp::Method* GetFullNameMethod();
        static Cerydra::Il2Cpp::Method* GetTypeHandleMethod();
        static Cerydra::Il2Cpp::Method* GetGenericArgumentsMethod();
        static Cerydra::Il2Cpp::Method* GetFieldsMethod();
        static Cerydra::Il2Cpp::Method* GetPropertiesMethod();
        static Cerydra::Il2Cpp::Method* IsByRefMethod();
        static Cerydra::Il2Cpp::Method* IsArrayMethod();
        static Cerydra::Il2Cpp::Method* IsValueTypeMethod();
        static Cerydra::Il2Cpp::Method* IsPointerMethod();
        static Cerydra::Il2Cpp::Method* GetTypeMethod();
        static Cerydra::Il2Cpp::Method* GetPropertyMethod();
        static Cerydra::Il2Cpp::Method* IsInterfaceMethod();
        static Cerydra::Il2Cpp::Method* GetConstructorMethod();
        static Cerydra::Il2Cpp::Method* IsAssignableFromMethod();
        static Cerydra::Il2Cpp::Method* GetInterfacesMethod();
        static Cerydra::Il2Cpp::Method* GetDeclaringTypeMethod();
        static Cerydra::Il2Cpp::Method* GetAssemblyMethod();
        static Cerydra::Il2Cpp::Method* GetFieldMethod();
    };

    class RuntimeFieldHandle : public Object
    {
    public:
        uintptr_t GetValue() const;

    private:
        static Cerydra::Il2Cpp::Class* StaticClass();
        static Cerydra::Il2Cpp::Method* GetValueMethod();
    };

    class MonoField : public Object
    {
    public:
        RuntimeType* GetDeclaringType() const;
        RuntimeType* GetFieldType() const;
        SystemString* GetName() const;
        RuntimeFieldHandle* GetFieldHandle() const;
        Object* GetRawConstantValue() const;
        Array<Object*>* GetCustomAttributes(bool inherit) const;
        Object* GetValue(Object* obj) const;
        bool IsLiteral() const;
        int32_t GetMetadataToken() const;
        void SetValue(Object* obj, Object* value) const;
        Cerydra::Il2Cpp::Field* GetMetaField() const;

    private:
        static Cerydra::Il2Cpp::Class* RuntimeFieldInfoClass();
        static Cerydra::Il2Cpp::Class* FieldInfoClass();
        static Cerydra::Il2Cpp::Method* GetDeclaringTypeMethod();
        static Cerydra::Il2Cpp::Method* GetFieldTypeMethod();
        static Cerydra::Il2Cpp::Method* GetNameMethod();
        static Cerydra::Il2Cpp::Method* GetFieldHandleMethod();
        static Cerydra::Il2Cpp::Method* GetRawConstantValueMethod();
        static Cerydra::Il2Cpp::Method* GetCustomAttributesMethod();
        static Cerydra::Il2Cpp::Method* GetValueMethod();
        static Cerydra::Il2Cpp::Method* IsLiteralMethod();
        static Cerydra::Il2Cpp::Method* GetMetadataTokenMethod();
        static Cerydra::Il2Cpp::Method* SetValueMethod();
    };

    class MonoProperty : public Object
    {
    public:
        RuntimeType* GetDeclaringType() const;
        RuntimeType* GetPropertyType() const;
        SystemString* GetName() const;
        void SetValue(Object* obj, Object* value) const;
        Object* GetValue(Object* obj) const;
        Object* GetValue(Object* obj, Array<Object*>* args) const;

    private:
        static Cerydra::Il2Cpp::Class* RuntimePropertyInfoClass();
        static Cerydra::Il2Cpp::Class* PropertyInfoClass();
        static Cerydra::Il2Cpp::Method* GetDeclaringTypeMethod();
        static Cerydra::Il2Cpp::Method* GetPropertyTypeMethod();
        static Cerydra::Il2Cpp::Method* GetNameMethod();
        static Cerydra::Il2Cpp::Method* SetValueMethod();
        static Cerydra::Il2Cpp::Method* GetValueMethod();
        static Cerydra::Il2Cpp::Method* GetValueWithArgsMethod();
    };

    class SystemObject : public Object
    {
    public:
        RuntimeType* GetType() const;

    private:
        static Cerydra::Il2Cpp::Class* StaticClass();
        static Cerydra::Il2Cpp::Method* GetTypeMethod();
    };

    class SystemInt32 : public Object
    {
    public:
        SystemString* ToString() const;

    private:
        static Cerydra::Il2Cpp::Class* StaticClass();
        static Cerydra::Il2Cpp::Method* ToStringMethod();
    };

    class SystemInt64 : public Object
    {
    public:
        static SystemInt64* FromAddress(uintptr_t ptr);
        SystemString* ToString() const;

    private:
        static Cerydra::Il2Cpp::Class* StaticClass();
        static Cerydra::Il2Cpp::Method* ToStringMethod();
    };

    class SystemDynamic : public Object
    {
    public:
        SystemString* ToString() const;
    };
}
