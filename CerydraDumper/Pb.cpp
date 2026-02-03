#include "pch.h"
#include "Pb.h"
#include <windows.h>
#include <fstream>
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
#include "CICollection.h"
#include <tuple>
#include <map>

struct Field {
    std::string Kind;   // e.g. int32, string
    std::string Name;
    uint32_t Number;

    Field(const std::string& kind, const std::string& name, uint32_t number)
        : Kind(kind), Name(name), Number(number) {
    }
};

struct Oneof {
    std::string Name;
    std::vector<Field> Fields;

    Oneof(const std::string& name) : Name(name) {}
};

struct Message {
    std::string Name;
    std::vector<Field> Fields;
    std::vector<Oneof> Oneofs;

    Message(const std::string& name) : Name(name) {}
};

struct Enum {
    std::string Name;
    std::vector<std::tuple<std::string, int>> Variants;

    Enum(const std::string& name) : Name(name) {}
};

struct ProtoFile {
    std::string Syntax = "proto3";
    std::vector<std::string> Imports;

    std::vector<Message> Messages;
    std::vector<Enum> Enums;
};


constexpr uint8_t WIRE_TYPE_VAR_INT = 0;
constexpr uint8_t WIRE_TYPE_I64 = 1;
constexpr uint8_t WIRE_TYPE_LENGTH_PREFIXED = 2;
constexpr uint8_t WIRE_TYPE_I32 = 5;

static int VarintLength(uint32_t v) {
    if (v == 0)
        return 1;

    int count = 0;
    while (v > 0) {
        count++;
        v >>= 7;
    }
    return count;
}

static int EncodeVarint(std::vector<uint8_t>& dst, uint32_t value) {
    const uint8_t MSB = 0x80;
    uint32_t n = value;
    int written = 0;

    while (n >= 0x80) {
        dst.push_back(static_cast<uint8_t>(MSB | (n & 0x7F)));
        n >>= 7;
        written++;
    }

    dst.push_back(static_cast<uint8_t>(n));
    written++;
    return written;
}

static int EncodeVarint(std::vector<uint8_t>& dst, uint64_t value) {
    const uint8_t MSB = 0x80;
    int written = 0;

    while (value >= 0x80) {
        dst.push_back(static_cast<uint8_t>((value & 0x7F) | MSB));
        value >>= 7;
        written++;
    }

    dst.push_back(static_cast<uint8_t>(value));
    written++;
    return written;
}

static uint32_t PackWireTag(uint32_t fieldId, uint8_t wireType) {
    return (fieldId << 3) | wireType;
}

static void EncodeZigZagVarint(std::vector<uint8_t>& buffer, int32_t value) {
    uint32_t zigzag = (static_cast<uint32_t>(value) << 1) ^ static_cast<uint32_t>(value >> 31);
    EncodeVarint(buffer, zigzag);
}

static void EncodeZigZagVarint(std::vector<uint8_t>& buffer, int64_t value) {
    uint64_t zigzag = (static_cast<uint64_t>(value) << 1) ^ static_cast<uint64_t>(value >> 63);
    EncodeVarint(buffer, zigzag);
}


struct OneofVariantInfo {
    int OneofEnumOffset;
    uintptr_t VariantType; // 使用 uintptr_t 代替 Type

    OneofVariantInfo(int oneofEnumOffset, uintptr_t variantType)
        : OneofEnumOffset(oneofEnumOffset), VariantType(variantType) {
    }
};

struct FieldMinimalInfo {
    int Tag;
    int Offset;
    bool IsZigZag = false;
    std::shared_ptr<OneofVariantInfo> OneofExtraData; // 可以为空

    FieldMinimalInfo(int tag, int offset,
        std::shared_ptr<OneofVariantInfo> oneofExtraData = nullptr)
        : Tag(tag), Offset(offset), OneofExtraData(oneofExtraData) {
    }
};

struct MessageMinimalInfo {
    std::string Name;
    std::vector<FieldMinimalInfo> Fields;

