#include "pch.h"
#include "PbE.h"
#include "OriginalNameAttribute.h"
#include "PrintHelper.h"
#include "RuntimeType.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <unordered_map>

using namespace Cerydra::CSharp;
using namespace Cerydra::IL2CPP;

namespace
{
    constexpr bool FIX_ENUM = true;

    std::string GetIndent(int indentLevel)
    {
        return std::string(indentLevel * 4, ' ');
    }

    std::string ToLower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    }

    bool IsTopLevelClass(const Class* klass)
    {
        return klass && klass->name.find('/') == std::string::npos && klass->name.find('+') == std::string::npos;
    }

    Assembly* FindGoogleProtobufAssembly()
    {
        for (auto* assembly : Il2CppRuntimeCache::Assemblies()) {
            if (!assembly || !assembly->image) {
                continue;
            }

            if (assembly->name.rfind("Google.Protobuf", 0) == 0) {
                DebugPrintA("[ProtoDump] Found Google.Protobuf Assembly: %s\n", assembly->name.c_str());
                return assembly;
            }

            for (auto* klass : assembly->image->classes) {
                if (klass && klass->namespaze.rfind("Google.Protobuf", 0) == 0) {
                    DebugPrintA("[ProtoDump] Found Google.Protobuf namespace in %s (%s.%s)\n",
                        assembly->name.c_str(),
                        klass->namespaze.c_str(),
                        klass->name.c_str());
                    return assembly;
                }
            }
        }

        return nullptr;
    }

    std::vector<Class*> GetAllProtobufMessages(Assembly* targetAssembly)
    {
        std::vector<Class*> messages;
        auto* iMessageClass = Il2CppRuntimeCache::GetClass("Google.Protobuf.IMessage");
        if (!targetAssembly || !targetAssembly->image || !iMessageClass) {
            return messages;
        }

        for (auto* klass : targetAssembly->image->classes) {
            if (!klass || klass->isEnum || klass->isInterface || !IsTopLevelClass(klass)) {
                continue;
            }

            if (klass->Implements(iMessageClass)) {
                messages.push_back(klass);
            }
        }

        return messages;
    }

    bool HasOriginalNameAttribute(const Field* field)
    {
        if (!field || !field->isLiteral) {
            return false;
        }

        RuntimeType enumType = RuntimeType::FromClass(field->klass);
        auto fieldObject = enumType.GetFieldObject(SystemString::PtrToStringAnsi(field->name.c_str()), 60);
        if (!fieldObject || fieldObject->IsNull()) {
            return false;
        }

        auto attrs = fieldObject->GetCustomAttributes(true);
        for (size_t i = 0; i < attrs.Length(); ++i) {
            RuntimeObject attr = attrs.Get<RuntimeObject>(i);
            auto* nativeClass = attr.GetNativeClass();
            auto* attrClass = Il2CppRuntimeCache::GetClassByAddress(reinterpret_cast<uintptr_t>(nativeClass));
            if (attrClass && attrClass->fullName == "Google.Protobuf.Reflection.OriginalNameAttribute") {
                return true;
            }
        }

        return false;
    }

    std::vector<Class*> GetAllProtobufEnums(Assembly* targetAssembly)
    {
        std::vector<Class*> enums;
        if (!targetAssembly || !targetAssembly->image) {
            return enums;
        }

        for (auto* klass : targetAssembly->image->classes) {
            if (!klass || !klass->isEnum || !IsTopLevelClass(klass)) {
                continue;
            }

            const bool hasOriginalName = std::any_of(klass->fields.begin(), klass->fields.end(), HasOriginalNameAttribute);
            if (hasOriginalName) {
                enums.push_back(klass);
            }
        }

        return enums;
    }

    std::string EnumToString(RuntimeType enumType, int value)
    {
        if (!enumType) {
            return "";
        }

        auto fields = enumType.GetFields(60);
        for (size_t i = 0; i < fields.Length(); ++i) {
            MonoField field(fields.Get<uintptr_t>(i));
            if (!field || !field.IsLiteral()) {
                continue;
            }

            auto enumValueObj = field.GetRawConstantValue();
            if (enumValueObj && enumValueObj.Unbox<int32_t>() == value) {
                return field.GetName().AsString();
            }
        }

        return "";
    }

    template <typename Ret, typename... Args>
    Ret InvokeObject(RuntimeObject obj, const char* methodName, const std::vector<std::string>& argTypes = {}, Args... args)
    {
        return InvokeDynamic<Ret>(obj.RawPtr(), methodName, argTypes, args...);
    }

    NativeList DescriptorList(RuntimeObject obj, const char* getter)
    {
        return InvokeObject<NativeList>(obj, getter);
    }

    std::vector<RuntimeObject> ListToObjects(NativeList list)
    {
        std::vector<RuntimeObject> result;
        if (!list) {
            return result;
        }

        auto items = list.Items();
        for (size_t i = 0; i < items.Length(); ++i) {
            RuntimeObject obj(items.Get<uintptr_t>(i));
            if (obj) {
                result.push_back(obj);
            }
        }
        return result;
    }

    std::vector<RuntimeObject> GetAllFields(RuntimeObject messageDescriptor)
    {
        if (!messageDescriptor) {
            return {};
        }

        auto fieldCollection = InvokeObject<RuntimeObject>(messageDescriptor, "get_Fields");
        auto fieldList = InvokeObject<NativeList>(fieldCollection, "InDeclarationOrder");
        return ListToObjects(fieldList);
    }

    std::string GetFieldTypeString(RuntimeObject field)
    {
        if (!field) {
            return "unknown";
        }

        const int fieldTypeInt = InvokeObject<int>(field, "get_FieldType");
        auto enumType = RuntimeType::FromName("Google.Protobuf.Reflection.FieldType");
        auto fieldType = ToLower(EnumToString(enumType, fieldTypeInt));

        if (fieldType == "int32" || fieldType == "int64" || fieldType == "uint32" || fieldType == "uint64"
            || fieldType == "sint32" || fieldType == "sint64" || fieldType == "fixed32" || fieldType == "fixed64"
            || fieldType == "sfixed32" || fieldType == "sfixed64" || fieldType == "float" || fieldType == "double"
            || fieldType == "bool" || fieldType == "string" || fieldType == "bytes") {
            return fieldType;
        }

        if (fieldType == "enum") {
            auto enumDescriptor = InvokeObject<RuntimeObject>(field, "get_EnumType");
            return enumDescriptor ? InvokeObject<SystemString>(enumDescriptor, "get_Name").AsString() : "unknown_enum";
        }

        if (fieldType == "message") {
            auto messageDescriptor = InvokeObject<RuntimeObject>(field, "get_MessageType");
            return messageDescriptor ? InvokeObject<SystemString>(messageDescriptor, "get_Name").AsString() : "unknown_message";
        }

        return fieldType;
    }

    std::string GetFieldDefinition(RuntimeObject field)
    {
        if (!field) {
            return "";
        }

        const auto fieldName = InvokeObject<SystemString>(field, "get_Name").AsString();
        const int fieldNumber = InvokeObject<int>(field, "get_FieldNumber");
        const bool isMap = InvokeObject<bool>(field, "get_IsMap");

        if (isMap) {
            auto mapEntryDescriptor = InvokeObject<RuntimeObject>(field, "get_MessageType");
            auto mapFields = GetAllFields(mapEntryDescriptor);
            if (mapFields.size() < 2) {
                return "// Error: Map fields count != 2";
            }

            return "map<" + GetFieldTypeString(mapFields[0]) + ", " + GetFieldTypeString(mapFields[1]) + "> "
                + fieldName + " = " + std::to_string(fieldNumber) + ";";
        }

        const bool isRepeated = InvokeObject<bool>(field, "get_IsRepeated");
        return std::string(isRepeated ? "repeated " : "") + GetFieldTypeString(field) + " "
            + fieldName + " = " + std::to_string(fieldNumber) + ";";
    }

    void GenerateEnumDefinitionByDescriptor(RuntimeObject enumDescriptor, std::stringstream& out, int indentLevel = 0)
    {
        if (!enumDescriptor) {
            return;
        }

        const auto indent = GetIndent(indentLevel);
        const auto enumName = InvokeObject<SystemString>(enumDescriptor, "get_Name").AsString();

        out << indent << "// Enum Descriptor Generation\n";
        out << indent << "enum " << enumName << " {\n";

        for (auto valueObj : ListToObjects(DescriptorList(enumDescriptor, "get_Values"))) {
            const auto name = InvokeObject<SystemString>(valueObj, "get_Name").AsString();
            const auto number = InvokeObject<int>(valueObj, "get_Number");
            out << indent << "    " << name << " = " << number << ";\n";
        }

        out << indent << "}\n\n";
    }

    void GenerateMessageDefinitionByDescriptor(RuntimeObject descriptor, std::stringstream& out, int indentLevel = 0)
    {
        if (!descriptor) {
            return;
        }

        const auto name = InvokeObject<SystemString>(descriptor, "get_Name").AsString();
        if (name.empty()) {
            DebugPrintA("[ProtoDump] Message descriptor has no name\n");
            out << "// Error: Message descriptor has no name\n";
            return;
        }

        if (name.size() >= 5 && name.ends_with("Entry")) {
            return;
        }

        const auto indent = GetIndent(indentLevel);
        out << indent << "// Message Descriptor Generation\n";
        out << indent << "message " << name << " {\n";

        auto fields = GetAllFields(descriptor);
        std::unordered_map<uintptr_t, std::vector<RuntimeObject>> oneofGroups;
        std::unordered_map<uintptr_t, RuntimeObject> oneofDescriptors;

        for (auto field : fields) {
            auto containingOneof = InvokeObject<RuntimeObject>(field, "get_ContainingOneof");
            if (containingOneof) {
                oneofGroups[containingOneof.RawPtr()].push_back(field);
                oneofDescriptors[containingOneof.RawPtr()] = containingOneof;
            }
        }

        for (auto& [oneofAddress, fieldList] : oneofGroups) {
            auto oneofDescriptor = oneofDescriptors[oneofAddress];
            const auto oneofName = InvokeObject<SystemString>(oneofDescriptor, "get_Name").AsString();
            out << indent << "    oneof " << oneofName << " {\n";
            for (auto field : fieldList) {
                out << indent << "        " << GetFieldDefinition(field) << "\n";
            }
            out << indent << "    }\n\n";
        }

        for (auto field : fields) {
            if (InvokeObject<RuntimeObject>(field, "get_ContainingOneof")) {
                continue;
            }
            out << indent << "    " << GetFieldDefinition(field) << "\n";
        }

        for (auto nestedEnum : ListToObjects(DescriptorList(descriptor, "get_EnumTypes"))) {
            GenerateEnumDefinitionByDescriptor(nestedEnum, out, indentLevel + 1);
        }

        for (auto nestedMessage : ListToObjects(DescriptorList(descriptor, "get_NestedTypes"))) {
            GenerateMessageDefinitionByDescriptor(nestedMessage, out, indentLevel + 1);
        }

        out << indent << "}\n\n";
    }

    void GenerateEnumDefinition(Class* klass, std::stringstream& out, int indentLevel = 0)
    {
        RuntimeType runtimeType = RuntimeType::FromClass(klass);
        const auto indent = GetIndent(indentLevel);

        out << indent << "// Class: " << klass->name << ", Namespaze: " << klass->namespaze
            << ", Assembly: " << (klass->image && klass->image->assembly ? klass->image->assembly->name : "") << "\n";
        out << indent << "// Il2Cpp Class Generation\n";
        out << indent << "enum " << klass->name << " {\n";

        auto fields = runtimeType.GetFields(60);
        for (size_t i = 0; i < fields.Length(); ++i) {
            MonoField field(fields.Get<uintptr_t>(i));
            if (!field || !field.IsLiteral()) {
                continue;
            }

            auto declaringType = field.GetDeclaringType();
            if (declaringType.GetName().AsString() != klass->name) {
                continue;
            }

            std::optional<std::string> enumName;
            auto attrs = field.GetCustomAttributes(true);
            for (size_t j = 0; j < attrs.Length(); ++j) {
                RuntimeObject attr(attrs.Get<uintptr_t>(j));
                auto* attrClass = Il2CppRuntimeCache::GetClassByAddress(reinterpret_cast<uintptr_t>(attr.GetNativeClass()));
                if (attrClass && attrClass->fullName == "Google.Protobuf.Reflection.OriginalNameAttribute") {
                    enumName = OriginalNameAttribute(attr.RawPtr()).GetName().AsString();
                }
            }
            if (!enumName) {
                enumName = field.GetName().AsString();
            }

            auto rawValue = field.GetRawConstantValue();
            auto value = rawValue.Unbox<int32_t>();
            auto outputName = *enumName;
            if (FIX_ENUM) {
                outputName = klass->name + "_" + outputName;
            }

            out << indent << "    " << outputName << " = " << value << ";\n";
        }

        out << indent << "}\n\n";
    }

    void GenerateMessageDefinition(Class* klass, std::stringstream& out, int indentLevel = 0)
    {
        if (!klass) {
            return;
        }

        auto* descriptorMethod = klass->GetMethod("get_Descriptor", {});
        if (!descriptorMethod) {
            DebugPrintA("[ProtoDump] [WARN] get_Descriptor not found: %s\n", klass->fullName.c_str());
            return;
        }

        RuntimeObject descriptor = InvokeStatic<RuntimeObject>(descriptorMethod);
        const auto indent = GetIndent(indentLevel);
        out << indent << "// Class: " << klass->name << ", Namespaze: " << klass->namespaze
            << ", Assembly: " << (klass->image && klass->image->assembly ? klass->image->assembly->name : "") << "\n";
        GenerateMessageDefinitionByDescriptor(descriptor, out, indentLevel);
    }
}

