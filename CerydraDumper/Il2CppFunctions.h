#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "./il2cpp/il2cpp-api-types.h"

uintptr_t GetGameAssemblyModuleBase();

uintptr_t GetUnityPlayerModuleBase();

struct MethodInfo
{
    const Il2CppClass* klass;
    const void* method_pointer;
    uint8_t _pad[0x1A];
    uint16_t flags;
};

static_assert(offsetof(MethodInfo, method_pointer) == 0x8);
static_assert(offsetof(MethodInfo, flags) == 0x2A);

inline std::unordered_map<std::string, void*> address_{};

void* FindIl2CppAddress(const std::string& funcName);

template <typename Return, typename... Args>
static auto Invoke(const std::string& funcName, Args... args) -> Return
{
    auto address = FindIl2CppAddress(funcName);
    if (!address) {
        throw std::runtime_error("IL2CPP API 未绑定: " + funcName);
    }

    auto fn = reinterpret_cast<Return(*)(Args...)>(address);
    if constexpr (std::is_void_v<Return>) {
        fn(std::forward<Args>(args)...);
    }
    else {
        return fn(std::forward<Args>(args)...);
    }
}

template <typename Signature>
class Il2CppApiStub;

template <typename Return, typename... Args>
class Il2CppApiStub<Return(Args...)>
{
public:
    constexpr explicit Il2CppApiStub(const char* funcName)
        : funcName_(funcName)
    {
    }

    auto operator()(Args... args) const -> Return
    {
        return Invoke<Return, Args...>(funcName_, std::forward<Args>(args)...);
    }

    [[nodiscard]] void* address() const
    {
		return FindIl2CppAddress(funcName_);
    }

    explicit operator bool() const
    {
        return address() != nullptr;
    }

    bool operator!() const
    {
        return address() == nullptr;
    }

private:
    const char* funcName_;
};

#define DO_API(r, n, p) inline Il2CppApiStub<r p> n{#n};
#define DO_API_NO_RETURN(r, n, p) inline Il2CppApiStub<r p> n{#n};
#include "./il2cpp/il2cpp-api-functions.h"
#undef DO_API
#undef DO_API_NO_RETURN

void InitIl2CppFunctions();
