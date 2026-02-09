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

static bool HasGoogleProtobuf()
{
    Il2CppDomain* domain = il2cpp_domain_get();
    if (!domain)
        return false;

    size_t assemblyCount = 0;
    const Il2CppAssembly** assemblies = il2cpp_domain_get_assemblies(domain, &assemblyCount);

    for (size_t i = 0; i < assemblyCount; ++i)
    {
        const Il2CppImage* image = il2cpp_assembly_get_image(assemblies[i]);
        if (!image)
            continue;

        auto* imageName = il2cpp_image_get_name(image);

        size_t classCount = il2cpp_image_get_class_count(image);
        for (size_t c = 0; c < classCount; ++c)
        {
            Il2CppClass* klass = const_cast<Il2CppClass*>(il2cpp_image_get_class(image, c));
            auto* clsName = il2cpp_class_get_name(klass);
            const char* nsp = il2cpp_class_get_namespace(klass);
            if (!klass || !nsp)
                continue;

            if (strncmp(nsp, "Google.Protobuf", 15) == 0)
            {
                DebugPrintA("[ProtoDump] Found Google.Protobuf namespace in %s (%s.%s)\n", imageName, nsp, clsName);
                return true;
            }
        }
    }

    return false;
}

static Il2CppClass* FindClass(const char* imageName, const char* namespaze, const char* className)
{
    if (!className)
        return nullptr;

    Il2CppDomain* domain = il2cpp_domain_get();
    if (!domain)
        return nullptr;

    size_t assemblyCount = 0;
    const Il2CppAssembly** assemblies = il2cpp_domain_get_assemblies(domain, &assemblyCount);

    for (size_t i = 0; i < assemblyCount; ++i)
    {
        const Il2CppImage* image = il2cpp_assembly_get_image(assemblies[i]);
        auto* imageNa = il2cpp_image_get_name(image);
        if (!image || !imageNa)
            continue;

        if (imageName && strcmp(imageNa, imageName) != 0)
            continue;

        size_t classCount = il2cpp_image_get_class_count(image);
        for (size_t c = 0; c < classCount; ++c)
        {
            Il2CppClass* klass = const_cast<Il2CppClass*>(il2cpp_image_get_class(image, c));
            auto* klassName = il2cpp_class_get_name(klass);
            auto* klassNsp = il2cpp_class_get_namespace(klass);
            if (!klass)
                continue;

            if (strcmp(klassName, className) != 0)
                continue;

            if (namespaze)
            {
                if (!klassNsp || strcmp(klassNsp, namespaze) != 0)
                    continue;
            }

            return klass;
        }
    }

    return nullptr;
}

static void InitProtobufAttributes()
{
    if (!_originalNameAttr)
        _originalNameAttr = FindClass(nullptr, "Google.Protobuf.Reflection", "OriginalNameAttribute");

    if (!_originalNameAttr)
        DebugPrintA("[ProtoDump] OriginalNameAttribute not found\n");
}

static bool IsInGoogleProtobufNamespace(Il2CppClass* klass)
{
    const char* ns = il2cpp_class_get_namespace(klass);
    if (!ns || !*ns)
        return false;

    return strcmp(ns, "Google.Protobuf") == 0 || strncmp(ns, "Google.Protobuf.", 16) == 0;
}

static bool IsProtobufMessage(Il2CppClass* klass)
{
    if (!klass) return false;

    if (IsInGoogleProtobufNamespace(klass)) return false;
    auto* klassEnumType = il2cpp_class_enum_basetype(klass);
    if (il2cpp_class_is_valuetype(klass) || klassEnumType) return false;

    void* iter = nullptr;
    Il2CppClass* iface = nullptr;

    while ((iface = il2cpp_class_get_interfaces(klass, &iter)))
    {
        auto* ifaceNsp = il2cpp_class_get_namespace(iface);
        auto* ifaceClsName = il2cpp_class_get_name(iface);
        if (!iface || !ifaceNsp || !ifaceClsName)
            continue;

        if (strcmp(ifaceNsp, "Google.Protobuf") != 0)
            continue;

        if (strncmp(ifaceClsName, "IMessage", 8) == 0)
            return true;
    }

    return false;
}

static bool IsProtobufEnum(Il2CppClass* klass)
{
    if (!il2cpp_class_is_enum(klass))
        return false;

    if (il2cpp_class_get_declaring_type(klass) != nullptr)
        return false;

    if (IsInGoogleProtobufNamespace(klass))
        return false;

    void* iter = nullptr;
    FieldInfo* field = nullptr;

    while ((field = il2cpp_class_get_fields(klass, &iter)) != nullptr)
    {
        if (!il2cpp_field_is_literal(field))
            continue;

        if (il2cpp_field_has_attribute(field, _originalNameAttr))
            return true;
    }

    return false;
}

