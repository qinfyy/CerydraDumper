#include "pch.h"
#include "PbE.h"
#include "Il2CppFunctions.h"
#include <vector>
#include <sstream>
#include "PrintHelper.h"
#include "Util.h"
#include <filesystem>
#include <unordered_map>
#include <fstream>
#include <windows.h>
#include "CAppDomain.h"
#include <sstream>
#include <string>
#include <vector>
#include "CSharpRuntime.h"
#include "PbUtil.h"
#include <optional>
#include "OriginalNameAttribute.h"
#include <regex>
#include <DbgHelp.h>
#include <iostream>
#include <filesystem>

static const bool FIX_ENUM = true;
static const char* PROTOBUF_IMAGE = "Assembly-CSharp.dll";

static Il2CppClass* _originalNameAttr = nullptr;

std::optional<CMonoAssembly> HasGoogleProtobuf()
{
    auto domain = CAppDomain::GetCurrentDomain();
    auto assemblies = domain.GetAssemblies().to_vec<CMonoAssembly>();

    for (size_t i = 0; i < assemblies.size(); ++i)
    {
        CMonoAssembly mono_assembly(assemblies[i]);
        std::string assembly_name = mono_assembly.GetName();

        if (assembly_name.rfind("Google.Protobuf", 0) == 0)
        {
            DebugPrintA("[ProtoDump] Found Google.Protobuf Assembly: %s\n", assembly_name.c_str());
            return mono_assembly;
        }

        auto types = mono_assembly.GetTypes(true);
        for (size_t j = 0; j < types.length(); ++j)
        {
            auto type_ptr = types.get<uintptr_t>(j);
            CRuntimeType type(type_ptr);

            auto typeNsp = type.GetNamespace().AsString();
            auto typeName = type.GetName().AsString();
            if (typeNsp.rfind("Google.Protobuf", 0) == 0)
            {
                DebugPrintA("[ProtoDump] Found Google.Protobuf namespace in %s (%s.%s)\n", assembly_name.c_str(), typeNsp.c_str(), typeName.c_str());
                return mono_assembly;
            }
        }
    }

    return std::nullopt;
}

static std::vector<CRuntimeType> GetAllProtobufMessages(CMonoAssembly monoAssembly, CMonoAssembly gpb)
{
    std::vector<CRuntimeType> messages;

    auto _iMessageType = gpb.GetTypeByName("Google.Protobuf.IMessage");

    auto types = monoAssembly.GetTypes(60 != 0);
    for (size_t i = 0; i < types.length(); ++i) {
        uintptr_t type_ptr = types.get<uintptr_t>(i);
        CRuntimeType runtime_type(type_ptr);

        if (runtime_type.IsGenericType() || runtime_type.IsEnum() || !runtime_type.GetDeclaringType().is_null())
            continue;

        // 获取当前类型的接口
        auto ifnsVec = runtime_type.GetInterfaces().to_vec<CRuntimeType>();
        bool isMessageType = false;
        for (size_t j = 0; j < ifnsVec.size(); ++j) {
            CRuntimeType ifn_type(ifnsVec[j]);
            if (_iMessageType.IsAssignableFrom(ifn_type)) {
                isMessageType = true;
                break;
            }
        }

        if (isMessageType) {
            messages.push_back(runtime_type);
        }
    }

    return messages;
}

