#pragma once

#include "CActivator.h"
#include "CSharpModel.h"
#include "CSystemString.h"
#include "CSystemType.h"
#include "Il2CppApiWrapper.h"
#include "PrintHelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class CMonoField;
class CMonoProperty;

class CRuntimeType : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    static Cerydra::IL2CPP::Class* RuntimeTypeClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.RuntimeType");
        return klass;
    }

    static Cerydra::IL2CPP::Class* TypeClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.Type");
        return klass;
    }

    static Cerydra::IL2CPP::Method* M_GetBaseType()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "get_BaseType", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_IsGenericType()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "get_IsGenericType", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_IsEnum()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "get_IsEnum", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetArrayRank()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(TypeClass(), "GetArrayRank", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetReflectedType()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "get_ReflectedType", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetElementType()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "GetElementType", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetNamespace()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "get_Namespace", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetName()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "get_Name", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetFullName()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "get_FullName", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetTypeHandle()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "get_TypeHandle", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetGenericArguments()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "GetGenericArguments", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetFields()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "GetFields", { "System.Reflection.BindingFlags" });
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetProperties()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "GetProperties", { "System.Reflection.BindingFlags" });
        return method;
    }

    static Cerydra::IL2CPP::Method* M_IsByRef()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(TypeClass(), "get_IsByRef", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_IsArray()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(TypeClass(), "get_IsArray", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_IsValueType()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(TypeClass(), "get_IsValueType", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_IsPointer()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(TypeClass(), "get_IsPointer", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetType()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(TypeClass(), "GetType", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetProperty()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(TypeClass(), "GetProperty", { "string" });
        return method;
    }

    static Cerydra::IL2CPP::Method* M_IsInterface()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(TypeClass(), "get_IsInterface", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetConstructor()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(TypeClass(), "GetConstructor", { "System.Type[]" });
        return method;
    }

    static Cerydra::IL2CPP::Method* M_IsAssignableFrom()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "IsAssignableFrom", { "System.Type" });
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetInterfaces()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "GetInterfaces", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetDeclaringType()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "get_DeclaringType", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetAssembly()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "get_Assembly", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetField()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeTypeClass(), "GetField", { "string", "System.Reflection.BindingFlags" });
        return method;
    }

    CRuntimeType GetBaseType() const
    {
        return Cerydra::CSharp::InvokeInstance<CRuntimeType>(ptr, M_GetBaseType());
    }

    bool IsGenericType() const
    {
        return Cerydra::CSharp::InvokeInstance<bool>(ptr, M_IsGenericType());
    }

    bool IsEnum() const
    {
        return Cerydra::CSharp::InvokeInstance<bool>(ptr, M_IsEnum());
    }

    int GetArrayRank() const
    {
        return Cerydra::CSharp::InvokeInstance<int>(ptr, M_GetArrayRank());
    }

    CRuntimeType GetReflectedType() const
    {
        return Cerydra::CSharp::InvokeInstance<CRuntimeType>(ptr, M_GetReflectedType());
    }

    CRuntimeType GetElementType() const
    {
        return Cerydra::CSharp::InvokeInstance<CRuntimeType>(ptr, M_GetElementType());
    }

    CSystemString GetNamespace() const
    {
        return Cerydra::CSharp::InvokeInstance<CSystemString>(ptr, M_GetNamespace());
    }

    CSystemString GetName() const
    {
        return Cerydra::CSharp::InvokeInstance<CSystemString>(ptr, M_GetName());
    }

    CSystemString GetFullName() const
    {
        return Cerydra::CSharp::InvokeInstance<CSystemString>(ptr, M_GetFullName());
    }

    CIl2CppObject GetTypeHandle() const
    {
        return Cerydra::CSharp::InvokeInstance<CIl2CppObject>(ptr, M_GetTypeHandle());
    }

    CIl2CppArray GetGenericArguments() const
    {
        return Cerydra::CSharp::InvokeInstance<CIl2CppArray>(ptr, M_GetGenericArguments());
    }

    CIl2CppArray GetFields(int bindingFlags) const
    {
        return Cerydra::CSharp::InvokeInstance<CIl2CppArray, int>(ptr, M_GetFields(), bindingFlags);
    }

    CIl2CppArray GetProperties(int bindingFlags) const
    {
        return Cerydra::CSharp::InvokeInstance<CIl2CppArray, int>(ptr, M_GetProperties(), bindingFlags);
    }

    bool IsByRef() const
    {
        return Cerydra::CSharp::InvokeInstance<bool>(ptr, M_IsByRef());
    }

    bool IsArray() const
    {
        return Cerydra::CSharp::InvokeInstance<bool>(ptr, M_IsArray());
    }

    bool IsValueType() const
    {
        return Cerydra::CSharp::InvokeInstance<bool>(ptr, M_IsValueType());
    }

    bool IsPointer() const
    {
        return Cerydra::CSharp::InvokeInstance<bool>(ptr, M_IsPointer());
    }

    uintptr_t GetType() const
    {
        return Cerydra::CSharp::InvokeInstance<uintptr_t>(ptr, M_GetType());
    }

    std::unique_ptr<CMonoProperty> GetProperty(CSystemString name) const
    {
        auto propertyPtr = Cerydra::CSharp::InvokeInstance<uintptr_t>(ptr, M_GetProperty(), name);
        return std::make_unique<CMonoProperty>(propertyPtr);
    }

    bool IsInterface() const
    {
        return Cerydra::CSharp::InvokeInstance<bool>(ptr, M_IsInterface());
    }

    uintptr_t GetConstructor(uintptr_t types)
    {
        return Cerydra::CSharp::InvokeInstance<uintptr_t>(ptr, M_GetConstructor(), types);
    }

    bool IsAssignableFrom(uintptr_t type)
    {
        return Cerydra::CSharp::InvokeInstance<bool>(ptr, M_IsAssignableFrom(), type);
    }

    CIl2CppArray GetInterfaces()
    {
        return Cerydra::CSharp::InvokeInstance<CIl2CppArray>(ptr, M_GetInterfaces());
    }

    CRuntimeType GetDeclaringType()
    {
        return Cerydra::CSharp::InvokeInstance<CRuntimeType>(ptr, M_GetDeclaringType());
    }

    CSystemString GetAssemblyName();

    std::unique_ptr<CMonoField> _GetField(CSystemString name, int bindingFlags) const
    {
        auto fieldPtr = Cerydra::CSharp::InvokeInstance<uintptr_t>(ptr, M_GetField(), name, bindingFlags);
        return std::make_unique<CMonoField>(fieldPtr);
    }

    CIl2CppField GetField(const char* name) const;

    static CRuntimeType FromClass(Cerydra::IL2CPP::Class* klass)
    {
        return CRuntimeType(klass ? reinterpret_cast<uintptr_t>(klass->GetTypeObject()) : 0);
    }

    static CRuntimeType FromClass(CIl2CppClass klass)
    {
        if (auto* metaClass = GetCachedClassMeta(klass.raw_ptr())) {
            return FromClass(metaClass);
        }

        auto typePtr = CSystemType::GetTypeFromHandle(reinterpret_cast<Il2CppType*>(klass.byval_arg().raw_ptr()));
        return CRuntimeType(typePtr);
    }

    static CRuntimeType FromName(const std::string& name)
    {
        auto* klass = GetCachedClassMeta(name);
        if (!klass) {
            throw std::runtime_error("找不到类: " + name);
        }
        return FromClass(klass);
    }

    CIl2CppType GetIl2CppType() const
    {
        if (is_null()) {
            return CIl2CppType(0);
        }

        return CIl2CppType(*reinterpret_cast<uintptr_t*>(ptr + 16));
    }

    Cerydra::IL2CPP::Type* GetMetaType() const
    {
        if (is_null()) {
            return nullptr;
        }

        auto typePtr = *reinterpret_cast<uintptr_t*>(ptr + 16);
        return Il2CppRuntimeCache::GetTypeByAddress(typePtr);
    }

    Cerydra::IL2CPP::Class* GetMetaClass() const
    {
        auto* type = GetMetaType();
        return type ? type->klass : nullptr;
    }
};

