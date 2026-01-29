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

    // 属性封装（对应 Rust cs_property!）
    inline CRuntimeType BaseType() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(
            this->ptr,
            "System.RuntimeType",
            "get_BaseType",
            {}
        );
    }

    inline bool IsGenericType() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.RuntimeType",
            "get_IsGenericType",
            {}
        );
    }

    inline bool IsEnum() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.RuntimeType",
            "get_IsEnum",
            {}
        );
    }

	// 必须是数组类型才能调用此函数
    //inline int ArrayRank() const {
    //    return InvokeIl2CppInstanceObjectMethod<int>(
    //        this->ptr,
    //        "System.Type",
    //        "GetArrayRank",
    //        {}
    //    );
    //}

    inline int ArrayRank() const {
        if (!this->ptr)
            throw std::runtime_error("Null reference exception! Method: GetArrayRank");

        // 获取对象对应的类
        auto klass = CIl2CppObject(this->ptr).get_class();
        if (klass.is_null())
            throw std::runtime_error("Cannot get class of instance!");

        // 查找方法
        auto method_info = klass.find_method("GetArrayRank", std::vector<std::string>{});
        if (!method_info)
            throw std::runtime_error("No such instance method GetArrayRank in " + klass.byval_arg().name());

        // 原生函数指针
        using Fn = int(__fastcall*)(uintptr_t);
        auto func = reinterpret_cast<Fn>(method_info.va());
        if (!func)
            throw std::runtime_error("Method VA is null! GetArrayRank");

        if (IsArray()) {
            return func(this->ptr);
        }
    }


    inline CRuntimeType ReflectedType() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(
            this->ptr,
            "System.RuntimeType",
            "get_ReflectedType",
            {}
        );
    }

    inline CRuntimeType ElementType() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(
            this->ptr,
            "System.RuntimeType",
            "GetElementType",
            {}
        );
    }

    // 返回 System.String 类型封装
    inline CSystemString Namespace() const {
        return CallIl2CppInstanceObjectMethod<CSystemString>(
            this->ptr,
            "System.RuntimeType",
            "get_Namespace",
            {}
        );
    }

    inline CSystemString Name() const {
        return CallIl2CppInstanceObjectMethod<CSystemString>(
            this->ptr,
            "System.RuntimeType",
            "get_Name",
            {}
        );
    }

    inline CSystemString FullName() const {
        return CallIl2CppInstanceObjectMethod<CSystemString>(
            this->ptr,
            "System.RuntimeType",
            "get_FullName",
            {}
        );
    }

    inline CIl2CppObject TypeHandle() const {
        return CallIl2CppInstanceObjectMethod<CIl2CppObject>(
            this->ptr,
            "System.RuntimeType",
            "get_TypeHandle",
            {}
        );
    }

    inline CIl2CppArray GenericArguments() const {
        return CallIl2CppInstanceObjectMethod<CIl2CppArray>(
            this->ptr,
            "System.RuntimeType",
            "GetGenericArguments",
            {}
        );
    }

    // 函数
    inline CIl2CppArray GetFields(int binding_flags) const {
        return CallIl2CppInstanceObjectMethod<CIl2CppArray>(
            this->ptr,
            "System.RuntimeType",
            "GetFields",
            { "System.Reflection.BindingFlags" },
            binding_flags
        );
    }

    inline CIl2CppArray GetProperties(int binding_flags) const {
        return CallIl2CppInstanceObjectMethod<CIl2CppArray>(
            this->ptr,
            "System.RuntimeType",
            "GetProperties",
            { "System.Reflection.BindingFlags" },
            binding_flags
        );
    }

    inline bool IsByRef() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.Type",
            "get_IsByRef",
            {}
        );
    }

    inline bool IsArray() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.Type",
            "get_IsArray",
            {}
        );
    }

    inline bool IsValueType() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.Type",
            "get_IsValueType",
            {}
        );
    }

    inline bool IsPointer() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.Type",
            "get_IsPointer",
            {}
        );
    }

    inline uintptr_t GetType() const {
        return CallIl2CppInstanceObjectMethod<uintptr_t>(
            this->ptr,
            "System.Type",
            "GetType",
            {}
        );
    }

    inline std::unique_ptr<CMonoProperty> GetProperty(CSystemString name) const {
        auto ptr = CallIl2CppInstanceObjectMethod<uintptr_t>(
            this->ptr,
            "System.Type",
            "GetProperty",
            { "string" },
            name
        );

        return std::make_unique<CMonoProperty>(ptr);
    }

    inline bool IsInterface() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.Type",
            "get_IsInterface",
            {}
        );
    }

    inline std::unique_ptr<CMonoField> _GetField(CSystemString name, int binding_flags) const {
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

    inline static CRuntimeType FromClass(CIl2CppClass klass) {
        uintptr_t type_ptr = CSystemType::GetTypeFromHandle((Il2CppType*)klass.byval_arg());
        return CRuntimeType(type_ptr);
    }

    inline static CRuntimeType FromName(const std::string& name) {
		auto* cClass = GetCachedClass(name);
        if (cClass == nullptr || cClass->is_null()) {
            throw std::runtime_error("No such class: " + name);
		}
        return FromClass(*cClass);
    }

    inline CIl2CppType GetIl2CppType() const {
        return CIl2CppType(*reinterpret_cast<uintptr_t*>(this->ptr + 16));
    }
};

