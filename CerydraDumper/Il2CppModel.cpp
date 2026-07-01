#include "pch.h"
#include "Il2CppModel.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace Cerydra::IL2CPP
{
    namespace
    {
        bool TypeMatches(const Type* type, const std::string& requested)
        {
            const auto normalized = NormalizeRequestedTypeName(requested);
            if (normalized.empty() || normalized == "*") {
                return true;
            }

            if (!type) {
                return false;
            }

            return normalized == type->name
                || normalized == type->aliasName
                || normalized == type->DisplayName();
        }
    }

    std::string MakeFullClassName(const std::string& namespaze, const std::string& name)
    {
        return namespaze.empty() ? name : namespaze + "." + name;
    }

    std::string AliasTypeName(const std::string& name)
    {
        if (name == "System.Int32") return "int";
        if (name == "System.UInt32") return "uint";
        if (name == "System.Int16") return "short";
        if (name == "System.UInt16") return "ushort";
        if (name == "System.Int64") return "long";
        if (name == "System.UInt64") return "ulong";
        if (name == "System.Byte") return "byte";
        if (name == "System.SByte") return "sbyte";
        if (name == "System.Boolean") return "bool";
        if (name == "System.Single") return "float";
        if (name == "System.Double") return "double";
        if (name == "System.String") return "string";
        if (name == "System.Char") return "char";
        if (name == "System.Object") return "object";
        if (name == "System.Void") return "void";
        if (name == "System.Decimal") return "decimal";
        if (name == "System.DateTime") return "DateTime";
        return name;
    }

    std::string NormalizeRequestedTypeName(const std::string& requested)
    {
        auto begin = requested.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos) {
            return "";
        }

        auto end = requested.find_last_not_of(" \t\r\n");
        auto normalized = requested.substr(begin, end - begin + 1);
        auto space = normalized.find_first_of(" \t");
        if (space != std::string::npos) {
            normalized = normalized.substr(0, space);
        }

        return normalized;
    }

    Class* Assembly::GetClass(const std::string& className, const std::string& namespaze, const std::string& parentName) const
    {
        if (!image) {
            return nullptr;
        }

        for (auto klass : image->classes) {
            if (!klass || klass->name != className) {
                continue;
            }

            const bool namespaceMatches = namespaze == "*" || namespaze.empty() || klass->namespaze == namespaze;
            const bool parentMatches = parentName == "*" || parentName.empty() || klass->parent == parentName;
            if (namespaceMatches && parentMatches) {
                return klass;
            }
        }

        return nullptr;
    }

    std::string Type::DisplayName() const
    {
        return aliasName.empty() ? name : aliasName;
    }

    uintptr_t Method::Rva() const
    {
        const auto va = Va();
        if (!va) {
            return 0;
        }

        return va - GetGameAssemblyModuleBase();
    }

    std::string Method::FormatParams() const
    {
        std::ostringstream out;
        out << name << "(";
        for (size_t i = 0; i < args.size(); ++i) {
            const auto type = args[i] ? args[i]->type : nullptr;
            out << (type ? type->DisplayName() : "");
            if (i + 1 < args.size()) {
                out << ",";
            }
        }
        out << ")";
        return out.str();
    }

    std::string Method::SignatureKey() const
    {
        const auto className = klass && klass->byvalType ? klass->byvalType->name : (klass ? klass->fullName : "");
        return className + "::" + FormatParams();
    }

    bool Method::Match(const std::string& methodName, const std::vector<std::string>& argTypes) const
    {
        if (name != methodName || args.size() != argTypes.size()) {
            return false;
        }

        for (size_t i = 0; i < argTypes.size(); ++i) {
            const auto arg = args[i];
            if (!TypeMatches(arg ? arg->type : nullptr, argTypes[i])) {
                return false;
            }
        }

        return true;
    }

    Field* Class::GetField(const std::string& fieldName) const
    {
        for (auto field : fields) {
            if (field && field->name == fieldName) {
                return field;
            }
        }
        return nullptr;
    }

    Method* Class::GetMethod(const std::string& methodName, const std::vector<std::string>& argTypes) const
    {
        for (auto method : methods) {
            if (method && method->Match(methodName, argTypes)) {
                return method;
            }
        }

        return nullptr;
    }

    Method* Class::GetMethodByReturnType(const std::string& returnType, const std::vector<std::string>& argTypes) const
    {
        for (auto method : methods) {
            if (!method || method->args.size() != argTypes.size() || !TypeMatches(method->returnType, returnType)) {
                continue;
            }

            bool argsMatch = true;
            for (size_t i = 0; i < argTypes.size(); ++i) {
                const auto arg = method->args[i];
                if (!TypeMatches(arg ? arg->type : nullptr, argTypes[i])) {
                    argsMatch = false;
                    break;
                }
            }

            if (argsMatch) {
                return method;
            }
        }

        return nullptr;
    }

    bool Class::Implements(const Class* interfaceClass) const
    {
        if (!interfaceClass) {
            return false;
        }

        for (auto* current = this; current; current = current->parentClass) {
            for (auto* iface : current->interfaces) {
                if (iface == interfaceClass) {
                    return true;
                }
            }
        }

        return false;
    }

    void* Class::GetTypeObject()
    {
        if (objType) {
            return objType;
        }

        if (!byvalType || !byvalType->address) {
            return nullptr;
        }

        objType = il2cpp_type_get_object(reinterpret_cast<const Il2CppType*>(byvalType->address));
        return objType;
    }

    void* Class::NewObject() const
    {
        if (!address) {
            return nullptr;
        }

        return il2cpp_object_new(reinterpret_cast<const Il2CppClass*>(address));
    }
}
