#pragma once

#include "CSharpInvoke.h"
#include "SystemString.h"
#include <cstdint>
#include <string>

namespace Cerydra::CSharp
{
    class MonoAssembly;
    class MonoField;
    class MonoProperty;

    enum class BindingFlags : uint32_t
    {
        Default = 0,
        IgnoreCase = 1,
        DeclaredOnly = 2,
        Instance = 4,
        Static = 8,
        Public = 16,
        NonPublic = 32,
        FlattenHierarchy = 64,
        InvokeMethod = 256,
        CreateInstance = 512,
        GetField = 1024,
        SetField = 2048,
        GetProperty = 4096,
        SetProperty = 8192,
        PutDispProperty = 16384,
        PutRefDispProperty = 32768,
        ExactBinding = 65536,
        SuppressChangeType = 131072,
        OptionalParamBinding = 262144,
        IgnoreReturn = 16777216,
    };

    constexpr BindingFlags operator|(BindingFlags left, BindingFlags right)
    {
        return static_cast<BindingFlags>(static_cast<uint32_t>(left) | static_cast<uint32_t>(right));
    }

    constexpr BindingFlags operator&(BindingFlags left, BindingFlags right)
    {
        return static_cast<BindingFlags>(static_cast<uint32_t>(left) & static_cast<uint32_t>(right));
    }

    constexpr BindingFlags operator^(BindingFlags left, BindingFlags right)
    {
        return static_cast<BindingFlags>(static_cast<uint32_t>(left) ^ static_cast<uint32_t>(right));
    }

    constexpr BindingFlags operator~(BindingFlags value)
    {
        return static_cast<BindingFlags>(~static_cast<uint32_t>(value));
    }

    constexpr BindingFlags& operator|=(BindingFlags& left, BindingFlags right)
    {
        left = left | right;
        return left;
    }

    constexpr BindingFlags& operator&=(BindingFlags& left, BindingFlags right)
    {
        left = left & right;
        return left;
    }

    constexpr BindingFlags& operator^=(BindingFlags& left, BindingFlags right)
    {
        left = left ^ right;
        return left;
    }

    inline constexpr BindingFlags kAllMemberBindingFlags = BindingFlags::Instance | BindingFlags::Static | BindingFlags::Public | BindingFlags::NonPublic;
    inline constexpr BindingFlags kPublicStaticFlattenHierarchyBindingFlags = BindingFlags::Static | BindingFlags::Public | BindingFlags::FlattenHierarchy;

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
        Array<MonoField*>* GetFields(BindingFlags bindingFlags) const;
        Array<MonoProperty*>* GetProperties(BindingFlags bindingFlags) const;
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
        MonoField* GetFieldObject(SystemString* name, BindingFlags bindingFlags) const;

        Cerydra::Il2Cpp::Field* GetField(const char* name) const;
        Cerydra::Il2Cpp::Type* GetMetaType() const;
        Cerydra::Il2Cpp::Class* GetMetaClass() const;
    };

    class RuntimeFieldHandle : public Object
    {
    public:
        uintptr_t GetValue() const;

    private:
        static Cerydra::Il2Cpp::Class* StaticClass();
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
        static Cerydra::Il2Cpp::Class* MonoFieldClass();
        static Cerydra::Il2Cpp::Class* FieldInfoClass();
        static Cerydra::Il2Cpp::Class* MemberInfoClass();
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
    };

    class SystemObject : public Object
    {
    public:
        RuntimeType* GetType() const;

    private:
        static Cerydra::Il2Cpp::Class* StaticClass();
    };

    class SystemInt32 : public Object
    {
    public:
        SystemString* ToString() const;

    private:
        static Cerydra::Il2Cpp::Class* StaticClass();
    };

    class SystemInt64 : public Object
    {
    public:
        static SystemInt64* FromAddress(uintptr_t ptr);
        SystemString* ToString() const;

    private:
        static Cerydra::Il2Cpp::Class* StaticClass();
    };

    class SystemDynamic : public Object
    {
    public:
        SystemString* ToString() const;
    };
}
