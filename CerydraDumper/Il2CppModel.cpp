#include "pch.h"
#include "Il2CppModel.h"
#include "Il2CppRuntimeCache.h"
#include "PrintHelper.h"
#include "RuntimeType.h"
#include "Util.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <il2cpp-blob.h>
#include <il2cpp-tabledefs.h>
#include <limits>
#include <optional>
#include <sstream>

namespace Cerydra::Il2Cpp
{
    namespace
    {
        constexpr int32_t kUnknownType = -1;
        constexpr auto kAllFieldBindingFlags = Cerydra::CSharp::kAllMemberBindingFlags;

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

        std::string EscapeStringLiteral(const std::string& value)
        {
            return "\"" + AsciiEscapeToEscapeLiterals(value) + "\"";
        }

        std::string EscapeCharLiteral(wchar_t value)
        {
            return "'" + AsciiEscapeToEscapeLiterals(Utf16ToUtf8(std::wstring(1, value))) + "'";
        }

        const Type* ResolveEnumUnderlyingType(const Type* type)
        {
            if (!type) {
                return type;
            }

            auto* enumClass = type->klass;
            if (!enumClass && type->address) {
                auto* nativeClass = il2cpp_class_from_type(reinterpret_cast<const Il2CppType*>(type->address));
                enumClass = FindClassByAddress(reinterpret_cast<uintptr_t>(nativeClass));
            }

            if (!enumClass || !enumClass->isEnum) {
                return type;
            }

            for (const auto* field : enumClass->fields) {
                if (field && field->name == "value__" && field->type) {
                    return field->type;
                }
            }

            return type;
        }

        int32_t GetLiteralTypeEnum(const Type* type)
        {
            const auto* resolvedType = ResolveEnumUnderlyingType(type);
            if (!resolvedType) {
                return kUnknownType;
            }
            if (resolvedType->typeEnum != kUnknownType) {
                return resolvedType->typeEnum;
            }

            const auto& name = resolvedType->name;
            const auto& aliasName = resolvedType->aliasName;
            if (name == "System.Boolean" || aliasName == "bool") return IL2CPP_TYPE_BOOLEAN;
            if (name == "System.Char" || aliasName == "char") return IL2CPP_TYPE_CHAR;
            if (name == "System.SByte" || aliasName == "sbyte") return IL2CPP_TYPE_I1;
            if (name == "System.Byte" || aliasName == "byte") return IL2CPP_TYPE_U1;
            if (name == "System.Int16" || aliasName == "short") return IL2CPP_TYPE_I2;
            if (name == "System.UInt16" || aliasName == "ushort") return IL2CPP_TYPE_U2;
            if (name == "System.Int32" || aliasName == "int") return IL2CPP_TYPE_I4;
            if (name == "System.UInt32" || aliasName == "uint") return IL2CPP_TYPE_U4;
            if (name == "System.Int64" || aliasName == "long") return IL2CPP_TYPE_I8;
            if (name == "System.UInt64" || aliasName == "ulong") return IL2CPP_TYPE_U8;
            if (name == "System.Single" || aliasName == "float") return IL2CPP_TYPE_R4;
            if (name == "System.Double" || aliasName == "double") return IL2CPP_TYPE_R8;
            if (name == "System.String" || aliasName == "string") return IL2CPP_TYPE_STRING;
            return kUnknownType;
        }

        template <typename T>
        std::optional<std::string> ReadIl2CppLiteralValue(const Field* field)
        {
            T value{};
            il2cpp_field_static_get_value(reinterpret_cast<FieldInfo*>(field->address), &value);
            std::ostringstream out;
            out << value;
            return out.str();
        }

