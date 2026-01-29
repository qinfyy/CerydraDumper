#pragma once
#include "Il2CppApiWrapper.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>
#include "CSharpRuntime.h"

class CBaseModule : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("RPG.Client.BaseModule");

    inline void AddPacketHandlersParent() const {
        CallIl2CppInstanceObjectMethod<void>(
            this->ptr,
            "RPG.Client.BaseModule",
            "_AddPacketHandlers",
            {} // 参数类型列表为空
        );
    }

    inline void AddPacketHandlersChild() const {
        CallIl2CppInstanceObjectMethodDynamic<void>(
            ptr,
            "_AddPacketHandlers",
            {}
        );
    }
};