    MessageMinimalInfo(const std::string& name) : Name(name) {}
};

void* get_field_data_ptr(Il2CppObject* obj, uint32_t field_offset)
{
    return reinterpret_cast<uint8_t*>(obj) + field_offset;
}


template<typename ConverterType>
static ConverterType _GetFieldValue(CIl2CppClass cls, uintptr_t ptr, const char* fieldName) {
    if (!ptr)
        throw std::runtime_error(std::string("Object is null! Cannot access field ") + fieldName);

    auto obj_class = CRuntimeType::FromClass(cls);
    auto field_info = obj_class.GetField(fieldName);
    if (!field_info)
        throw std::runtime_error(std::string("No such field: ") + fieldName);

    auto value_obj = field_info.get_value_object(CIl2CppObject(ptr));
    if (!value_obj)
        throw std::runtime_error(std::string("Field ") + fieldName + " is null");

    return ConverterType(value_obj.raw_ptr());
}

// 获取字段值（考虑 collection / enum / object）
uintptr_t FetchValue(CMonoField field, uintptr_t obj) {
    auto val = field.GetValue(obj);

	auto il2cppClass = val.get_class();
    auto fieldType = field.GetFieldType();
    auto fieldName = field.GetName().AsString();
    auto fieldTypeName = fieldType.GetFullName().AsString();
    auto il2cppField = field.GetIl2CppField();
    auto il2cppFieldOffset = il2cppField.get_offset();
    auto il2cppFieldName = il2cppField.name();

    if (val.is_null()) {
        return 0;
    }

    uint8_t* addr = reinterpret_cast<uint8_t*>(val.raw_ptr()) + il2cppFieldOffset;

    // collection: 返回 Count
    if (fieldTypeName.find("System.Byte") != std::string::npos || fieldTypeName.find("System.SByte") != std::string::npos || fieldTypeName.find("System.Boolean") != std::string::npos)
    {
        //auto ptr = get_field_data_ptr(reinterpret_cast<Il2CppObject*>(il2cppField.raw_ptr()), il2cppFieldOffset);
        //uintptr_t value = static_cast<uintptr_t>(*reinterpret_cast<uint8_t*>(addr));
        auto value_obj = il2cppField.get_value_object(val);
        auto objv = CSystemDynamic(value_obj);
		auto str = objv.ToString().AsString();
        uint8_t value = std::stoul(str);
        //uintptr_t value = static_cast<uintptr_t>(_GetFieldValue<CSystemDynamic>(il2cppClass, val, fieldName.c_str()));

        return value;
    }
    else if (fieldTypeName.find("System.UInt16") != std::string::npos || fieldTypeName.find("System.Int16") != std::string::npos)
    {
        //return val.unbox<uint16_t>();

        //uintptr_t value = static_cast<uintptr_t>(*reinterpret_cast<uint16_t*>(addr));
        auto value_obj = il2cppField.get_value_object(val);
        auto objv = CSystemDynamic(value_obj);
        auto str = objv.ToString().AsString();
        uint16_t value = std::stoul(str);
        //uintptr_t value = static_cast<uintptr_t>(_GetFieldValue<CSystemDynamic>(il2cppClass, val, fieldName.c_str()));

        return value;
    }
    else if (fieldTypeName.find("System.UInt32") != std::string::npos || fieldTypeName.find("System.Int32") != std::string::npos || fieldTypeName.find("System.Single") != std::string::npos)
    {
        //return val.unbox<uint32_t>();

        //uintptr_t value = static_cast<uintptr_t>(*reinterpret_cast<uint32_t*>(addr));
        auto value_obj = il2cppField.get_value_object(val);
        auto objv = CSystemDynamic(value_obj);
        auto str = objv.ToString().AsString();
        uint32_t value = std::stoul(str);
        //uintptr_t value = static_cast<uintptr_t>(_GetFieldValue<CSystemDynamic>(il2cppClass, val, fieldName.c_str()));

        return value;

    }
    else if (fieldTypeName.find("System.UInt64") != std::string::npos || fieldTypeName.find("System.Int64") != std::string::npos || fieldTypeName.find("System.Double") != std::string::npos)
    {
        //return val.unbox<uint64_t>();

        //uintptr_t value = static_cast<uintptr_t>(*reinterpret_cast<uint64_t*>(addr));
        auto value_obj = il2cppField.get_value_object(val);
        auto objv = CSystemDynamic(value_obj);
        auto str = objv.ToString().AsString();
        uint64_t value = std::stoul(str);
        //uintptr_t value = static_cast<uintptr_t>(_GetFieldValue<CSystemDynamic>(il2cppClass, val, fieldName.c_str()));

        return value;
    }
    else if (fieldTypeName.find("System.String") != std::string::npos || fieldTypeName.find("System.Object") != std::string::npos || fieldTypeName.find("Google.Protobuf.ByteString") != std::string::npos || fieldTypeName.find("Google.Protobuf.WellKnownTypes.Any") != std::string::npos)
    {
        //return val.unbox<uintptr_t>();

        uintptr_t value = *reinterpret_cast<uintptr_t*>(addr);

        return value;
    }

    int genericArgumentsCount = fieldType.GetGenericArguments().length();
    if (genericArgumentsCount != 0) {
        uintptr_t value = *reinterpret_cast<uintptr_t*>(addr);
        //auto v = val.unbox<uintptr_t>();
        CICollection ico(value);
        uintptr_t count = ico.get_Count();
        return count;
    }

    if (fieldType.IsEnum()) {
        //return val.unbox<int>();

        //uintptr_t value = static_cast<uintptr_t>(*reinterpret_cast<uint32_t*>(addr));
        auto value_obj = il2cppField.get_value_object(val);
        auto objv = CSystemDynamic(value_obj);
        auto str = objv.ToString().AsString();
        uint32_t value = std::stoul(str);

        return value;
    }

    uintptr_t value = *reinterpret_cast<uintptr_t*>(addr);

    return value;
}