class CRuntimeFieldHandle : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    static Cerydra::IL2CPP::Class* StaticClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.RuntimeFieldHandle");
        return klass;
    }

    static Cerydra::IL2CPP::Method* M_GetValue()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(StaticClass(), "get_Value", {});
        return method;
    }

    uintptr_t GetValue() const
    {
        return Cerydra::CSharp::InvokeInstance<uintptr_t>(ptr, M_GetValue());
    }
};

class CMonoField : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    static Cerydra::IL2CPP::Class* RuntimeFieldInfoClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.Reflection.RuntimeFieldInfo");
        return klass;
    }

    static Cerydra::IL2CPP::Class* FieldInfoClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.Reflection.FieldInfo");
        return klass;
    }

    static Cerydra::IL2CPP::Method* M_GetDeclaringType()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeFieldInfoClass(), "get_DeclaringType", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetFieldType()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeFieldInfoClass(), "get_FieldType", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetName()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeFieldInfoClass(), "get_Name", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetFieldHandle()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeFieldInfoClass(), "get_FieldHandle", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetRawConstantValue()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeFieldInfoClass(), "GetRawConstantValue", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetCustomAttributes()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeFieldInfoClass(), "GetCustomAttributes", { "bool" });
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetValue()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeFieldInfoClass(), "GetValue", { "object" });
        return method;
    }

    static Cerydra::IL2CPP::Method* M_IsLiteral()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(FieldInfoClass(), "get_IsLiteral", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetMetadataToken()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimeFieldInfoClass(), "get_MetadataToken", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_SetValue()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(FieldInfoClass(), "SetValue", { "object", "object" });
        return method;
    }

    CRuntimeType GetDeclaringType() const
    {
        return Cerydra::CSharp::InvokeInstance<CRuntimeType>(ptr, M_GetDeclaringType());
    }

    CRuntimeType GetFieldType() const
    {
        return Cerydra::CSharp::InvokeInstance<CRuntimeType>(ptr, M_GetFieldType());
    }

    CSystemString GetName() const
    {
        return Cerydra::CSharp::InvokeInstance<CSystemString>(ptr, M_GetName());
    }

    CRuntimeFieldHandle GetFieldHandle() const
    {
        return Cerydra::CSharp::InvokeInstance<CRuntimeFieldHandle>(ptr, M_GetFieldHandle());
    }

    uintptr_t GetRawConstantValue() const
    {
        return Cerydra::CSharp::InvokeInstance<uintptr_t>(ptr, M_GetRawConstantValue());
    }

    CIl2CppArray GetCustomAttributes(bool inherit) const
    {
        return Cerydra::CSharp::InvokeInstance<CIl2CppArray, bool>(ptr, M_GetCustomAttributes(), inherit);
    }

    CIl2CppObject GetValue(uintptr_t obj) const
    {
        return Cerydra::CSharp::InvokeInstance<CIl2CppObject>(ptr, M_GetValue(), obj);
    }

    bool IsLiteral() const
    {
        return Cerydra::CSharp::InvokeInstance<bool>(ptr, M_IsLiteral());
    }

    int32_t GetMetadataToken() const
    {
        return Cerydra::CSharp::InvokeInstance<int32_t>(ptr, M_GetMetadataToken());
    }

    void SetValue(uintptr_t obj, uintptr_t value) const
    {
        Cerydra::CSharp::InvokeInstance<void>(ptr, M_SetValue(), obj, value);
    }

    CIl2CppField GetIl2CppField() const
    {
        auto fieldPtr = *reinterpret_cast<uintptr_t*>(ptr + 24);
        return CIl2CppField(fieldPtr);
    }
};

