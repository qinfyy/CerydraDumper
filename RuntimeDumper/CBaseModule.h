#pragma once
#include "Il2CppApiWrapper.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>
#include "CCSharpRuntime.h"

class CBaseModule : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("RPG.Client.BaseModule");

    //cs_method_custom!(pub add_packet_handlers_parent, "_AddPacketHandlers", &[], &String::from("RPG.Client.BaseModule"), (), (), self);
    //cs_method!(pub add_packet_handlers_child, "_AddPacketHandlers", &[], (), (), self);

    //???
    inline void AddPacketHandlersParent() const {
        CallIl2CppInstanceObjectMethod<void>(
            this->ptr,
            "RPG.Client.BaseModule",
            "_AddPacketHandlers",
            {} // 参数类型列表为空
        );
    }

    // 对应 Rust 的 cs_method!
    // pub add_packet_handlers_child
    inline void AddPacketHandlersChild() const {
        CallIl2CppInstanceObjectMethodDynamic<void>(
            ptr,
            "_AddPacketHandlers",
            {}
        );
    }
};