class CRuntimeFieldHandle : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.RuntimeFieldHandle");

    inline uintptr_t Value() const {
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

    CS_CLASS("System.Reflection.MonoField");

    // 属性
    inline CRuntimeType DeclaringType() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(
            this->ptr,
            "System.Reflection.MonoField",
            "get_DeclaringType",
            {}
        );
    }

    inline CRuntimeType FieldType() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(
            this->ptr,
            "System.Reflection.MonoField",
            "get_FieldType",
            {}
        );
    }

    inline CSystemString Name() const {
        return CallIl2CppInstanceObjectMethod<CSystemString>(
            this->ptr,
            "System.Reflection.MonoField",
            "get_Name",
            {}
        );
    }

    inline CRuntimeFieldHandle FieldHandle() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeFieldHandle>(
            this->ptr,
            "System.Reflection.MonoField",
            "get_FieldHandle",
            {}
        );
    }

    inline uintptr_t RawConstantValue() const {
        return CallIl2CppInstanceObjectMethod<uintptr_t>(
            this->ptr,
            "System.Reflection.MonoField",
            "GetRawConstantValue",
            {}
        );
    }

    // 函数
    inline CIl2CppArray GetCustomAttributes(bool inherit) const {
        return CallIl2CppInstanceObjectMethod<CIl2CppArray>(
            this->ptr,
            "System.Reflection.MonoField",
            "GetCustomAttributes",
            { "bool" },
            inherit
        );
    }

    inline CIl2CppObject GetValue(uintptr_t obj) const {
        return CallIl2CppInstanceObjectMethod<CIl2CppObject>(
            this->ptr,
            "System.Reflection.MonoField",
            "GetValue",
            { "object" },
            obj
        );
    }

    inline bool IsLiteral() const {
        return CallIl2CppInstanceObjectMethod<bool>(
            this->ptr,
            "System.Reflection.FieldInfo",
            "get_IsLiteral",
            {}
        );
    }

    inline int32_t MetadataToken() const {
        return CallIl2CppInstanceObjectMethod<int32_t>(
            this->ptr,
            "System.Reflection.MemberInfo",
            "get_MetadataToken",
            {}
        );
    }

    inline void SetValue(uintptr_t obj, uintptr_t value) const {
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
    inline CIl2CppField GetIl2CppField() const {
		uintptr_t fieldPtr = *reinterpret_cast<uintptr_t*>(this->ptr + 24);
        return CIl2CppField(fieldPtr);
    }
};

class CMonoProperty : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.Reflection.MonoProperty");

    // 属性
    inline CRuntimeType DeclaringType() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(
            this->ptr,
            "System.Reflection.MonoProperty",
            "get_DeclaringType",
            {}
        );
    }

    inline CRuntimeType PropertyType() const {
        return CallIl2CppInstanceObjectMethod<CRuntimeType>(
            this->ptr,
            "System.Reflection.MonoProperty",
            "get_PropertyType",
            {}
        );
    }

    inline CSystemString Name() const {
        return CallIl2CppInstanceObjectMethod<CSystemString>(
            this->ptr,
            "System.Reflection.MonoProperty",
            "get_Name",
            {}
        );
    }

    // 函数
    inline void SetValue(uintptr_t obj, uintptr_t value) const {
        CallIl2CppInstanceObjectMethod<void>(
            this->ptr,
            "System.Reflection.PropertyInfo",
            "SetValue",
            { "object", "object" },
            obj,
            value
        );
    }

    inline CIl2CppObject GetValue(uintptr_t obj) const {
        return CallIl2CppInstanceObjectMethod<CIl2CppObject>(
            this->ptr,
            "System.Reflection.PropertyInfo",
            "GetValue",
            { "object" },
            obj
        );
    }

    inline uintptr_t GetValue(uintptr_t obj, uintptr_t args) const {
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

    // 对应 Rust 的 get_type
    inline CRuntimeType GetType() const {
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

    inline CSystemString ToString() const {
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

    inline CSystemString ToString() const {
        return CallIl2CppInstanceObjectMethod<CSystemString>(
            this->ptr,
            "System.Int64",
            "ToString",
            {}
        );
    }

    inline static CSystemInt64 FromPtr(uintptr_t ptr) {
        return CSystemInt64(ptr);
    }
};

// System.Dynamic
class CSystemDynamic : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;
    //CS_CLASS("System.Dynamic.DynamicObject");

    inline CSystemString ToString() const {
        if (this->ptr == 0) {
            throw std::runtime_error("SystemDynamic::ToString: null object");
        }

        auto obj = CIl2CppObject(this->ptr);
        auto obj_class = obj.get_class();
        if (obj_class.is_null()) {
            throw std::runtime_error("SystemDynamic::ToString: object class is null");
        }

        return CallIl2CppInstanceObjectMethod<CSystemString>(
            this->ptr,
            obj_class.byval_arg().name().c_str(), // 动态类名
            "ToString",
            {}
        );
    }
};