class CMonoProperty : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    static Cerydra::IL2CPP::Class* RuntimePropertyInfoClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.Reflection.RuntimePropertyInfo");
        return klass;
    }

    static Cerydra::IL2CPP::Class* PropertyInfoClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.Reflection.PropertyInfo");
        return klass;
    }

    static Cerydra::IL2CPP::Method* M_GetDeclaringType()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimePropertyInfoClass(), "get_DeclaringType", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetPropertyType()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimePropertyInfoClass(), "get_PropertyType", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetName()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(RuntimePropertyInfoClass(), "get_Name", {});
        return method;
    }

    static Cerydra::IL2CPP::Method* M_SetValue()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(PropertyInfoClass(), "SetValue", { "object", "object" });
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetValue()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(PropertyInfoClass(), "GetValue", { "object" });
        return method;
    }

    static Cerydra::IL2CPP::Method* M_GetValueWithArgs()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(PropertyInfoClass(), "GetValue", { "object", "System.Object[]" });
        return method;
    }

    CRuntimeType GetDeclaringType() const
    {
        return Cerydra::CSharp::InvokeInstance<CRuntimeType>(ptr, M_GetDeclaringType());
    }

    CRuntimeType GetPropertyType() const
    {
        return Cerydra::CSharp::InvokeInstance<CRuntimeType>(ptr, M_GetPropertyType());
    }

    CSystemString GetName() const
    {
        return Cerydra::CSharp::InvokeInstance<CSystemString>(ptr, M_GetName());
    }

    void SetValue(uintptr_t obj, uintptr_t value) const
    {
        Cerydra::CSharp::InvokeInstance<void>(ptr, M_SetValue(), obj, value);
    }

    CIl2CppObject GetValue(uintptr_t obj) const
    {
        return Cerydra::CSharp::InvokeInstance<CIl2CppObject>(ptr, M_GetValue(), obj);
    }

    uintptr_t GetValue(uintptr_t obj, uintptr_t args) const
    {
        return Cerydra::CSharp::InvokeInstance<uintptr_t>(ptr, M_GetValueWithArgs(), obj, args);
    }
};

