//#pragma once
//#include <cstdint>
//#include <vector>
//#include <string>
//#include "Il2CppFunctions.h" // 假设里面有 il2cpp_domain_get_assemblies, il2cpp_domain_assembly_open
//#include "Util.h" // 你的字符串辅助函数
//
//class Il2CppAssembly; // 前置声明
//
//class Il2CppDomain {
//public:
//    // 构造函数
//    explicit Il2CppDomain(uintptr_t ptr = 0) : _ptr(ptr) {}
//
//    // 判断指针是否有效
//    explicit operator bool() const { return _ptr != 0; }
//
//    // 原始指针
//    uintptr_t raw() const { return _ptr; }
//
//    // 获取所有程序集
//    std::vector<Il2CppAssembly> assemblies() const {
//        if (!_ptr) return {};
//
//        size_t count = 0;
//
//        il2cpp_functions funcs;
//        Il2CppAssembly** ptrs = funcs.il2cpp_domain_get_assemblies(reinterpret_cast<Il2CppDomain*>(_ptr), &count);
//        std::vector<Il2CppAssembly> result;
//        result.reserve(count);
//
//        for (size_t i = 0; i < count; ++i) {
//            result.emplace_back(reinterpret_cast<uintptr_t>(ptrs[i]));
//        }
//        return result;
//    }
//
//    // 根据名字打开程序集
//    Il2CppAssembly assembly_open(const std::string& name) const;
//
//private:
//    uintptr_t _ptr = 0;
//};