static std::vector<CRuntimeType> GetAllProtobufMessages(CMonoAssembly monoAssembly)
{
    auto domain = CAppDomain::GetCurrentDomain();
    auto assemblies = domain.GetAssemblies().to_vec<CMonoAssembly>();
    CMonoAssembly protobuf_lib;
    std::vector<CRuntimeType> messages;

    // 找到 Google.Protobuf 的程序集
    for (size_t i = 0; i < assemblies.size(); ++i) {
        CMonoAssembly mono_assembly(assemblies[i]);
        std::string assembly_name = mono_assembly.GetName().AsString();
        if (assembly_name.rfind("Google.Protobuf", 0) == 0) {
            protobuf_lib = mono_assembly;
            break;
        }
    }

    auto _iMessageType = protobuf_lib.GetTypeByName("Google.Protobuf.IMessage");

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

    //while ((field = il2cpp_class_get_fields(enumClass, &iter)) != nullptr)
    //{
    //    auto* fieldName = il2cpp_field_get_name(field);

    //    // 枚举的 value__ 字段不要
    //    if (strcmp(fieldName, "value__") == 0)
    //        continue;

    //    // 只处理字面量字段
    //    if (!il2cpp_field_is_literal(field))
    //        continue;

    //    int64_t fieldValue = 0;
    //    il2cpp_field_static_get_value(field, &fieldValue);

    //    if (fieldValue == value)
    //    {
    //        return fieldName;
    //    }
    //}

    //// 找不到匹配值就返回 unknown
    //return "";
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
        //const MethodInfo* getEnumType = il2cpp_class_get_method_from_name(fieldClass, "get_EnumType", 0);
        //if (!getEnumType) return "unknown_enum";

        //Il2CppException* exc = nullptr;
        //Il2CppObject* enumDescriptor = il2cpp_runtime_invoke(getEnumType, field, nullptr, &exc);
        //if (exc || !enumDescriptor) return "unknown_enum";

        // 调用 EnumDescriptor.get_Name()
        //Il2CppClass* enumDescClass = il2cpp_object_get_class(enumDescriptor);
        //const MethodInfo* getNameMethod = il2cpp_class_get_method_from_name(enumDescClass, "get_Name", 0);
        //if (!getNameMethod) return "unknown_enum";

        //exc = nullptr;
        //Il2CppObject* nameObj = il2cpp_runtime_invoke(getNameMethod, enumDescriptor, nullptr, &exc);
        //if (exc || !nameObj) return "unknown_enum";

        //return Il2CppStringToUtf8String((Il2CppString*)nameObj);
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

        //// 获取 MessageDescriptor
        //const MethodInfo* getMessageType = il2cpp_class_get_method_from_name(fieldClass, "get_MessageType", 0);
        //if (!getMessageType) return "unknown_message";

        //Il2CppException* exc = nullptr;
        //Il2CppObject* msgDescriptor = il2cpp_runtime_invoke(getMessageType, field, nullptr, &exc);
        //if (exc || !msgDescriptor) return "unknown_message";

        //// 调用 MessageDescriptor.get_Name()
        //Il2CppClass* msgDescClass = il2cpp_object_get_class(msgDescriptor);
        //const MethodInfo* getNameMethod = il2cpp_class_get_method_from_name(msgDescClass, "get_Name", 0);
        //if (!getNameMethod) return "unknown_message";

        //exc = nullptr;
        //Il2CppObject* nameObj = il2cpp_runtime_invoke(getNameMethod, msgDescriptor, nullptr, &exc);
        //if (exc || !nameObj) return "unknown_message";

        //return Il2CppStringToUtf8String((Il2CppString*)nameObj);
    }

    //Il2CppObject* fieldTypeValue = nullptr;
    //{
    //    void* iter = nullptr;
    //    const MethodInfo* method = nullptr;
    //    while ((method = il2cpp_class_get_methods(fieldClass, &iter)) != nullptr)
    //    {
    //        const char* name = il2cpp_method_get_name(method);
    //        if (!name) continue;
    //        if (strcmp(name, "get_FieldType") != 0) continue;
    //        if (!il2cpp_method_is_instance(method)) continue;
    //        if (il2cpp_method_get_param_count(method) != 0) continue;

    //        Il2CppException* exc = nullptr;
    //        fieldTypeValue = il2cpp_runtime_invoke(method, field, nullptr, &exc);
    //        if (exc || !fieldTypeValue)
    //            return "unknown";
    //        break;
    //    }
    //}

    //if (!fieldTypeValue)
    //    return "unknown";

    //// 获取 FieldType 枚举名称
    //Il2CppClass* fieldTypeEnumClass = FindClass(nullptr, "Google.Protobuf.Reflection", "FieldType");
    //if (!fieldTypeEnumClass)
    //    return "unknown";

    //int fieldTypeInt = *(int*)il2cpp_object_unbox(fieldTypeValue);
    //std::string fieldTypeName = EnumToString(fieldTypeEnumClass, fieldTypeInt);
    //if (fieldTypeName.empty())
    //    return "unknown";

    //for (auto& c : fieldTypeName) c = tolower(c); // 转小写
    //std::string FieldType = fieldTypeName;

    //// 处理基本类型
    //if (FieldType == "int32") return "int32";
    //if (FieldType == "int64") return "int64";
    //if (FieldType == "uint32") return "uint32";
    //if (FieldType == "uint64") return "uint64";
    //if (FieldType == "sint32") return "sint32";
    //if (FieldType == "sint64") return "sint64";
    //if (FieldType == "fixed32") return "fixed32";
    //if (FieldType == "fixed64") return "fixed64";
    //if (FieldType == "sfixed32") return "sfixed32";
    //if (FieldType == "sfixed64") return "sfixed64";
    //if (FieldType == "float") return "float";
    //if (FieldType == "double") return "double";
    //if (FieldType == "bool") return "bool";
    //if (FieldType == "string") return "string";
    //if (FieldType == "bytes") return "bytes";

    // enum 类型
    //if (FieldType == "enum")
    //{
    //    // 获取 EnumDescriptor
    //    const MethodInfo* getEnumType = il2cpp_class_get_method_from_name(fieldClass, "get_EnumType", 0);
    //    if (!getEnumType) return "unknown_enum";

    //    Il2CppException* exc = nullptr;
    //    Il2CppObject* enumDescriptor = il2cpp_runtime_invoke(getEnumType, field, nullptr, &exc);
    //    if (exc || !enumDescriptor) return "unknown_enum";

    //    // 调用 EnumDescriptor.get_Name()
    //    Il2CppClass* enumDescClass = il2cpp_object_get_class(enumDescriptor);
    //    const MethodInfo* getNameMethod = il2cpp_class_get_method_from_name(enumDescClass, "get_Name", 0);
    //    if (!getNameMethod) return "unknown_enum";

    //    exc = nullptr;
    //    Il2CppObject* nameObj = il2cpp_runtime_invoke(getNameMethod, enumDescriptor, nullptr, &exc);
    //    if (exc || !nameObj) return "unknown_enum";

    //    return Il2CppStringToUtf8String((Il2CppString*)nameObj);
    //}

    //if (FieldType == "message")
    //{
    //    // 获取 MessageDescriptor
    //    const MethodInfo* getMessageType = il2cpp_class_get_method_from_name(fieldClass, "get_MessageType", 0);
    //    if (!getMessageType) return "unknown_message";

    //    Il2CppException* exc = nullptr;
    //    Il2CppObject* msgDescriptor = il2cpp_runtime_invoke(getMessageType, field, nullptr, &exc);
    //    if (exc || !msgDescriptor) return "unknown_message";

    //    // 调用 MessageDescriptor.get_Name()
    //    Il2CppClass* msgDescClass = il2cpp_object_get_class(msgDescriptor);
    //    const MethodInfo* getNameMethod = il2cpp_class_get_method_from_name(msgDescClass, "get_Name", 0);
    //    if (!getNameMethod) return "unknown_message";

    //    exc = nullptr;
    //    Il2CppObject* nameObj = il2cpp_runtime_invoke(getNameMethod, msgDescriptor, nullptr, &exc);
    //    if (exc || !nameObj) return "unknown_message";

    //    return Il2CppStringToUtf8String((Il2CppString*)nameObj);
    //}

    return FieldType;
}