static std::vector<CRuntimeType> GetAllProtobufEnums(CMonoAssembly monoAssembly)
{
    std::vector<CRuntimeType> enums;

    // 获取 assembly 内所有类型
    auto types = monoAssembly.GetTypes(60 != 0); // length 或者 max 60
    for (size_t i = 0; i < types.length(); ++i) {
        uintptr_t type_ptr = types.get<uintptr_t>(i);
        CRuntimeType t(type_ptr);
        DebugPrintA("CRuntimeType Name: %s\n", t.GetName().AsString().c_str());

        // 必须是 enum，且没有 DeclaringType（顶层 enum）
        if (!t.IsEnum() || !t.GetDeclaringType().is_null())
            continue;

        bool hasOriginalName = false;

        // 遍历字段
        auto fields = t.GetFields(60); // 60 = 最大字段数
        for (size_t j = 0; j < fields.length(); ++j) {
            auto field_ptr = fields.get<uintptr_t>(j);
            CMonoField field(field_ptr);

            // 跳过非字面量字段（Enum 成员）
            if (!field.IsLiteral())
                continue;

            // 检查 OriginalNameAttribute
            auto attrs = field.GetCustomAttributes(true);
            for (size_t k = 0; k < attrs.length(); ++k) {
                CIl2CppObject attrObj = attrs.get<CIl2CppObject>(k);
                if (!attrObj)
                    continue;

                auto className = attrObj.get_class().name();
                if (className == "OriginalNameAttribute") {
                    hasOriginalName = true;
                    break;
                }
            }

            if (hasOriginalName)
                break;
        }

        if (hasOriginalName)
            enums.push_back(t);
    }

    return enums;
}

static std::string EnumToString(CRuntimeType enumType, int value)
{
    if (!enumType)
        return "";

    void* iter = nullptr;
    FieldInfo* field = nullptr;

    auto fields = enumType.GetFields(60);

    for (auto i = 0; i < fields.length(); i++) {
        auto field = fields.get<CMonoField>(i);

        if (!field.IsLiteral())
            continue;

        uintptr_t enum_val_ptr = field.GetRawConstantValue();
        CIl2CppObject enum_val_obj(enum_val_ptr);
        int32_t fieldValue = enum_val_obj.unbox<int32_t>();
        //il2cpp_field_static_get_value(field, &fieldValue);

        if (fieldValue == value)
        {
            return field.GetName();
        }
    }

    return "";
}

// 获取 FieldType 名称
static std::string GetFieldTypeString(CIl2CppObject field)
{
    if (!field)
        return "unknown";

    Il2CppClass* fieldClass = il2cpp_object_get_class(field);
    if (!fieldClass)
        return "unknown";

    // 获取 FieldType 枚举值
    int fieldTypeInt = CallIl2CppInstanceObjectMethodDynamic<int>(field, "get_FieldType", {});

    auto enumT = CRuntimeType::FromName("Google.Protobuf.Reflection.FieldType");
    std::string fieldTypeName = EnumToString(enumT, fieldTypeInt);


    for (auto& c : fieldTypeName) c = tolower(c); // 转小写
    std::string FieldType = fieldTypeName;

    // 处理基本类型
    if (FieldType == "int32") return "int32";
    if (FieldType == "int64") return "int64";
    if (FieldType == "uint32") return "uint32";
    if (FieldType == "uint64") return "uint64";
    if (FieldType == "sint32") return "sint32";
    if (FieldType == "sint64") return "sint64";
    if (FieldType == "fixed32") return "fixed32";
    if (FieldType == "fixed64") return "fixed64";
    if (FieldType == "sfixed32") return "sfixed32";
    if (FieldType == "sfixed64") return "sfixed64";
    if (FieldType == "float") return "float";
    if (FieldType == "double") return "double";
    if (FieldType == "bool") return "bool";
    if (FieldType == "string") return "string";
    if (FieldType == "bytes") return "bytes";


    // enum 类型
    if (FieldType == "enum")
    {
        // 获取 EnumDescriptor
        CIl2CppObject enumDescriptor = CallIl2CppInstanceObjectMethodDynamic<CIl2CppObject>(field, "get_EnumType", {});

        if (!enumDescriptor)
            return "unknown_enum";

        CSystemString nameObj = CallIl2CppInstanceObjectMethodDynamic<CSystemString>(enumDescriptor, "get_Name", {});
        if (!enumDescriptor)
            return "unknown_enum";

        return nameObj;
    }


    if (FieldType == "message")
    {

        CIl2CppObject enumDescriptor = CallIl2CppInstanceObjectMethodDynamic<CIl2CppObject>(field, "get_MessageType", {});

        if (!enumDescriptor)
            return "unknown_message";

        CSystemString nameObj = CallIl2CppInstanceObjectMethodDynamic<CSystemString>(enumDescriptor, "get_Name", {});
        if (!enumDescriptor)
            return "unknown_message";

        return nameObj;
    }

    return FieldType;
}

