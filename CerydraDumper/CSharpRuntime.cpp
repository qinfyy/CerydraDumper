#include "pch.h"
#include "CSharpRuntime.h"
#include "CMonoAssembly.h"

CIl2CppField CRuntimeType::GetField(const char* name) const {
    try {
        // 先尝试当前类型
        std::unique_ptr<CMonoField> field_ptr = this->_GetField(name, 60); // BindingFlags = 60
        if (field_ptr && !field_ptr->is_null()) {
            return field_ptr->GetIl2CppField();
        }

        // 再尝试基类
        auto base_type = this->GetBaseType();
        if (!base_type.is_null()) {
            auto base_field_ptr = base_type._GetField(name, 60);
            if (base_field_ptr && !base_field_ptr->is_null()) {
                return base_field_ptr->GetIl2CppField();
            }
        }
    }
    catch (...) {
        // 如果当前类型失败，尝试基类
        try {
            auto base_type = this->GetBaseType();
            if (!base_type.is_null()) {
                auto base_field_ptr = base_type._GetField(name, 60);
                if (base_field_ptr && !base_field_ptr->is_null()) {
                    return base_field_ptr->GetIl2CppField();
                }
            }
        }
        catch (...) {
            // 失败直接抛异常
        }
    }

    throw std::runtime_error(
        "No such field " + std::string(name) + " in " + this->GetIl2CppType().name()
    );
}


//CSystemString CRuntimeType::GetAssemblyName() {
//    auto monoAssembly = nullptr
//
//    return monoAssembly.GetName();
//}