// 获取 protobuf 字段名字
static std::string GetFieldName(Il2CppObject* field)
{
    if (!field)
        return "unknown";

    Il2CppClass* klass = il2cpp_object_get_class(field);
    if (!klass)
        return "unknown";

    // 找 Name 属性
    const MethodInfo* getNameMethod = il2cpp_class_get_method_from_name(klass, "get_Name", 0);
    if (!getNameMethod)
        return "unknown";

    Il2CppException* exc = nullptr;
    Il2CppObject* result = il2cpp_runtime_invoke(getNameMethod, field, nullptr, &exc);
    if (exc || !result)
        return "unknown";

    return Il2CppStringToUtf8String((Il2CppString*)result);
}

// 获取 protobuf 字段编号
static int GetFieldNumber(Il2CppObject* field)
{
    if (!field)
        return 0;

    Il2CppClass* klass = il2cpp_object_get_class(field);
    if (!klass)
        return 0;

    // 找 FieldNumber 属性
    const MethodInfo* getNumberMethod = il2cpp_class_get_method_from_name(klass, "get_FieldNumber", 0);
    if (!getNumberMethod)
        return 0;

    Il2CppException* exc = nullptr;
    Il2CppObject* result = il2cpp_runtime_invoke(getNumberMethod, field, nullptr, &exc);
    if (exc || !result)
        return 0;

    // 枚举值一般是 int
    return *(int*)il2cpp_object_unbox(result);
}
//
//static Il2CppObject* GetFieldsCollection(Il2CppObject* messageDescriptor)
//{
//    if (!messageDescriptor) return nullptr;
//
//    Il2CppClass* descriptorClass = il2cpp_object_get_class(messageDescriptor);
//    if (!descriptorClass) return nullptr;
//
//    // 获取 get_Fields 方法
//    const MethodInfo* getFieldsMethod = il2cpp_class_get_method_from_name(descriptorClass, "get_Fields", 0);
//    if (!getFieldsMethod)
//    {
//        DebugPrintA("[ProtoDump] get_Fields method not found for %s\n", il2cpp_class_get_name(descriptorClass));
//        return nullptr;
//    }
//
//    Il2CppException* exc = nullptr;
//    Il2CppObject* fieldsCollection = il2cpp_runtime_invoke(getFieldsMethod, messageDescriptor, nullptr, &exc);
//
//    if (exc)
//    {
//        DebugPrintA("[ProtoDump] Exception when calling get_Fields\n");
//        return nullptr;
//    }
//
//    if (!fieldsCollection)
//    {
//        DebugPrintA("[ProtoDump] get_Fields returned null\n");
//        return nullptr;
//    }
//
//    //DebugPrintA("[ProtoDump] Successfully got FieldCollection for %s\n", il2cpp_class_get_name(descriptorClass));
//    return fieldsCollection;
//}

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
    //if (!fieldCollection)
    //    return fields;

    //Il2CppClass* fcClass = il2cpp_object_get_class(fieldCollection);
    //if (!fcClass)
    //    return fields;

    //// FieldCollection.InDeclarationOrder() 方法返回 IList<FieldDescriptor>
    //const MethodInfo* inDeclOrderMethod = il2cpp_class_get_method_from_name(fcClass, "InDeclarationOrder", 0);
    //if (!inDeclOrderMethod)
    //{
    //    DebugPrintA("[ProtoDump] InDeclarationOrder method not found\n");
    //    return fields;
    //}

    //Il2CppException* exc = nullptr;
    //Il2CppObject* fieldListObj = il2cpp_runtime_invoke(inDeclOrderMethod, fieldCollection, nullptr, &exc);
    //if (exc || !fieldListObj)
    //{
    //    DebugPrintA("[ProtoDump] Failed to get fields in declaration order\n");
    //    return fields;
    //}

 /*   Il2CppClass* listClass = il2cpp_object_get_class(fieldListObj);
    if (!listClass)
        return fields;

    const MethodInfo* getItemMethod = il2cpp_class_get_method_from_name(listClass, "get_Item", 1);
    if (!getItemMethod)
    {
        DebugPrintA("[ProtoDump] IList get_Item method not found\n");
        return fields;
    }

    const MethodInfo* countMethod = il2cpp_class_get_method_from_name(listClass, "get_Count", 0);
    if (!countMethod)
    {
        DebugPrintA("[ProtoDump] IList get_Count method not found\n");
        return fields;
    }

    exc = nullptr;
    Il2CppObject* countObj = il2cpp_runtime_invoke(countMethod, fieldListObj, nullptr, &exc);
    if (exc || !countObj)
        return fields;

    int count = *(int*)il2cpp_object_unbox(countObj);

    for (int i = 0; i < count; ++i)
    {
        void* args[1] = { &i };
        Il2CppException* excField = nullptr;
        Il2CppObject* field = il2cpp_runtime_invoke(getItemMethod, fieldListObj, args, &excField);
        if (!excField && field)
        {
            fields.push_back(field);
        }
    }*/

    return fields;
}