        std::optional<std::string> FormatBoxedLiteral(Cerydra::CSharp::Object* value, const Type* type)
        {
            const auto typeEnum = GetLiteralTypeEnum(type);
            if (!value) {
                return "null";
            }

            switch (typeEnum) {
            case IL2CPP_TYPE_BOOLEAN:
                return value->Unbox<bool>() ? "true" : "false";
            case IL2CPP_TYPE_CHAR:
                return EscapeCharLiteral(value->Unbox<wchar_t>());
            case IL2CPP_TYPE_I1:
                return std::to_string(static_cast<int>(value->Unbox<int8_t>()));
            case IL2CPP_TYPE_U1:
                return std::to_string(static_cast<unsigned int>(value->Unbox<uint8_t>()));
            case IL2CPP_TYPE_I2:
                return std::to_string(value->Unbox<int16_t>());
            case IL2CPP_TYPE_U2:
                return std::to_string(value->Unbox<uint16_t>());
            case IL2CPP_TYPE_I4:
                return std::to_string(value->Unbox<int32_t>());
            case IL2CPP_TYPE_U4:
                return std::to_string(value->Unbox<uint32_t>());
            case IL2CPP_TYPE_I8:
                return std::to_string(value->Unbox<int64_t>());
            case IL2CPP_TYPE_U8:
                return std::to_string(value->Unbox<uint64_t>());
            case IL2CPP_TYPE_R4:
            {
                std::ostringstream out;
                out << std::setprecision(std::numeric_limits<float>::max_digits10) << value->Unbox<float>() << "f";
                return out.str();
            }
            case IL2CPP_TYPE_R8:
            {
                std::ostringstream out;
                out << std::setprecision(std::numeric_limits<double>::max_digits10) << value->Unbox<double>();
                return out.str();
            }
            case IL2CPP_TYPE_STRING:
            {
                auto* str = reinterpret_cast<Cerydra::CSharp::SystemString*>(value);
                if (!str) {
                    return std::string("null");
                }
                return EscapeStringLiteral(str->AsString());
            }
            default:
                break;
            }

            auto* str = reinterpret_cast<Cerydra::CSharp::SystemDynamic*>(value)->ToString();
            if (!str) {
                return std::nullopt;
            }

            return str->AsString();
        }

        std::optional<std::string> ReadLiteralValueByIl2CppApi(const Field* field)
        {
            if (!field || !field->address || !field->type || !il2cpp_field_static_get_value) {
                return std::nullopt;
            }

            switch (GetLiteralTypeEnum(field->type)) {
            case IL2CPP_TYPE_BOOLEAN:
            {
                bool value{};
                il2cpp_field_static_get_value(reinterpret_cast<FieldInfo*>(field->address), &value);
                return value ? "true" : "false";
            }
            case IL2CPP_TYPE_CHAR:
            {
                wchar_t value{};
                il2cpp_field_static_get_value(reinterpret_cast<FieldInfo*>(field->address), &value);
                return EscapeCharLiteral(value);
            }
            case IL2CPP_TYPE_I1:
            {
                int8_t value{};
                il2cpp_field_static_get_value(reinterpret_cast<FieldInfo*>(field->address), &value);
                return std::to_string(static_cast<int>(value));
            }
            case IL2CPP_TYPE_U1:
            {
                uint8_t value{};
                il2cpp_field_static_get_value(reinterpret_cast<FieldInfo*>(field->address), &value);
                return std::to_string(static_cast<unsigned int>(value));
            }
            case IL2CPP_TYPE_I2:
                return ReadIl2CppLiteralValue<int16_t>(field);
            case IL2CPP_TYPE_U2:
                return ReadIl2CppLiteralValue<uint16_t>(field);
            case IL2CPP_TYPE_I4:
                return ReadIl2CppLiteralValue<int32_t>(field);
            case IL2CPP_TYPE_U4:
                return ReadIl2CppLiteralValue<uint32_t>(field);
            case IL2CPP_TYPE_I8:
                return ReadIl2CppLiteralValue<int64_t>(field);
            case IL2CPP_TYPE_U8:
                return ReadIl2CppLiteralValue<uint64_t>(field);
            case IL2CPP_TYPE_R4:
            {
                float value{};
                il2cpp_field_static_get_value(reinterpret_cast<FieldInfo*>(field->address), &value);
                std::ostringstream out;
                out << std::setprecision(std::numeric_limits<float>::max_digits10) << value << "f";
                return out.str();
            }
            case IL2CPP_TYPE_R8:
            {
                double value{};
                il2cpp_field_static_get_value(reinterpret_cast<FieldInfo*>(field->address), &value);
                std::ostringstream out;
                out << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
                return out.str();
            }
            case IL2CPP_TYPE_STRING:
            {
                Il2CppString* value{};
                il2cpp_field_static_get_value(reinterpret_cast<FieldInfo*>(field->address), &value);
                if (!value) {
                    return std::string("null");
                }
                return EscapeStringLiteral(Il2CppStringToUtf8String(value));
            }
            default:
                return std::nullopt;
            }
        }