class CTrackedValues {
public:
    CTrackedValues(uintptr_t messagePtr, CRuntimeType type)
        : m_message(messagePtr)
    {
        CIl2CppArray fields = type.GetFields(52);

        for (int i = 0; i < fields.length(); i++) {
            CMonoField field = fields.get<CMonoField>(i);
            uintptr_t fieldPtr = field.raw_ptr();
            m_fields.push_back(fieldPtr);

            auto val = FetchValue(field, messagePtr);
            m_values[fieldPtr] = val;
        }
    }

    // 检测变化并更新，返回变化字段列表
    std::vector<uintptr_t> DetectChangesAndUpdate() {
        std::vector<uintptr_t> changedFields;
        std::unordered_map<uintptr_t, uintptr_t> updates;

        for (auto& kvp : m_values) {
            uintptr_t fieldPtr = kvp.first;
            uintptr_t oldValue = kvp.second;

			CMonoField field(fieldPtr);
            auto newValue = FetchValue(field, m_message);

            if (!ObjectEquals(oldValue, newValue)) {
                changedFields.push_back(fieldPtr);
                updates[fieldPtr] = newValue;
            }
        }

        // 更新字典
        for (auto& kvp : updates) {
            m_values[kvp.first] = kvp.second;
        }

        return changedFields;
    }

    // 获取保存的值
    uintptr_t GetSavedValue(uintptr_t fieldPtr) {
        return m_values[fieldPtr];
    }

    // 更新保存的值
    void ReplaceValue(uintptr_t fieldPtr, uintptr_t value) {
        m_values[fieldPtr] = value;
    }

private:
    uintptr_t m_message;

    std::vector<uintptr_t> m_fields;
    std::unordered_map<uintptr_t, uintptr_t> m_values;

