#pragma once
#include "Il2CppApiWrapper.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>

class CAppDomain : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.AppDomain");

    inline CIl2CppArray GetAssemblies() {
        auto klass = GetCachedClass("System.AppDomain"); if (klass->is_null()) {
            throw std::runtime_error(std::string("No such class: ") + "System.AppDomain");
        } auto method_info = klass->find_method("GetAssemblies", {}); if (method_info.is_null()) {
            throw std::runtime_error(std::string("No such method: ") + "GetAssemblies");
        } auto func = reinterpret_cast<uintptr_t(__fastcall*)(uintptr_t)> (method_info.va()); try {
            auto result = func(ptr);
            return CIl2CppArray(result);
        }
        catch (...) {
            throw std::runtime_error(std::string("Exception in method: ") + "GetAssemblies");
        }
    }

    CS_METHOD_STATIC_AUTO_CTOR(GetCurrentDomain, "get_CurrentDomain", {}, CAppDomain, (), ());
};
