#pragma once
#include "Il2CppApiWrapper.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>

class CSystemType : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.Type");

    //CS_METHOD_STATIC_AUTO_CTOR(GetTypeFromHandle, "GetTypeFromHandle", FN_ARGS("System.RuntimeTypeHandle"), CSystemType, (Il2CppType* ty), (ty));

    //cs_method!(pub get_type_from_handle, "GetTypeFromHandle", &["System.RuntimeTypeHandle"], Self, (ty: crate::il2cpp::api::Il2CppType));
    CS_METHOD_STATIC_AUTO_CTOR(
        GetTypeFromHandle,           // fn_name：C++ 调用名
        "GetTypeFromHandle",         // method_name：IL2CPP 中的原方法名
        { "System.RuntimeTypeHandle" }, // fnArgs：参数类型列表
        CSystemType,                 // ret_type：返回值封装类型,对应 Self
        (Il2CppType* ty),              // args_decl：函数参数声明
        (ty)                         // args_name：调用时传入的参数
    );
};

