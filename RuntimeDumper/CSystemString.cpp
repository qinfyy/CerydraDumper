#include "pch.h"
#include "CSystemString.h"
#include <codecvt>
#include <locale>
#include <cstring>
#include "Util.h"

std::string CSystemString::AsString() const
{
    if (is_null()) return "";

    //uint32_t str_length = *(uint32_t*)(ptr + 16);
    //const wchar_t* str_ptr = (const wchar_t*)(ptr + 20);

    //std::wstring u16str(str_ptr, str_length);

    return Il2CppToUtf8String((Il2CppString*)ptr);
}