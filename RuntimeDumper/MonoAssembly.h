#pragma once
#include "Il2CppApiWrapper.h"
#include "SystemString.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>

class CMonoAssembly : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CS_CLASS("System.Runtime.InteropServices.Marshal");

    inline CSystemString GetFullName() {
        auto klass = GetCachedClass("System.Reflection.Assembly"); if (klass->is_null()) {
            throw std::runtime_error(std::string("No such class: ") + "System.Reflection.Assembly");
        } auto method_info = klass->find_method("get_FullName", {}); if (method_info.is_null()) {
            throw std::runtime_error(std::string("No such method: ") + "get_FullName");
        } auto func = reinterpret_cast<uintptr_t(__fastcall*)(uintptr_t)> (method_info.va()); try {
            auto result = func(ptr);
            return CSystemString(result);
        }
        catch (...) {
            throw std::runtime_error(std::string("Exception in method: ") + "get_FullName");
        }
    }

    inline CIl2CppArray GetTypes(bool flags) {
        auto klass = GetCachedClass("System.Reflection.Assembly"); if (klass->is_null()) {
            throw std::runtime_error(std::string("No such class: ") + "System.Reflection.Assembly");
        } auto method_info = klass->find_method("GetTypes", { "bool" }); if (method_info.is_null()) {
            throw std::runtime_error(std::string("No such method: ") + "GetTypes");
        } auto func = reinterpret_cast<uintptr_t(__fastcall*)(uintptr_t, bool)> (method_info.va()); try {
            auto result = func(ptr, flags);
            return CIl2CppArray(result);
        }
        catch (...) {
            throw std::runtime_error(std::string("Exception in method: ") + "GetTypes");
        }
    }
};