static Il2CppObject* GetMessageDescriptor(Il2CppClass* messageClass)
{
    void* iter = nullptr;
    const MethodInfo* method = nullptr;

    while ((method = il2cpp_class_get_methods(messageClass, &iter)) != nullptr)
    {
        const char* name = il2cpp_method_get_name(method);
        if (!name || strcmp(name, "get_Descriptor") != 0)
            continue;

        if (il2cpp_method_is_instance(method)) // must be static
            continue;

        if (il2cpp_method_get_param_count(method) != 0)
            continue;

        Il2CppException* exc = nullptr;
        Il2CppObject* descriptor = il2cpp_runtime_invoke(method, nullptr, nullptr, &exc);
        if (exc) return nullptr;

        return descriptor;
    }

    return nullptr;
}

static std::string GetDescriptorName(Il2CppObject* descriptor)
{
    if (!descriptor) return "";

    Il2CppClass* klass = il2cpp_object_get_class(descriptor);

    void* iter = nullptr;
    const MethodInfo* method = nullptr;

    while ((method = il2cpp_class_get_methods(klass, &iter)) != nullptr)
    {
        const char* name = il2cpp_method_get_name(method);
        if (!name || strcmp(name, "get_Name") != 0)
            continue;

        if (!il2cpp_method_is_instance(method))
            continue;

        if (il2cpp_method_get_param_count(method) != 0)
            continue;

        Il2CppException* exc = nullptr;
        Il2CppObject* result = il2cpp_runtime_invoke(method, descriptor, nullptr, &exc);
        if (exc || !result)
            return "";

        return Il2CppStringToUtf8String((Il2CppString*)result);
    }

    return "";
}

