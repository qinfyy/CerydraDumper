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
    class Assembly;
    class Image;
    class Class;
    class Type;
    class Field;
    class Method;

    std::string MakeFullClassName(const std::string& namespaze, const std::string& name);
    std::string AliasTypeName(const std::string& name);
    std::string NormalizeRequestedTypeName(const std::string& requested);
    Assembly* Get(const std::string& assemblyName);
    Class* FindClass(const std::string& fullOrAliasName);
    Class* FindClass(const std::string& namespaze, const std::string& name);
    Class* FindClassByAddress(uintptr_t address);
    Field* FindFieldByAddress(uintptr_t address);
    Type* FindTypeByAddress(uintptr_t address);

    class Assembly final
    {
    public:
        void* address{};
        std::string name;
        std::string file;
        Image* image{};

        Image* Get() const;
        Class* Get(
            const std::string& name,
            const std::string& namespaze = "*",
            const std::string& parent = "*") const;
    };

    class Image final
    {
    public:
        void* address{};
        std::string name;
        std::string file;
        Assembly* assembly{};
        std::vector<Class*> classes;

        Class* Get(
            const std::string& name,
            const std::string& namespaze = "*",
            const std::string& parent = "*") const;
    };

    class Type final
    {
    public:
        void* address{};
        std::string name;
        std::string aliasName;
        int32_t typeEnum{-1};
        uint32_t attrs{};
        bool byRef{};
        Class* klass{};

        std::string DisplayName() const;
    };

    class Field final
    {
    public:
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

    class Method final
    {
    public:
        void* address{};
        std::string name;
        Class* klass{};
        Type* returnType{};
        int32_t flags{};
        bool isStatic{};
        void* function{};

        class Arg
        {
        public:
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

    class Class final
    {
    public:
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
        RType* Get(const std::string& memberName, const std::vector<std::string>& args = {}) const
        {
            if constexpr (std::is_same_v<RType, Field>) {
                return GetField(memberName);
            }
            else if constexpr (std::is_same_v<RType, Method>) {
                auto* method = GetMethod(memberName, args);
                if (method || !args.empty()) {
                    return method;
                }

                for (auto* candidate : methods) {
                    if (candidate && candidate->name == memberName) {
                        return candidate;
                    }
                }

                return nullptr;
            }
            else if constexpr (std::is_same_v<RType, std::int32_t>) {
                auto* field = GetField(memberName);
                return field ? reinterpret_cast<RType*>(static_cast<intptr_t>(field->offset)) : nullptr;
            }
            else {
                return nullptr;
            }
        }

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