void DumpProtos2(Assembly* targetAssembly, const char* path)
{
    std::filesystem::path filePath(path);
    const auto directory = filePath.parent_path();
    if (!directory.empty() && !std::filesystem::exists(directory)) {
        std::filesystem::create_directories(directory);
    }

    if (!FindGoogleProtobufAssembly()) {
        DebugPrintA("[ProtoDump] Google.Protobuf not found, skip proto dump\n");
        return;
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        DebugPrintA("[ProtoDump] [ERROR] 打开文件失败: %s\n", path);
        return;
    }

    auto messages = GetAllProtobufMessages(targetAssembly);
    auto enums = GetAllProtobufEnums(targetAssembly);
    DebugPrintA("enums count: %zu\n", enums.size());
    DebugPrintA("messages count: %zu\n", messages.size());

    std::stringstream output;
    output << "// CerydarDumper\n\n";
    output << "syntax = \"proto3\";\n\n";

    for (auto* enumClass : enums) {
        DebugPrintA("messagesEnum: %s.%s\n", enumClass->namespaze.c_str(), enumClass->name.c_str());
        GenerateEnumDefinition(enumClass, output);
    }

    for (auto* messageClass : messages) {
        DebugPrintA("[ProtoDump] Message: %s.%s\n", messageClass->namespaze.c_str(), messageClass->name.c_str());
        GenerateMessageDefinition(messageClass, output);
    }

    file << output.str();
    DebugPrintA("[ProtoDump] dump done!\n");
}