    bool ObjectEquals(uintptr_t a, uintptr_t b) {
        if (a == 0 && b != 0) return false;
        if (a != 0 && b == 0) return false;
        return a == b;
    }
};

typedef int32_t il2cpp_array_lower_bound_t;
#define IL2CPP_ARRAY_MAX_INDEX ((int32_t) 0x7fffffff)
#define IL2CPP_ARRAY_MAX_SIZE  ((uint32_t) 0xffffffff)

typedef struct Il2CppArrayBounds
{
    il2cpp_array_size_t length;
    il2cpp_array_lower_bound_t lower_bound;
} Il2CppArrayBounds;

#if IL2CPP_COMPILER_MSVC
#pragma warning( push )
#pragma warning( disable : 4200 )
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif

//Warning: Updates to this struct must also be made to IL2CPPArraySize C code
typedef struct Il2CppArray
{
    // Il2CppObject
    void* klass;
    void* monitor;

    // Il2CppArray
    /* bounds is NULL for szarrays */
    Il2CppArrayBounds* bounds;
    /* total number of elements of the array */
    il2cpp_array_size_t max_length;
} Il2CppArray;

//Il2CppArray* CreateByteArray(const uint8_t* bytes, int32_t length, uintptr_t klass)
//{
//    // byte = 1 字节
//    size_t size = sizeof(Il2CppArray) + sizeof(uint8_t) * length;
//    //uint8_t* mem = (uint8_t*)malloc(size);
//    uint8_t* mem = new uint8_t[size];
//    if (!mem) {
//        //return nullptr;
//        throw std::runtime_error("[CreateByteArray] Unable to allocate memory");
//    }
//
//    Il2CppArray* arr = (Il2CppArray*)mem;
//    arr->klass = (void*)klass;   // System.Byte[] 的 Il2CppClass*
//    arr->monitor = nullptr;
//    arr->bounds = nullptr;       // szarray
//    arr->max_length = length;
//
//    // 数组数据紧跟在 Il2CppArray 后面
//    uint8_t* data = (uint8_t*)(arr + 1);
//
//    if (bytes)
//        memcpy(data, bytes, length);
//    else
//        memset(data, 0, length);
//
//    return arr;
//}

Il2CppArray* CreateByteArray(const uint8_t* bytes, int32_t length, uintptr_t klass)
{

    Il2CppArray* arr = il2cpp_array_new((Il2CppClass*)klass, length);
    uint8_t* data = reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(arr) + 0x20);
    if (bytes)
        memcpy(data, bytes, length);
    else
        memset(data, 0, length);

    return arr;
}


uintptr_t create_input_stream(BYTE* data, int32_t len) {

    auto protobuf_lib = CIl2CppDomain::get().assembly_open("Google.Protobuf.dll").get_image();

	auto coded_input_stream = GetCachedClass("Google.Protobuf.CodedInputStream");
    if (coded_input_stream == nullptr) {
        throw std::runtime_error("failed to find Google.Protobuf.CodedInputStream");
	}

    auto coded_input_stream_obj = CIl2CppObject::from_uintptr((uintptr_t)il2cpp_object_new(coded_input_stream));
    auto constructor = coded_input_stream->find_method(".ctor", { "System.Byte[]" });
    if (constructor.is_null()) {
        throw std::runtime_error("failed to find CodedInputStream(byte[]) constructor");
	}

    auto byte_array_class = constructor.get_param(0).get_class();
	auto byte_array = CreateByteArray(data, static_cast<int32_t>(len), (uintptr_t)byte_array_class.raw_ptr());

    CallIl2CppInstanceObjectMethod<void>(
        coded_input_stream_obj.raw_ptr(),
        "Google.Protobuf.CodedInputStream",
        ".ctor",
        { "System.Byte[]" },
        byte_array
	);

    return coded_input_stream_obj;
}