        std::optional<std::string> ReadLiteralValueByCSharp(const Field* field)
        {
            if (!field || !field->klass || field->name.empty()) {
                return std::nullopt;
            }

            auto* runtimeType = field->klass->GetTypeObject();
            if (!runtimeType) {
                return std::nullopt;
            }

            auto* fieldName = Cerydra::CSharp::SystemString::PtrToStringAnsi(field->name.c_str());
            auto* monoField = fieldName ? runtimeType->GetFieldObject(fieldName, kAllFieldBindingFlags) : nullptr;
            if (!monoField || !monoField->IsLiteral()) {
                return std::nullopt;
            }

            return FormatBoxedLiteral(monoField->GetRawConstantValue(), field->type);
        }

        std::string TypeNameForParam(const Type* type)
        {
            if (!type) {
                return "void";
            }

            auto typeName = type->DisplayName();
            return typeName.empty() ? "object" : typeName;
        }

        bool ClassNameMatches(const Class* klass, const std::string& fullOrAliasName)
        {
            if (!klass) {
                return false;
            }

            if (klass->fullName == fullOrAliasName || klass->name == fullOrAliasName) {
                return true;
            }

            return klass->byvalType
                && (klass->byvalType->name == fullOrAliasName || klass->byvalType->aliasName == fullOrAliasName);
        }