static std::string GetFieldDefinition(CIl2CppObject field)
{
    if (!field) return "";

    //std::string fieldName = GetFieldName(field);
    //int fieldNumber = GetFieldNumber(field);

    auto sysstrFieldName = CallIl2CppInstanceObjectMethodDynamic<CSystemString>(field, "get_Name", {});
    auto fieldName = sysstrFieldName.AsString();

    int fieldNumber = CallIl2CppInstanceObjectMethodDynamic<int>(field, "get_FieldNumber", {});

    // 判断是否 Map
    //CIl2CppClass fieldClass = field.get_class();
    //const MethodInfo* isMapMethod = il2cpp_class_get_method_from_name(fieldClass, "get_IsMap", 0);
    //bool isMap = false;

    auto isMap = CallIl2CppInstanceObjectMethodDynamic<bool>(field, "get_IsMap", {});

    //if (isMapMethod)
    //{
    //    Il2CppException* exc = nullptr;
    //    Il2CppObject* res = il2cpp_runtime_invoke(isMapMethod, field, nullptr, &exc);
    //    if (!exc && res)
    //        isMap = *(bool*)il2cpp_object_unbox(res);
    //}

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

        //// 获取 MessageType，即 MapEntryDescriptor
        //const MethodInfo* getMessageType = il2cpp_class_get_method_from_name(fieldClass, "get_MessageType", 0);
        //if (!getMessageType) return "// Error: Map MessageType missing";

        //Il2CppException* exc = nullptr;
        //Il2CppObject* mapEntryDescriptor = il2cpp_runtime_invoke(getMessageType, field, nullptr, &exc);
        //if (exc || !mapEntryDescriptor) return "// Error: Map MessageType invoke failed";

        //// 获取字段集合 FieldCollection
        //Il2CppObject* mapFieldsCollection = GetFieldsCollection(mapEntryDescriptor);
        //if (!mapFieldsCollection) return "// Error: Map FieldCollection missing";

        //auto mapFields = GetAllFields(mapEntryDescriptor);
        //if (mapFields.size() < 2)
        //    return "// Error: Map fields count != 2";

        //// 第一个是 key，第二个是 value
        //std::string keyType = GetFieldTypeString(mapFields[0]);
        //std::string valueType = GetFieldTypeString(mapFields[1]);

        //return "map<" + keyType + ", " + valueType + "> " + fieldName + " = " + std::to_string(fieldNumber) + ";";
    }
    else
    {
        // 判断是否 repeated
        auto isRepeated = CallIl2CppInstanceObjectMethodDynamic<bool>(field, "get_IsRepeated", {});
        //const MethodInfo* isRepeatedMethod = il2cpp_class_get_method_from_name(fieldClass, "get_IsRepeated", 0);
        //bool isRepeated = false;

        //if (isRepeatedMethod)
        //{
        //    Il2CppException* exc = nullptr;
        //    Il2CppObject* res = il2cpp_runtime_invoke(isRepeatedMethod, field, nullptr, &exc);
        //    if (!exc && res)
        //        isRepeated = *(bool*)il2cpp_object_unbox(res);
        //}

        std::string typeStr = GetFieldTypeString(field);
        std::string repeatedStr = isRepeated ? "repeated " : "";

        return repeatedStr + typeStr + " " + fieldName + " = " + std::to_string(fieldNumber) + ";";
    }
}

static std::string GetIndent(int indentLevel)
{
    return std::string(indentLevel * 4, ' ');
}