class CBruteforcer {
private:
    uintptr_t _message;
    CTrackedValues* _trackedValues;
    CIl2CppMethod* _mergeFromMethod;

public:
    CBruteforcer(uintptr_t message, CRuntimeType type)
    {
        _message = message;
        _trackedValues = new CTrackedValues(message, type); // 调用拷贝/移动构造

        //Il2CppClass* klass = il2cpp_object_get_class(message);
        CIl2CppClass klass2 = type.GetIl2CppType().get_class();
        CIl2CppClass* cisType = GetCachedClass("Google.Protobuf.CodedInputStream");

        //_mergeFromMethod = il2cpp_class_get_method_from_name(
        //    klass, "MergeFrom", 1 // 一个参数
        //);
        auto klass2Name = klass2.name();
		auto fn = klass2.find_method("MergeFrom", { "Google.Protobuf.CodedInputStream" });
        if (fn.is_null()) {
            throw std::runtime_error("MergeFrom(CodedInputStream) not found");
        }

        _mergeFromMethod = &fn;

        if (!_mergeFromMethod)
            throw std::runtime_error("MergeFrom(CodedInputStream) not found");
    }

    std::vector<FieldMinimalInfo> Input(uint32_t tag, const std::vector<uint8_t>& data) {
        std::vector<FieldMinimalInfo> result;

        // 构造 protobuf 字节流: [tag varint][data]
        std::vector<uint8_t> buffer;
        EncodeVarint(buffer, tag);
        buffer.insert(buffer.end(), data.begin(), data.end());

        // 创建 CodedInputStream
        uintptr_t cis0 = create_input_stream(buffer.data(), buffer.size());
        CIl2CppObject cis(cis0);

        // 调用 MergeFrom(_message, cis)
        //void* params[1] = { cis };
        //il2cpp_runtime_invoke(_mergeFromMethod, _message, params, nullptr);

        CallIl2CppInstanceObjectMethodDynamic<void>(
            _message,
            "MergeFrom",
            { "Google.Protobuf.CodedInputStream" },
            cis
		);

        // 检测字段变化
        auto changedFields = _trackedValues->DetectChangesAndUpdate();

        if (changedFields.size() == 1) {
            // 普通字段
            CMonoField field(changedFields[0]);
            result.push_back(FieldMinimalInfo(tag, GetFieldOffset(field)));
        }
        else if (changedFields.size() == 2) {
            // Oneof 字段（storage + case enum）
            //FieldInfo* first = changedFields[0];
            //FieldInfo* second = changedFields[1];
            CMonoField first(changedFields[0]);
            CMonoField second(changedFields[1]);

			auto firstType = first.GetFieldType();

            CMonoField dataField;
            CMonoField enumField;
            uintptr_t variantType;

            //il2cpp_field_is_value_type(first)
            if (!firstType.IsEnum() && !firstType.IsValueType()) {
                dataField = first;
                enumField = second;
                variantType = firstType;
            }
            else {
                dataField = second;
                enumField = first;
                variantType = firstType;
            }

            auto oneofInfo = std::make_shared<OneofVariantInfo>(GetFieldOffset(enumField), variantType);
            result.push_back(FieldMinimalInfo(tag, GetFieldOffset(dataField), oneofInfo));
        }
        else if (changedFields.size() > 2) {
            // 多于两个字段变化 → 特殊处理
            for (auto field : changedFields) {
				CMonoField tagField(field);
                result.push_back(FieldMinimalInfo(tag, GetFieldOffset(tagField)));
            }
        }

        // changedFields.size() == 0 → 不做任何操作

        return result;
    }

    ~CBruteforcer() {
        delete _trackedValues;
    }
private:
    int GetFieldOffset(CMonoField field) {
        // MetadataToken 或者 il2cpp_field_get_offset
        return field.GetMetadataToken();
    }
};

