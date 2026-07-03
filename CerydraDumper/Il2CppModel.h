#pragma once

#include "Il2CppFunctions.h"
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

namespace Cerydra::CSharp
{
    class Object;
    class RuntimeType;
}

namespace Cerydra::IL2CPP
{
    struct Assembly;
    struct Image;
    struct Class;
    struct Type;
    struct Field;
    struct Method;

    std::string MakeFullClassName(const std::string& namespaze, const std::string& name);
    std::string AliasTypeName(const std::string& name);
    std::string NormalizeRequestedTypeName(const std::string& requested);

    struct Assembly final
    {
        void* address{};
        std::string name;
        std::string file;
        Image* image{};

        Class* GetClass(
            const std::string& name,
            const std::string& namespaze = "*",
            const std::string& parent = "*") const;
    };

    struct Image final
    {
        void* address{};
        std::string name;
        std::string file;
        Assembly* assembly{};
        std::vector<Class*> classes;
    };

    struct Type final
    {
        void* address{};
        std::string name;
        std::string aliasName;
        int32_t typeEnum{-1};
        uint32_t attrs{};
        bool byRef{};
        Class* klass{};

        std::string DisplayName() const;
    };

    struct Field final
    {
        void* address{};
        std::string name;
        Type* type{};
        Class* klass{};
        int32_t flags{};
        int32_t offset{};
        bool isStatic{};
        bool isLiteral{};

        bool IsLiteral() const;
        std::string LiteralValue() const;

        template <typename T>
        void GetStaticValue(T* value) const
        {
            il2cpp_field_static_get_value(reinterpret_cast<FieldInfo*>(address), value);
        }

        template <typename T>
        void SetStaticValue(T* value) const
        {
            il2cpp_field_static_set_value(reinterpret_cast<FieldInfo*>(address), value);
        }

        template <typename T, typename C>
        struct Variable
        {
            int32_t offset{};

            void Init(const Field* field)
            {
                offset = field ? field->offset : 0;
            }

            T Get(C* obj) const
            {
                return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(obj) + offset);
            }

            void Set(C* obj, T value) const
            {
                *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(obj) + offset) = value;
            }

            T& operator[](C* obj) const
            {
                return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(obj) + offset);
            }
        };
    };

    struct Method final
    {
        void* address{};
        std::string name;
        Class* klass{};
        Type* returnType{};
        int32_t flags{};
        bool isStatic{};
        void* function{};

        struct Arg
        {
            std::string name;
            Type* type{};

            std::string DisplayName(size_t index) const;
        };

        std::vector<Arg*> args;

        uintptr_t Va() const
        {
            return reinterpret_cast<uintptr_t>(function);
        }

        uintptr_t Rva() const;
        std::string FormatParams() const;
        std::string ParamModifier(size_t index) const;
        std::string FormatParam(size_t index, bool includeModifier) const;
        std::string SignatureKey() const;
        bool Match(const std::string& methodName, const std::vector<std::string>& argTypes) const;

        template <typename Return, typename... Args>
        Return Invoke(Args... callArgs) const
        {
            auto fn = reinterpret_cast<Return(__fastcall*)(Args...)>(function);
            if constexpr (std::is_void_v<Return>) {
                fn(callArgs...);
            }
            else {
                return fn(callArgs...);
            }
        }
    };

    struct Class final
    {
        void* address{};
        std::string name;
        std::string fullName;
        std::string namespaze;
        std::string parent;
        Image* image{};
        Class* parentClass{};
        Type* byvalType{};
        void* objType{};
        int32_t flags{};
        bool isEnum{};
        bool isValueType{};
        bool isInterface{};

        std::vector<Field*> fields;
        std::vector<Method*> methods;
        std::vector<Class*> interfaces;

        Field* GetField(const std::string& name) const;
        Method* GetMethod(const std::string& name, const std::vector<std::string>& args = {}) const;
        Method* GetMethodByReturnType(const std::string& returnType, const std::vector<std::string>& args = {}) const;
        bool Implements(const Class* interfaceClass) const;

        template <typename RType>
        RType GetValue(void* obj, const std::string& fieldName) const
        {
            auto field = GetField(fieldName);
            if (!obj || !field || field->offset < 0 || field->isStatic) {
                return RType{};
            }

            return *reinterpret_cast<RType*>(reinterpret_cast<uintptr_t>(obj) + field->offset);
        }

        template <typename RType>
        void SetValue(void* obj, const std::string& fieldName, RType value) const
        {
            auto field = GetField(fieldName);
            if (!obj || !field || field->offset < 0 || field->isStatic) {
                return;
            }

            *reinterpret_cast<RType*>(reinterpret_cast<uintptr_t>(obj) + field->offset) = value;
        }

        Cerydra::CSharp::RuntimeType* GetTypeObject();
        Cerydra::CSharp::Object* NewObject() const;
    };
}