static std::vector<Il2CppObject*> GetAllNestedTypes(Il2CppObject* messageDescriptor)
{
    std::vector<Il2CppObject*> result;
    if (!messageDescriptor) return result;

    Il2CppClass* descClass = il2cpp_object_get_class(messageDescriptor);
    if (!descClass) return result;

    const MethodInfo* getNestedMethod = il2cpp_class_get_method_from_name(descClass, "get_NestedTypes", 0);
    if (!getNestedMethod) return result;

    Il2CppException* exc = nullptr;
    Il2CppObject* listObj = il2cpp_runtime_invoke(getNestedMethod, messageDescriptor, nullptr, &exc);
    if (exc || !listObj) return result;

    // listObj 是 IList<MessageDescriptor>
    Il2CppClass* listClass = il2cpp_object_get_class(listObj);
    const MethodInfo* countMethod = il2cpp_class_get_method_from_name(listClass, "get_Count", 0);
    const MethodInfo* itemMethod = il2cpp_class_get_method_from_name(listClass, "get_Item", 1);
    if (!countMethod || !itemMethod) return result;

    exc = nullptr;
    Il2CppObject* countObj = il2cpp_runtime_invoke(countMethod, listObj, nullptr, &exc);
    if (exc || !countObj) return result;
    int count = *(int*)il2cpp_object_unbox(countObj);

    for (int i = 0; i < count; ++i)
    {
        void* args[1] = { &i };
        Il2CppException* excField = nullptr;
        Il2CppObject* nested = il2cpp_runtime_invoke(itemMethod, listObj, args, &excField);
        if (!excField && nested)
            result.push_back(nested);
    }

    return result;
}

static std::vector<Il2CppObject*> GetAllEnumTypes(Il2CppObject* messageDescriptor)
{
    std::vector<Il2CppObject*> result;
    if (!messageDescriptor) return result;

    Il2CppClass* descClass = il2cpp_object_get_class(messageDescriptor);
    if (!descClass) return result;

    const MethodInfo* getEnumMethod = il2cpp_class_get_method_from_name(descClass, "get_EnumTypes", 0);
    if (!getEnumMethod) return result;

    Il2CppException* exc = nullptr;
    Il2CppObject* listObj = il2cpp_runtime_invoke(getEnumMethod, messageDescriptor, nullptr, &exc);
    if (exc || !listObj) return result;

    // listObj 是 IList<EnumDescriptor>
    Il2CppClass* listClass = il2cpp_object_get_class(listObj);
    const MethodInfo* countMethod = il2cpp_class_get_method_from_name(listClass, "get_Count", 0);
    const MethodInfo* itemMethod = il2cpp_class_get_method_from_name(listClass, "get_Item", 1);
    if (!countMethod || !itemMethod) return result;

    exc = nullptr;
    Il2CppObject* countObj = il2cpp_runtime_invoke(countMethod, listObj, nullptr, &exc);
    if (exc || !countObj) return result;
    int count = *(int*)il2cpp_object_unbox(countObj);

    for (int i = 0; i < count; ++i)
    {
        void* args[1] = { &i };
        Il2CppException* excField = nullptr;
        Il2CppObject* e = il2cpp_runtime_invoke(itemMethod, listObj, args, &excField);
        if (!excField && e)
            result.push_back(e);
    }

    return result;
}

