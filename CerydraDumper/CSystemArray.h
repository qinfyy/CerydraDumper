#pragma once
#include "Il2CppApiWrapper.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>

class CSystemArray : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.Array");

    inline int GetLength() {
        return CallIl2CppInstanceObjectMethod<int>(
            this->ptr,                // instance 指针
            "System.Array",           // 类名
            "get_Length",             // 方法名
            {}                        // 参数类型，这里没有参数
        );
    }

    inline size_t GetValue(int index) {
        return CallIl2CppInstanceObjectMethod<size_t>(
            this->ptr,                // instance 指针
            "System.Array",           // 类名
            "GetValue",               // 方法名
            { "int" },                // 参数类型
            index                     // 实参
        );
    }
};

