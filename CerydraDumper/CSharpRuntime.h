#pragma once
#include "Il2CppApiWrapper.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>
#include "CSystemType.h"
#include "CSystemString.h"
#include "PrintHelper.h"
#include "CActivator.h"

class CMonoField;
class CMonoProperty;

class CRuntimeType : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.RuntimeType");

    // 属性
    CRuntimeType GetBaseType() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(
            this->ptr,
            "System.RuntimeType",
            "get_BaseType",
            {}
        );
    }

    bool IsGenericType() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.RuntimeType",
            "get_IsGenericType",
            {}
        );
    }

    bool IsEnum() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.RuntimeType",
            "get_IsEnum",
            {}
        );
    }

	// 必须是数组类型才能调用此函数
    int GetArrayRank() const {
        return CallIl2CppInstanceObjectMethod<int>(
            this->ptr,
            "System.Type",
            "GetArrayRank",
            {}
        );
    }

    CRuntimeType GetReflectedType() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(
            this->ptr,
            "System.RuntimeType",
            "get_ReflectedType",
            {}
        );
    }

    CRuntimeType GetElementType() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(
            this->ptr,
            "System.RuntimeType",
            "GetElementType",
            {}
        );
    }

    CSystemString GetNamespace() const {
        return CallIl2CppInstanceObjectMethod<CSystemString>(
            this->ptr,
            "System.RuntimeType",
            "get_Namespace",
            {}
        );
    }

    CSystemString GetName() const {
        return CallIl2CppInstanceObjectMethod<CSystemString>(
            this->ptr,
            "System.RuntimeType",
            "get_Name",
            {}
        );
    }

    CSystemString GetFullName() const {
        return CallIl2CppInstanceObjectMethod<CSystemString>(
            this->ptr,
            "System.RuntimeType",
            "get_FullName",
            {}
        );
    }

    CIl2CppObject GetTypeHandle() const {
        return CallIl2CppInstanceObjectMethod<CIl2CppObject>(
            this->ptr,
            "System.RuntimeType",
            "get_TypeHandle",
            {}
        );
    }

    CIl2CppArray GetGenericArguments() const {
        return CallIl2CppInstanceObjectMethod<CIl2CppArray>(
            this->ptr,
            "System.RuntimeType",
            "GetGenericArguments",
            {}
        );
    }

    // 函数
    CIl2CppArray GetFields(int binding_flags) const {
        return CallIl2CppInstanceObjectMethod<CIl2CppArray>(
            this->ptr,
            "System.RuntimeType",
            "GetFields",
            { "System.Reflection.BindingFlags" },
            binding_flags
        );
    }

    CIl2CppArray GetProperties(int binding_flags) const {
        return CallIl2CppInstanceObjectMethod<CIl2CppArray>(
            this->ptr,
            "System.RuntimeType",
            "GetProperties",
            { "System.Reflection.BindingFlags" },
            binding_flags
        );
    }

    bool IsByRef() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.Type",
            "get_IsByRef",
            {}
        );
    }

    bool IsArray() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.Type",
            "get_IsArray",
            {}
        );
    }

    bool IsValueType() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.Type",
            "get_IsValueType",
            {}
        );
    }

    bool IsPointer() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.Type",
            "get_IsPointer",
            {}
        );
    }

    uintptr_t GetType() const {
        return CallIl2CppInstanceObjectMethod<uintptr_t>(
            this->ptr,
            "System.Type",
            "GetType",
            {}
        );
    }

    std::unique_ptr<CMonoProperty> GetProperty(CSystemString name) const {
        auto ptr = CallIl2CppInstanceObjectMethod<uintptr_t>(
            this->ptr,
            "System.Type",
            "GetProperty",
            { "string" },
            name
        );

        return std::make_unique<CMonoProperty>(ptr);
    }

    bool IsInterface() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.Type",
            "get_IsInterface",
            {}
        );
    }

    uintptr_t GetConstructor(uintptr_t Types) {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.Type",
            "GetConstructor",
            {"System.Type[]"},
            Types
        );
    }

    std::unique_ptr<CMonoField> _GetField(CSystemString name, int binding_flags) const {
        auto ptr = CallIl2CppInstanceObjectMethod<uintptr_t>(
            this->ptr,
            "System.RuntimeType",
            "GetField",
            { "string", "System.Reflection.BindingFlags" },
            name,
            binding_flags
        );

        return std::make_unique<CMonoField>(ptr);
    }

    CIl2CppField GetField(const char* name) const;

    static CRuntimeType FromClass(CIl2CppClass klass) {
        uintptr_t type_ptr = CSystemType::GetTypeFromHandle((Il2CppType*)klass.byval_arg());
        return CRuntimeType(type_ptr);
    }

    static CRuntimeType FromName(const std::string& name) {
		auto* cClass = GetCachedClass(name);
        if (cClass == nullptr || cClass->is_null()) {
            throw std::runtime_error("No such class: " + name);
		}
        return FromClass(*cClass);
    }

    CIl2CppType GetIl2CppType() const {
        return CIl2CppType(*reinterpret_cast<uintptr_t*>(this->ptr + 16));
    }
};

class CRuntimeFieldHandle : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.RuntimeFieldHandle");

    uintptr_t GetValue() const {
        return CallIl2CppInstanceObjectMethod<uintptr_t>(
            this->ptr,
            "System.RuntimeFieldHandle",
            "get_Value",
            {}
        );
    }
};