static CMonoField GetFieldInfoFromMetadataToken(CRuntimeType type, int32_t metadataToken)
{
    // BindingFlags 必须包含 Public/NonPublic/Instance
    CIl2CppArray fields = type.GetFields(52);
    for (int i = 0; i < fields.length(); i++)
    {
        auto fieldPtr = fields.get<uintptr_t>(i);
        CMonoField field(fieldPtr);
        auto md = field.GetMetadataToken();
        if (md == metadataToken)
            return field;
    }

    return CMonoField(0); // 找不到返回 null
}

void Pb(CMonoAssembly mono_assembly, const char* path) {
    auto types = mono_assembly.GetTypes(60 != 0);
    for (int t = 0; t < types.length(); t++)
    {
        uintptr_t type_ptr = types.get<uintptr_t>(t);
        CRuntimeType type(type_ptr);
        // 只要 class
        if (type.IsEnum())
            continue;
        if (type)

        DebugPrintA("[Pb] Name: %s\n", type.GetFullName().AsString().c_str());
        try {
            CIl2CppObject msg = CActivator::CreateInstance(type);
            CTrackedValues snapshot(msg.raw_ptr(), type);
            CBruteforcer bruteforcer(msg, type);

            // 单条消息字段探测
            auto messageFields = std::vector<Field>();
            auto oneofs = std::map<std::string, Oneof>();

            for (int fieldNumber = 1; fieldNumber <= 2048; fieldNumber++) {
                uint8_t wireTypeVarint = WIRE_TYPE_VAR_INT;
                uint32_t wireTagVarint = PackWireTag(fieldNumber, wireTypeVarint);

                uint32_t testValue = 123;
                std::vector<uint8_t> dataVarint;
                EncodeVarint(dataVarint, testValue);


                std::vector<FieldMinimalInfo> Varint_changedFields = bruteforcer.Input(wireTagVarint, dataVarint);
                auto d_count = Varint_changedFields.size();

                for(auto& field : Varint_changedFields)
                {
                    CMonoField fi = GetFieldInfoFromMetadataToken(type, field.Offset);
                    if (fi.is_null())
                        continue;

                    std::string fieldName = fi.GetName().AsString();

                    CIl2CppObject valueAfterVarint = fi.GetValue(msg);
					uintptr_t valAfterVarintRaw = FetchValue(fi, msg);
                    // ZigZag 编码
                    int zigzagValue = -123;
                    std::vector<uint8_t> dataZigzag;
                    EncodeZigZagVarint(dataZigzag, zigzagValue);
                    std::vector<FieldMinimalInfo> changedFieldsZigzag = bruteforcer.Input((int)wireTagVarint, dataZigzag);
                    CIl2CppObject valueAfteZigzag = fi.GetValue(msg);
                    uintptr_t valueAfteZigzagRaw = FetchValue(fi, msg);

                    bool isZigzag = false;

                    //if (valueAfterZigzag is int intVal)
                    //    isZigzag = intVal == zigzagValue;
                    //else if (valueAfterZigzag is long longVal)
                    //    isZigzag = longVal == zigzagValue;

                    std::string oneofInfo = "";
                    // oneof 输出
      /*              if (field.OneofExtraData != null)
                    {
                        PushOneof(fi.Name, oneofs, field, msg, ref oneofInfo, isZigzag ? "zigzag" : "");
                    }*/
                    //else
                    //{
                    //    // 普通字段
                    //    messageFields.Add(new Field(
                    //        CSharpTypeToProtobufType(fi.FieldType, isZigzag ? "zigzag" : ""),
                    //        fi.Name,
                    //        (uint)(field.Tag >> 3)
                    //    ));
                    //}

                    DebugPrintA("[VARINT] fieldNumber: %d, Name: %s, ValueAfterVarint: %d, ValueAfterZigzag: %d, IsZigZag: %d {oneofInfo}\n",
						fieldNumber, fieldName.c_str(), valAfterVarintRaw, valueAfteZigzagRaw, isZigzag
                    );
                }

            }
        }
        catch (const std::exception& e) {
            DebugPrintA("[Pb] Exception: %s, skip: %s\n", e.what(), type.GetFullName().AsString().c_str());
        }

    }
}