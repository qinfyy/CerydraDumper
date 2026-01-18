#pragma once
#include "Il2CppApiWrapper.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>

class CSystemString : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.Runtime.InteropServices.Marshal");

    // 对应 Rust 的 cs_method! ptr_to_string_ansi
    CS_METHOD_STATIC_AUTO_CTOR(PtrToStringAnsi, "PtrToStringAnsi", {"System.IntPtr"}, CSystemString, (const char* charPtr), (charPtr));

    // 从 Il2CppString* 转 std::string
    std::string AsString() const;
};