static void GenerateEnumDefinitionByDescriptor(CIl2CppObject enumDescriptor, std::stringstream& out, int indentLevel = 0)
{
    if (!enumDescriptor)
        return;

    out << "\n";
    std::string indent = GetIndent(indentLevel);

    //Il2CppClass* descClass = il2cpp_object_get_class(enumDescriptor);
    //if (!descClass) return;

    // 获取 Name
    std::string enumName = CallIl2CppInstanceObjectMethodDynamic<CSystemString>(enumDescriptor, "get_Name", {});
    //const MethodInfo* getNameMethod = il2cpp_class_get_method_from_name(descClass, "get_Name", 0);
    //if (!getNameMethod) return;

    //Il2CppException* exc = nullptr;
    //Il2CppObject* nameObj = il2cpp_runtime_invoke(getNameMethod, enumDescriptor, nullptr, &exc);
    //if (exc || !nameObj) return;

    //std::string enumName = Il2CppStringToUtf8String((Il2CppString*)nameObj);

    // 输出注释
    out << indent << "// Enum Descriptor Generation\n";
    out << indent << "enum " << enumName << " {\n";

    // 获取 Values
    auto valueListObj = CallIl2CppInstanceObjectMethodDynamic<CNativeList>(enumDescriptor, "get_Values", {});
    auto valueArrayObj = valueListObj.Items();
    auto count = valueArrayObj.length();

    for (int i = 0; i < count; ++i)
    {
        uintptr_t obj_ptr = valueArrayObj.get<uintptr_t>(i);
        //CIl2CppObject obj(obj_ptr);
        //auto e = CallIl2CppInstanceObjectMethodDynamic<CIl2CppObject>(listObj, "get_Item", {"int"}, i);
 

        //auto val = CallIl2CppInstanceObjectMethodDynamic<CIl2CppObject>(valueListObj, "get_Item", {"int"}, i);


        //void* args[1] = { &i };
        //Il2CppException* excValue = nullptr;
        //Il2CppObject* val = il2cpp_runtime_invoke(itemMethod, valueListObj, args, &excValue);
        if (obj_ptr)
        {
            // FieldDescriptor 的 Name 和 Number
            //Il2CppClass* valClass = il2cpp_object_get_class(val);
            //const MethodInfo* getFieldName = il2cpp_class_get_method_from_name(valClass, "get_Name", 0);
            //const MethodInfo* getFieldNumber = il2cpp_class_get_method_from_name(valClass, "get_Number", 0);
            //if (!getFieldName || !getFieldNumber) continue;

            std::string sName = CallIl2CppInstanceObjectMethodDynamic<CSystemString>(obj_ptr, "get_Name", {});
            auto iNum = CallIl2CppInstanceObjectMethodDynamic<int>(obj_ptr, "get_Number", {});

            out << indent << "    " << sName << " = " << iNum << ";\n";
        }
    }

    //const MethodInfo* getValuesMethod = il2cpp_class_get_method_from_name(descClass, "get_Values", 0);
    //if (!getValuesMethod) return;

    //exc = nullptr;
    //Il2CppObject* valueListObj = il2cpp_runtime_invoke(getValuesMethod, enumDescriptor, nullptr, &exc);
    //if (exc || !valueListObj) return;

    //Il2CppClass* listClass = il2cpp_object_get_class(valueListObj);
    //const MethodInfo* countMethod = il2cpp_class_get_method_from_name(listClass, "get_Count", 0);
    //const MethodInfo* itemMethod = il2cpp_class_get_method_from_name(listClass, "get_Item", 1);
    //if (!countMethod || !itemMethod) return;

    //exc = nullptr;
    //Il2CppObject* countObj = il2cpp_runtime_invoke(countMethod, valueListObj, nullptr, &exc);
    //if (exc || !countObj) return;

    //int count = *(int*)il2cpp_object_unbox(countObj);

    //for (int i = 0; i < count; ++i)
    //{
    //    void* args[1] = { &i };
    //    Il2CppException* excValue = nullptr;
    //    Il2CppObject* val = il2cpp_runtime_invoke(itemMethod, valueListObj, args, &excValue);
    //    if (!excValue && val)
    //    {
    //        // FieldDescriptor 的 Name 和 Number
    //        Il2CppClass* valClass = il2cpp_object_get_class(val);
    //        const MethodInfo* getFieldName = il2cpp_class_get_method_from_name(valClass, "get_Name", 0);
    //        const MethodInfo* getFieldNumber = il2cpp_class_get_method_from_name(valClass, "get_Number", 0);
    //        if (!getFieldName || !getFieldNumber) continue;

    //        Il2CppObject* fNameObj = il2cpp_runtime_invoke(getFieldName, val, nullptr, &excValue);
    //        Il2CppObject* fNumObj = il2cpp_runtime_invoke(getFieldNumber, val, nullptr, &excValue);
    //        if (excValue || !fNameObj || !fNumObj) continue;

    //        std::string fieldName = Il2CppStringToUtf8String((Il2CppString*)fNameObj);
    //        int fieldNumber = *(int*)il2cpp_object_unbox(fNumObj);

    //        out << indent << "    " << fieldName << " = " << fieldNumber << ";\n";
    //    }
    //}

    out << indent << "}\n\n";
}

