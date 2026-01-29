#include "pch.h"
#include "CSystemString.h"
#include <codecvt>
#include <locale>
#include <cstring>
#include "Util.h"

std::string CSystemString::AsString() const
{
    if (is_null()) return "";

    return Il2CppToUtf8String((Il2CppString*)ptr);
}