class CSystemObject : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    static Cerydra::IL2CPP::Class* StaticClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.Object");
        return klass;
    }

    static Cerydra::IL2CPP::Method* M_GetType()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(StaticClass(), "GetType", {});
        return method;
    }

    CRuntimeType GetType() const
    {
        return Cerydra::CSharp::InvokeInstance<CRuntimeType>(ptr, M_GetType());
    }
};

class CSystemInt32 : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    static Cerydra::IL2CPP::Class* StaticClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.Int32");
        return klass;
    }

    static Cerydra::IL2CPP::Method* M_ToString()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(StaticClass(), "ToString", {});
        return method;
    }

    CSystemString ToString() const
    {
        return Cerydra::CSharp::InvokeInstance<CSystemString>(ptr, M_ToString());
    }
};

class CSystemInt64 : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    static Cerydra::IL2CPP::Class* StaticClass()
    {
        static auto* klass = Cerydra::CSharp::RequireClass("System.Int64");
        return klass;
    }

    static Cerydra::IL2CPP::Method* M_ToString()
    {
        static auto* method = Cerydra::CSharp::RequireMethod(StaticClass(), "ToString", {});
        return method;
    }

    CSystemString ToString() const
    {
        return Cerydra::CSharp::InvokeInstance<CSystemString>(ptr, M_ToString());
    }

    static CSystemInt64 FromPtr(uintptr_t ptr)
    {
        return CSystemInt64(ptr);
    }
};

class CSystemDynamic : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CSystemString ToString() const
    {
        if (!ptr) {
            throw std::runtime_error("SystemDynamic::ToString: 对象为空");
        }

        return Cerydra::CSharp::InvokeDynamic<CSystemString>(ptr, "ToString", {});
    }
};
