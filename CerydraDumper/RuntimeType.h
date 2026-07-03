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
        static Cerydra::IL2CPP::Class* RuntimeTypeClass();
        static Cerydra::IL2CPP::Class* TypeClass();

        static RuntimeType* FromClass(Cerydra::IL2CPP::Class* klass);
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

        Cerydra::IL2CPP::Field* GetField(const char* name) const;
        Cerydra::IL2CPP::Type* GetMetaType() const;
        Cerydra::IL2CPP::Class* GetMetaClass() const;

    private:
        static Cerydra::IL2CPP::Method* GetBaseTypeMethod();
        static Cerydra::IL2CPP::Method* IsGenericTypeMethod();
        static Cerydra::IL2CPP::Method* IsEnumMethod();
        static Cerydra::IL2CPP::Method* GetArrayRankMethod();
        static Cerydra::IL2CPP::Method* GetReflectedTypeMethod();
        static Cerydra::IL2CPP::Method* GetElementTypeMethod();
        static Cerydra::IL2CPP::Method* GetNamespaceMethod();
        static Cerydra::IL2CPP::Method* GetNameMethod();
        static Cerydra::IL2CPP::Method* GetFullNameMethod();
        static Cerydra::IL2CPP::Method* GetTypeHandleMethod();
        static Cerydra::IL2CPP::Method* GetGenericArgumentsMethod();
        static Cerydra::IL2CPP::Method* GetFieldsMethod();
        static Cerydra::IL2CPP::Method* GetPropertiesMethod();
        static Cerydra::IL2CPP::Method* IsByRefMethod();
        static Cerydra::IL2CPP::Method* IsArrayMethod();
        static Cerydra::IL2CPP::Method* IsValueTypeMethod();
        static Cerydra::IL2CPP::Method* IsPointerMethod();
        static Cerydra::IL2CPP::Method* GetTypeMethod();
        static Cerydra::IL2CPP::Method* GetPropertyMethod();
        static Cerydra::IL2CPP::Method* IsInterfaceMethod();
        static Cerydra::IL2CPP::Method* GetConstructorMethod();
        static Cerydra::IL2CPP::Method* IsAssignableFromMethod();
        static Cerydra::IL2CPP::Method* GetInterfacesMethod();
        static Cerydra::IL2CPP::Method* GetDeclaringTypeMethod();
        static Cerydra::IL2CPP::Method* GetAssemblyMethod();
        static Cerydra::IL2CPP::Method* GetFieldMethod();
    };

    class RuntimeFieldHandle : public Object
    {
    public:
        uintptr_t GetValue() const;

    private:
        static Cerydra::IL2CPP::Class* StaticClass();
        static Cerydra::IL2CPP::Method* GetValueMethod();
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
        Cerydra::IL2CPP::Field* GetMetaField() const;

    private:
        static Cerydra::IL2CPP::Class* RuntimeFieldInfoClass();
        static Cerydra::IL2CPP::Class* FieldInfoClass();
        static Cerydra::IL2CPP::Method* GetDeclaringTypeMethod();
        static Cerydra::IL2CPP::Method* GetFieldTypeMethod();
        static Cerydra::IL2CPP::Method* GetNameMethod();
        static Cerydra::IL2CPP::Method* GetFieldHandleMethod();
        static Cerydra::IL2CPP::Method* GetRawConstantValueMethod();
        static Cerydra::IL2CPP::Method* GetCustomAttributesMethod();
        static Cerydra::IL2CPP::Method* GetValueMethod();
        static Cerydra::IL2CPP::Method* IsLiteralMethod();
        static Cerydra::IL2CPP::Method* GetMetadataTokenMethod();
        static Cerydra::IL2CPP::Method* SetValueMethod();
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
        static Cerydra::IL2CPP::Class* RuntimePropertyInfoClass();
        static Cerydra::IL2CPP::Class* PropertyInfoClass();
        static Cerydra::IL2CPP::Method* GetDeclaringTypeMethod();
        static Cerydra::IL2CPP::Method* GetPropertyTypeMethod();
        static Cerydra::IL2CPP::Method* GetNameMethod();
        static Cerydra::IL2CPP::Method* SetValueMethod();
        static Cerydra::IL2CPP::Method* GetValueMethod();
        static Cerydra::IL2CPP::Method* GetValueWithArgsMethod();
    };

    class SystemObject : public Object
    {
    public:
        RuntimeType* GetType() const;

    private:
        static Cerydra::IL2CPP::Class* StaticClass();
        static Cerydra::IL2CPP::Method* GetTypeMethod();
    };

    class SystemInt32 : public Object
    {
    public:
        SystemString* ToString() const;

    private:
        static Cerydra::IL2CPP::Class* StaticClass();
        static Cerydra::IL2CPP::Method* ToStringMethod();
    };

    class SystemInt64 : public Object
    {
    public:
        static SystemInt64* FromAddress(uintptr_t ptr);
        SystemString* ToString() const;

    private:
        static Cerydra::IL2CPP::Class* StaticClass();
        static Cerydra::IL2CPP::Method* ToStringMethod();
    };

    class SystemDynamic : public Object
    {
    public:
        SystemString* ToString() const;
    };
}