        bool TypeAddressMatches(const Type* type, uintptr_t address)
        {
            return type && reinterpret_cast<uintptr_t>(type->address) == address;
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

    Assembly* Get(const std::string& assemblyName)
    {
        return Il2CppRuntimeCache::GetAssembly(assemblyName);
    }

    Class* FindClass(const std::string& fullOrAliasName)
    {
        for (auto* assembly : Il2CppRuntimeCache::Assemblies()) {
            if (!assembly || !assembly->image) {
                continue;
            }

            for (auto* klass : assembly->image->classes) {
                if (ClassNameMatches(klass, fullOrAliasName)) {
                    return klass;
                }
            }
        }

        return nullptr;
    }

    Class* FindClass(const std::string& namespaze, const std::string& name)
    {
        return FindClass(MakeFullClassName(namespaze, name));
    }

    Class* FindClassByAddress(uintptr_t address)
    {
        if (!address) {
            return nullptr;
        }

        for (auto* assembly : Il2CppRuntimeCache::Assemblies()) {
            if (!assembly || !assembly->image) {
                continue;
            }

            for (auto* klass : assembly->image->classes) {
                if (klass && reinterpret_cast<uintptr_t>(klass->address) == address) {
                    return klass;
                }
            }
        }

        return nullptr;
    }

    Field* FindFieldByAddress(uintptr_t address)
    {
        if (!address) {
            return nullptr;
        }

        for (auto* assembly : Il2CppRuntimeCache::Assemblies()) {
            if (!assembly || !assembly->image) {
                continue;
            }

            for (auto* klass : assembly->image->classes) {
                if (!klass) {
                    continue;
                }

                for (auto* field : klass->fields) {
                    if (field && reinterpret_cast<uintptr_t>(field->address) == address) {
                        return field;
                    }
                }
            }
        }

        return nullptr;
    }

    Type* FindTypeByAddress(uintptr_t address)
    {
        if (!address) {
            return nullptr;
        }

        for (auto* assembly : Il2CppRuntimeCache::Assemblies()) {
            if (!assembly || !assembly->image) {
                continue;
            }

            for (auto* klass : assembly->image->classes) {
                if (!klass) {
                    continue;
                }

                if (TypeAddressMatches(klass->byvalType, address)) {
                    return klass->byvalType;
                }

                for (auto* field : klass->fields) {
                    if (field && TypeAddressMatches(field->type, address)) {
                        return field->type;
                    }
                }

                for (auto* method : klass->methods) {
                    if (!method) {
                        continue;
                    }

                    if (TypeAddressMatches(method->returnType, address)) {
                        return method->returnType;
                    }

                    for (auto* arg : method->args) {
                        if (arg && TypeAddressMatches(arg->type, address)) {
                            return arg->type;
                        }
                    }
                }
            }
        }

        return nullptr;
    }

    Image* Assembly::Get() const
    {
        return image;
    }

    Class* Assembly::Get(const std::string& className, const std::string& namespaze, const std::string& parentName) const
    {
        return image ? image->Get(className, namespaze, parentName) : nullptr;
    }

    Class* Image::Get(const std::string& className, const std::string& namespaze, const std::string& parentName) const
    {
        for (auto klass : classes) {
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

    bool Field::IsLiteral() const
    {
        return isLiteral || (flags & FIELD_ATTRIBUTE_LITERAL) != 0;
    }

    std::string Field::LiteralValue() const
    {
        if (!IsLiteral()) {
            return "";
        }

        try {
            if (auto value = ReadLiteralValueByIl2CppApi(this)) {
                return *value;
            }

            if (auto value = ReadLiteralValueByCSharp(this)) {
                return *value;
            }
        }
        catch (const std::exception& ex) {
            DebugPrintA(
                "[DumpCs] 常量读取失败: %s.%s, %s\n",
                klass ? klass->fullName.c_str() : "<unknown>",
                name.c_str(),
                ex.what());
        }
        catch (...) {
            DebugPrintA(
                "[DumpCs] 常量读取失败: %s.%s, 未知异常\n",
                klass ? klass->fullName.c_str() : "<unknown>",
                name.c_str());
        }

        return "";
    }

    std::string Method::Arg::DisplayName(size_t index) const
    {
        return name.empty() ? "arg" + std::to_string(index + 1) : name;
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

    std::string Method::ParamModifier(size_t index) const
    {
        if (index >= args.size() || !args[index] || !args[index]->type) {
            return "";
        }

        const auto* type = args[index]->type;
        if (type->byRef) {
            if ((type->attrs & PARAM_ATTRIBUTE_OUT) && !(type->attrs & PARAM_ATTRIBUTE_IN)) {
                return "out ";
            }
            if ((type->attrs & PARAM_ATTRIBUTE_IN) && !(type->attrs & PARAM_ATTRIBUTE_OUT)) {
                return "in ";
            }
            return "ref ";
        }

        std::string modifier;
        if (type->attrs & PARAM_ATTRIBUTE_IN) {
            modifier += "[In] ";
        }
        if (type->attrs & PARAM_ATTRIBUTE_OUT) {
            modifier += "[Out] ";
        }
        return modifier;
    }

    std::string Method::FormatParam(size_t index, bool includeModifier) const
    {
        if (index >= args.size()) {
            return "";
        }

        const auto* arg = args[index];
        std::ostringstream out;
        if (includeModifier) {
            out << ParamModifier(index);
        }
        out << TypeNameForParam(arg ? arg->type : nullptr) << " "
            << (arg ? arg->DisplayName(index) : "arg" + std::to_string(index + 1));
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
        if (!interfaceClass || !address || !interfaceClass->address) {
            return false;
        }

        return il2cpp_class_is_subclass_of(
            reinterpret_cast<Il2CppClass*>(address),
            reinterpret_cast<Il2CppClass*>(interfaceClass->address),
            true);
    }

    Cerydra::CSharp::RuntimeType* Class::GetTypeObject()
    {
        if (objType) {
            return reinterpret_cast<Cerydra::CSharp::RuntimeType*>(objType);
        }

        if (!byvalType || !byvalType->address) {
            return nullptr;
        }

        objType = il2cpp_type_get_object(reinterpret_cast<const Il2CppType*>(byvalType->address));
        return reinterpret_cast<Cerydra::CSharp::RuntimeType*>(objType);
    }

    Cerydra::CSharp::Object* Class::NewObject() const
    {
        if (!address) {
            return nullptr;
        }

        return reinterpret_cast<Cerydra::CSharp::Object*>(
            il2cpp_object_new(reinterpret_cast<const Il2CppClass*>(address)));
    }
}