static std::vector<CIl2CppObject> GetAllFields(CIl2CppObject messageDescriptor)
{
    std::vector<CIl2CppObject> fields;

    if (!messageDescriptor)
        return fields;

    //CIl2CppObject fieldCollection = GetFieldsCollection(messageDescriptor);
    
    //class FieldCollection
    uintptr_t fieldCollection = CallIl2CppInstanceObjectMethodDynamic<uintptr_t>(messageDescriptor, "get_Fields", {});

    auto fieldList = CallIl2CppInstanceObjectMethodDynamic<CNativeList>(fieldCollection, "InDeclarationOrder", {});
    if (!fieldList)
        return fields;

    auto fieldArr = fieldList.Items();
    for (auto i = 0; i < fieldArr.length(); i++) {
        auto obj_ptr = fieldArr.get<uintptr_t>(i);
        CIl2CppObject obj(obj_ptr);
        fields.push_back(obj);
    }

    return fields;
}

static std::string GetFieldDefinition(CIl2CppObject field)
{
    if (!field) return "";

    auto sysstrFieldName = CallIl2CppInstanceObjectMethodDynamic<CSystemString>(field, "get_Name", {});
    auto fieldName = sysstrFieldName.AsString();

    int fieldNumber = CallIl2CppInstanceObjectMethodDynamic<int>(field, "get_FieldNumber", {});

    auto isMap = CallIl2CppInstanceObjectMethodDynamic<bool>(field, "get_IsMap", {});

    if (isMap)
    {
        CIl2CppObject mapEntryDescriptor = CallIl2CppInstanceObjectMethodDynamic<CIl2CppObject>(field, "get_MessageType", {});

        CIl2CppObject mapFieldsCollection = CallIl2CppInstanceObjectMethodDynamic<CIl2CppObject>(mapEntryDescriptor, "get_Fields", {});
        if (!mapFieldsCollection)
            return "// Error: Map FieldCollection missing";

        auto mapFields = GetAllFields(mapEntryDescriptor);
        if (mapFields.size() < 2)
            return "// Error: Map fields count != 2";
        std::string keyType = GetFieldTypeString(mapFields[0]);
        std::string valueType = GetFieldTypeString(mapFields[1]);

        return "map<" + keyType + ", " + valueType + "> " + fieldName + " = " + std::to_string(fieldNumber) + ";";
    }
    else
    {
        auto isRepeated = CallIl2CppInstanceObjectMethodDynamic<bool>(field, "get_IsRepeated", {});

        std::string typeStr = GetFieldTypeString(field);
        std::string repeatedStr = isRepeated ? "repeated " : "";

        return repeatedStr + typeStr + " " + fieldName + " = " + std::to_string(fieldNumber) + ";";
    }
}

static std::string GetIndent(int indentLevel)
{
    return std::string(indentLevel * 4, ' ');
}

static void GenerateEnumDefinitionByDescriptor(CIl2CppObject enumDescriptor, std::stringstream& out, int indentLevel = 0)
{
    if (!enumDescriptor)
        return;

    std::string indent = GetIndent(indentLevel);

    std::string enumName = CallIl2CppInstanceObjectMethodDynamic<CSystemString>(enumDescriptor, "get_Name", {});

    out << indent << "// Enum Descriptor Generation\n";
    out << indent << "enum " << enumName << " {\n";

    auto valueListObj = CallIl2CppInstanceObjectMethodDynamic<CNativeList>(enumDescriptor, "get_Values", {});
    auto valueArrayObj = valueListObj.Items();
    auto count = valueArrayObj.length();

    for (int i = 0; i < count; ++i)
    {
        uintptr_t obj_ptr = valueArrayObj.get<uintptr_t>(i);
        if (obj_ptr)
        {
            std::string sName = CallIl2CppInstanceObjectMethodDynamic<CSystemString>(obj_ptr, "get_Name", {});
            auto iNum = CallIl2CppInstanceObjectMethodDynamic<int>(obj_ptr, "get_Number", {});

            out << indent << "    " << sName << " = " << iNum << ";\n";
        }
    }

    out << indent << "}\n\n";
}