static void GenerateMessageDefinitionByDescriptor(CIl2CppObject descriptor, std::stringstream& out, int indentLevel = 0)
{
    if (!descriptor) return;

    // 获取 Message 名称
    //std::string name = GetDescriptorName(descriptor);
    auto klass = descriptor.get_class();
    CSystemString systemString = CallIl2CppInstanceObjectMethodDynamic<CSystemString>(descriptor, "get_Name", {});
    std::string name = systemString;

    if (name.empty()) {
        DebugPrintA("[ProtoDump] Message descriptor has no name\n");
        out << "// Error: Message descriptor has no name\n";
        return;
    }

    // 过滤 MapEntry 消息
    if (name.size() >= 5 && std::equal(name.end() - 5, name.end(), L"Entry"))
        return;

    out << "\n";
    std::string indent = GetIndent(indentLevel);

    out << indent << "// Message Descriptor Generation\n";
    out << indent << "message " << name << " {\n";

    // oneof + 普通字段
    auto fields = GetAllFields(descriptor);
    std::unordered_map<CIl2CppObject*, std::vector<CIl2CppObject>> oneofGroups;

    for (auto field : fields)
    {
        //FieldCollection

        auto name = field.get_class().name();

        CIl2CppObject containingOneof = CallIl2CppInstanceObjectMethodDynamic<CIl2CppObject>(field, "get_ContainingOneof", {});

        /*const MethodInfo* getOneof = il2cpp_class_get_method_from_name(il2cpp_object_get_class(field), "get_ContainingOneof", 0);
        if (getOneof)
        {
            Il2CppException* exc = nullptr;
            containingOneof = il2cpp_runtime_invoke(getOneof, field, nullptr, &exc);
        }*/
        if (containingOneof)
            oneofGroups[&containingOneof].push_back(field);
    }

    for (auto& [oneofDesc, fieldList] : oneofGroups)
    {
        auto cls = oneofDesc->get_class();
        auto sysstrOneofName = CallIl2CppInstanceObjectMethodDynamic<CSystemString>(*oneofDesc, "get_Name", {});
        auto oneofName = sysstrOneofName.AsString();

        //const MethodInfo* getName = il2cpp_class_get_method_from_name(il2cpp_object_get_class(oneofDesc), "get_Name", 0);
        //std::string oneofName = "unknown_oneof";
        //if (getName)
        //{
        //    Il2CppException* exc = nullptr;
        //    Il2CppObject* nameObj = il2cpp_runtime_invoke(getName, oneofDesc, nullptr, &exc);
        //    if (!exc && nameObj)
        //        oneofName = Il2CppStringToUtf8String((Il2CppString*)nameObj);
        //}

        out << indent << "    oneof " << oneofName << " {\n";
        for (auto field : fieldList)
            out << indent << "        " << GetFieldDefinition(field) << "\n";
        out << indent << "    }\n\n";
    }

    for (auto field : fields)
    {
        CIl2CppObject containingOneof = CallIl2CppInstanceObjectMethodDynamic<CIl2CppObject>(field, "get_ContainingOneof", {});

        if (containingOneof) continue;

        out << indent << "    " << GetFieldDefinition(field) << "\n";
    }

    //// 嵌套枚举
    std::vector<CIl2CppObject> nestedEnums;
    auto enumListObj = CallIl2CppInstanceObjectMethodDynamic<CNativeList>(descriptor, "get_EnumTypes", {});
    //int count = CallIl2CppInstanceObjectMethodDynamic<int>(listObj, "get_Count", {});
    auto enumArrayObj = enumListObj.Items();
    auto enumArrayCount = enumArrayObj.length();
    for (int i = 0; i < enumArrayCount; ++i)
    {
        //void* args[1] = { &i };
        //Il2CppException* excField = nullptr;
        uintptr_t obj_ptr = enumArrayObj.get<uintptr_t>(i);
        CIl2CppObject obj(obj_ptr);
        //auto e = CallIl2CppInstanceObjectMethodDynamic<CIl2CppObject>(listObj, "get_Item", {"int"}, i);
        if (obj)
            nestedEnums.push_back(obj);
    }

    //auto nestedEnums = GetAllEnumTypes(descriptor);
    for (auto e : nestedEnums)
        GenerateEnumDefinitionByDescriptor(e, out, indentLevel + 1);

    // 嵌套消息
    std::vector<CIl2CppObject> nestedMessages;
    auto messageListObj = CallIl2CppInstanceObjectMethodDynamic<CNativeList>(descriptor, "get_NestedTypes", {});
    //int count = CallIl2CppInstanceObjectMethodDynamic<int>(listObj, "get_Count", {});
    auto messageArrayObj = messageListObj.Items();
    auto messageArrayCount = messageArrayObj.length();
    for (int i = 0; i < messageArrayCount; ++i)
    {
        //void* args[1] = { &i };
        //Il2CppException* excField = nullptr;
        uintptr_t obj_ptr = messageArrayObj.get<uintptr_t>(i);
        CIl2CppObject obj(obj_ptr);
        //auto e = CallIl2CppInstanceObjectMethodDynamic<CIl2CppObject>(listObj, "get_Item", {"int"}, i);
        if (obj)
            nestedMessages.push_back(obj);
    }

    //auto nestedMessages = GetAllNestedTypes(descriptor);
    for (auto nested : nestedMessages)
        GenerateMessageDefinitionByDescriptor(nested, out, indentLevel + 1);

    out << indent << "}\n";
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

    //Il2CppObject* descriptor = GetMessageDescriptor(rt);
    //if (!descriptor) return;

    auto il2cppClass = rt.GetIl2CppType().get_class();
    //il2cppClass.find_method("get_Descriptor", {});
    uintptr_t descriptor_ptr = CallIl2CppStaticMethodInternal<uintptr_t>(il2cppClass, "get_Descriptor", {});
    CIl2CppObject descriptor(descriptor_ptr);

    //std::string className = il2cpp_class_get_name(messageClass);
    //std::string nsp = il2cpp_class_get_namespace(messageClass);
    //auto* clsImage = il2cpp_class_get_image(messageClass);
    //std::string image = il2cpp_image_get_name(clsImage);

    std::string indent = GetIndent(indentLevel);
    std::string className = rt.GetName();
    std::string nsp = rt.GetNamespace();
    //auto* clsImage = il2cpp_class_get_image(enumClass);
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

    if (!HasGoogleProtobuf())
    {
        DebugPrintA("[ProtoDump] Google.Protobuf not found, skip proto dump\n");
        return;
    }

    std::ofstream file(path);
    if (file.is_open()) {
        auto messages = GetAllProtobufMessages(mono_assembly);
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