class CMonoField : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.Reflection.RuntimeFieldInfo");

    // 属性
    CRuntimeType GetDeclaringType() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(
            this->ptr,
            "System.Reflection.RuntimeFieldInfo",
            "get_DeclaringType",
            {}
        );
    }

    CRuntimeType GetFieldType() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(
            this->ptr,
            "System.Reflection.RuntimeFieldInfo",
            "get_FieldType",
            {}
        );
    }

    CSystemString GetName() const {
        return CallIl2CppInstanceObjectMethod<CSystemString>(
            this->ptr,
            "System.Reflection.RuntimeFieldInfo",
            "get_Name",
            {}
        );
    }

    CRuntimeFieldHandle GetFieldHandle() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeFieldHandle>(
            this->ptr,
            "System.Reflection.RuntimeFieldInfo",
            "get_FieldHandle",
            {}
        );
    }

    uintptr_t GetRawConstantValue() const {
        return CallIl2CppInstanceObjectMethod<uintptr_t>(
            this->ptr,
            "System.Reflection.RuntimeFieldInfo",
            "GetRawConstantValue",
            {}
        );
    }

    // 函数
    CIl2CppArray GetCustomAttributes(bool inherit) const {
        return CallIl2CppInstanceObjectMethod<CIl2CppArray>(
            this->ptr,
            "System.Reflection.RuntimeFieldInfo",
            "GetCustomAttributes",
            { "bool" },
            inherit
        );
    }

    CIl2CppObject GetValue(uintptr_t obj) const {
        return CallIl2CppInstanceObjectMethod<CIl2CppObject>(
            this->ptr,
            "System.Reflection.RuntimeFieldInfo",
            "GetValue",
            { "object" },
            obj
        );
    }

    bool IsLiteral() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.Reflection.FieldInfo",
            "get_IsLiteral",
            {}
        );
    }

    int32_t GetMetadataToken() const {
        return CallIl2CppInstanceObjectMethod<int32_t>(
            this->ptr,
            "System.Reflection.RuntimeFieldInfo",
            "get_MetadataToken",
            {}
        );
    }

    void SetValue(uintptr_t obj, uintptr_t value) const {
        CallIl2CppInstanceObjectMethod<void>(
            this->ptr,
            "System.Reflection.FieldInfo",
            "SetValue",
            { "object", "object" },
            obj,
            value
        );
    }

    // 访问 IL2CPP 内部字段
    CIl2CppField GetIl2CppField() const {
		uintptr_t fieldPtr = *reinterpret_cast<uintptr_t*>(this->ptr + 24);
        return CIl2CppField(fieldPtr);
    }
};

class CMonoProperty : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.Reflection.RuntimePropertyInfo");

    // 属性
    CRuntimeType GetDeclaringType() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(
            this->ptr,
            "System.Reflection.RuntimePropertyInfo",
            "get_DeclaringType",
            {}
        );
    }

    CRuntimeType GetPropertyType() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(
            this->ptr,
            "System.Reflection.RuntimePropertyInfo",
            "get_PropertyType",
            {}
        );
    }

    CSystemString GetName() const {
        return CallIl2CppInstanceObjectMethod<CSystemString>(
            this->ptr,
            "System.Reflection.RuntimePropertyInfo",
            "get_Name",
            {}
        );
    }

    // 函数
    void SetValue(uintptr_t obj, uintptr_t value) const {
        CallIl2CppInstanceObjectMethod<void>(
            this->ptr,
            "System.Reflection.PropertyInfo",
            "SetValue",
            { "object", "object" },
            obj,
            value
        );
    }

    CIl2CppObject GetValue(uintptr_t obj) const {
        return CallIl2CppInstanceObjectMethod<CIl2CppObject>(
            this->ptr,
            "System.Reflection.PropertyInfo",
            "GetValue",
            { "object" },
            obj
        );
    }

    uintptr_t GetValue(uintptr_t obj, uintptr_t args) const {
        return CallIl2CppInstanceObjectMethod<uintptr_t>(
            this->ptr,
            "System.Reflection.PropertyInfo",
            "GetValue",
            { "object", "System.Object[]" },
            obj,
            args
        );
    }
};

class CSystemObject : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.Object");

    CRuntimeType GetType() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(
            this->ptr,
            "System.Object",
            "GetType",
            {}
        );
    }
};

// System.Int32
class CSystemInt32 : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.Int32");

    CSystemString ToString() const {
        return CallIl2CppInstanceObjectMethod<CSystemString>(
            this->ptr,
            "System.Int32",
            "ToString",
            {}
        );
    }
};

// System.Int64
class CSystemInt64 : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.Int64");

    CSystemString ToString() const {
        return CallIl2CppInstanceObjectMethod<CSystemString>(
            this->ptr,
            "System.Int64",
            "ToString",
            {}
        );
    }

    static CSystemInt64 FromPtr(uintptr_t ptr) {
        return CSystemInt64(ptr);
    }
};

// System.Dynamic
class CSystemDynamic : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;
    //CS_CLASS("System.Dynamic.DynamicObject");

    CSystemString ToString() const {
        if (this->ptr == 0) {
            throw std::runtime_error("SystemDynamic::ToString: null object");
        }

        auto obj = CIl2CppObject(this->ptr);
        auto obj_class = obj.get_class();
        if (obj_class.is_null()) {
            throw std::runtime_error("SystemDynamic::ToString: object class is null");
        }

        return CallIl2CppInstanceObjectMethodDynamic<CSystemString>(
            this->ptr,
            "ToString",
            {}
        );
    }
};