static void GenerateMessageDefinitionByDescriptor(CIl2CppObject descriptor, std::stringstream& out, int indentLevel = 0)
{
    if (!descriptor) return;

    auto klass = descriptor.get_class();
    CSystemString systemString = CallIl2CppInstanceObjectMethodDynamic<CSystemString>(descriptor, "get_Name", {});
    std::string name = systemString;

    if (name.empty()) {
        DebugPrintA("[ProtoDump] Message descriptor has no name\n");
        out << "// Error: Message descriptor has no name\n";
        return;
    }

    if (name.size() >= 5 && std::equal(name.end() - 5, name.end(), L"Entry"))
        return;

    std::string indent = GetIndent(indentLevel);

    out << indent << "// Message Descriptor Generation\n";
    out << indent << "message " << name << " {\n";

    auto fields = GetAllFields(descriptor);
    std::unordered_map<CIl2CppObject*, std::vector<CIl2CppObject>> oneofGroups;

    for (auto field : fields)
    {
        //FieldCollection

        auto name = field.get_class().name();

        CIl2CppObject containingOneof = CallIl2CppInstanceObjectMethodDynamic<CIl2CppObject>(field, "get_ContainingOneof", {});

        if (containingOneof)
            oneofGroups[&containingOneof].push_back(field);
    }

    for (auto& [oneofDesc, fieldList] : oneofGroups)
    {
        auto cls = oneofDesc->get_class();
        auto sysstrOneofName = CallIl2CppInstanceObjectMethodDynamic<CSystemString>(*oneofDesc, "get_Name", {});
        auto oneofName = sysstrOneofName.AsString();
        out << indent << "    oneof " << oneofName << " {\n";
        for (auto field : fieldList)
            out << indent << "        " << GetFieldDefinition(field) << "\n";
        out << indent << "    }\n\n";
    }

    for (auto field : fields)
    {
        CIl2CppObject containingOneof = CallIl2CppInstanceObjectMethodDynamic<CIl2CppObject>(field, "get_ContainingOneof", {});
        if (containingOneof)
            continue;

        out << indent << "    " << GetFieldDefinition(field) << "\n";
    }

    //// 嵌套枚举
    std::vector<CIl2CppObject> nestedEnums;
    auto enumListObj = CallIl2CppInstanceObjectMethodDynamic<CNativeList>(descriptor, "get_EnumTypes", {});
    auto enumArrayObj = enumListObj.Items();
    auto enumArrayCount = enumArrayObj.length();
    for (int i = 0; i < enumArrayCount; ++i)
    {
        uintptr_t obj_ptr = enumArrayObj.get<uintptr_t>(i);
        CIl2CppObject obj(obj_ptr);
        if (obj)
            nestedEnums.push_back(obj);
    }

    for (auto e : nestedEnums)
        GenerateEnumDefinitionByDescriptor(e, out, indentLevel + 1);

    // 嵌套消息
    std::vector<CIl2CppObject> nestedMessages;
    auto messageListObj = CallIl2CppInstanceObjectMethodDynamic<CNativeList>(descriptor, "get_NestedTypes", {});
    auto messageArrayObj = messageListObj.Items();
    auto messageArrayCount = messageArrayObj.length();
    for (int i = 0; i < messageArrayCount; ++i)
    {
        uintptr_t obj_ptr = messageArrayObj.get<uintptr_t>(i);
        CIl2CppObject obj(obj_ptr);
        if (obj)
            nestedMessages.push_back(obj);
    }

    for (auto nested : nestedMessages)
        GenerateMessageDefinitionByDescriptor(nested, out, indentLevel + 1);

    out << indent << "}\n\n";
}

static void GenerateEnumDefinition(CRuntimeType rt, std::stringstream& out, int indentLevel = 0)
{
    std::string indent = GetIndent(indentLevel);
    std::string className = rt.GetName();
    std::string nsp = rt.GetNamespace();
    //auto* clsImage = il2cpp_class_get_image(enumClass);
    std::string assemblyName = rt.GetAssemblyName();

    out << indent << "// Class: " << className << ", Namespaze: " << nsp << ", Assembly: " << assemblyName << "\n";
    out << indent << "// Il2Cpp Class Generation\n";
    out << indent << "enum " << className << " {\n";

    void* iter = nullptr;
    FieldInfo* field = nullptr;

    auto fields = rt.GetFields(60);
    size_t i = 0;
    for (size_t i = 0; i < fields.length(); ++i) {
        auto field = fields.get<CMonoField>(i);
        if (!field) continue;

        auto fieldType = field.GetFieldType();
        auto full_name_field = fieldType.GetFullName().AsString();
        if (field.GetDeclaringType().GetName().AsString() != rt.GetName().AsString()) continue;

        if (!field.IsLiteral())
            continue;

        std::optional<std::string> enum_key;
        auto attrs = field.GetCustomAttributes(true);
        for (int j = 0; j < attrs.length(); ++j) {
            auto n_rt = CIl2CppObject(attrs.get<uintptr_t>(j));
            if (n_rt.get_class().name() == "OriginalNameAttribute") {
                enum_key = OriginalNameAttribute(n_rt.raw_ptr()).GetName();
            }
        }
        if (!enum_key.has_value()) enum_key = field.GetName();

        uintptr_t ptr = field.GetRawConstantValue();
        CIl2CppObject obj_val(ptr);
        int enum_value = obj_val.unbox<int32_t>();

        std::string strName = *enum_key;
        if (FIX_ENUM) 
            strName = rt.GetName().AsString() + "_" + strName;

        out << indent << "    " << strName << " = " << enum_value << ";\n";
    }

    out << indent << "}\n\n";
}

static void GenerateMessageDefinition(CRuntimeType rt, std::stringstream& out, int indentLevel = 0)
{
    if (!rt)
        return;

    auto il2cppClass = rt.GetIl2CppType().get_class();
    uintptr_t descriptor_ptr = CallIl2CppStaticMethodInternal<uintptr_t>(il2cppClass, "get_Descriptor", {});
    CIl2CppObject descriptor(descriptor_ptr);

    std::string indent = GetIndent(indentLevel);
    std::string className = rt.GetName();
    std::string nsp = rt.GetNamespace();
    std::string assemblyName = rt.GetAssemblyName();

    out << indent << "// Class: " << className << ", Namespaze: " << nsp << ", Assembly: " << assemblyName << "\n";

    // 解析 descriptor 生成 proto
    GenerateMessageDefinitionByDescriptor(descriptor, out, indentLevel);
}

// 主函数
void DumpProtos2(CMonoAssembly mono_assembly, const char* path)
{
    std::filesystem::path filePath(path);
    std::filesystem::path directory = filePath.parent_path();
    if (!std::filesystem::exists(directory)) {
        std::filesystem::create_directories(directory);
    }

    auto gpb = HasGoogleProtobuf();

    if (!gpb.has_value())
    {
        DebugPrintA("[ProtoDump] Google.Protobuf not found, skip proto dump\n");
        return;
    }

    std::ofstream file(path);
    if (file.is_open()) {
        auto messages = GetAllProtobufMessages(mono_assembly, gpb.value());
        auto enums = GetAllProtobufEnums(mono_assembly);
        DebugPrintA("enums count: %d\n", enums.size());
        DebugPrintA("messages count: %d\n", messages.size());

        std::stringstream ss;
        ss << "// CerydarDumper\n\n";
        ss << "syntax = \"proto3\";\n\n";

        for (auto e : enums) {
            auto enumName = e.GetName().AsString();
            auto enumNsp = e.GetNamespace().AsString();
            DebugPrintA("messagesEnum: %s.%s\n", enumNsp.c_str(), enumName.c_str());
            GenerateEnumDefinition(e, ss);
        }

        for (auto m : messages) {
            auto enumName = m.GetName().AsString();
            auto enumNsp = m.GetNamespace().AsString();
            DebugPrintA("[ProtoDump] Message: %s.%s\n", enumNsp.c_str(), enumName.c_str());
           GenerateMessageDefinition(m, ss);
        }

        file << ss.str();
        file.close();
        DebugPrintA("[ProtoDump] dump done!\n");
    }
    else
    {
        DebugPrintA("[ProtoDump] [ERROR] Failed to open file for writing: %s\n", path);
    }
}
