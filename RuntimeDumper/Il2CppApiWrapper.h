#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept> // 必须
#include "Il2CppFunctions.h"
#include "PrintHelper.h"

// 提前声明
class CIl2CppAssembly;
class CIl2CppImage;
class CIl2CppClass;
class CIl2CppType;
class CIl2CppMethod;
class CIl2CppField;
class CIl2CppObject;

class CIl2CppWrapBase {
protected:
    uintptr_t ptr = 0;

public:
    CIl2CppWrapBase() : ptr(0) {}
    explicit CIl2CppWrapBase(uintptr_t p) : ptr(p) {}

    bool is_null() const { return ptr == 0; }

    operator void* () const { return reinterpret_cast<void*>(ptr); }
    operator uintptr_t() const { return ptr; }

    uintptr_t raw_ptr() const { return ptr; }
};

class CIl2CppDomain : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;
    operator Il2CppDomain*() const { return (Il2CppDomain*)ptr; }

    std::vector<CIl2CppAssembly> assemblies() const;
    CIl2CppAssembly assembly_open(const std::string& name) const;

    static CIl2CppDomain get();
};

class CIl2CppAssembly : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    CIl2CppImage get_image() const;
};

class CIl2CppImage : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    std::string name() const;
    size_t class_count() const;
    std::vector<CIl2CppClass> classes() const;
};

class CIl2CppClass : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    std::string name() const;
    std::string namespace_name() const;
    CIl2CppClass get_parent() const;
    CIl2CppType byval_arg() const;
    std::vector<CIl2CppMethod> methods() const;
    std::vector<CIl2CppField> fields() const;
    int32_t get_flags() const;
    bool is_enum() const;
    bool is_value_type() const;

    CIl2CppMethod find_method_by_name(const std::string& name) const;
    CIl2CppMethod find_method(const std::string& name, const std::vector<std::string>& arg_types) const;
    CIl2CppMethod find_method_by_return_type(const std::string& return_type, const std::vector<std::string>& arg_types) const;
};

class CIl2CppType : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    std::string name() const;
    uint32_t get_attrs() const;
    bool is_by_ref() const;
    std::string formatted_name() const;
    CIl2CppClass get_class() const;
};

class CIl2CppMethod : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    ::MethodInfo* method_info() const;
    std::string name() const;
    CIl2CppType return_type() const;
    CIl2CppClass class_ptr() const;
    uintptr_t va() const;
    uintptr_t rva() const;
    bool is_valid() const;
    uint32_t param_count() const;
    CIl2CppType get_param(uint32_t i) const;
    std::string param_type_formatted(uint32_t i) const;
    std::string format_params() const;
    int32_t get_flags() const;
};

class CIl2CppField : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    std::string name() const;

    int32_t get_flags() const;

    size_t get_offset() const;

    CIl2CppType get_type() const;

    CIl2CppObject get_value_object(const CIl2CppObject& instance) const;
};

class CIl2CppObject : public CIl2CppWrapBase {
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    static const CIl2CppObject NULL_OBJ;

    static CIl2CppObject from_uintptr(uintptr_t p) { return CIl2CppObject(p); }

    CIl2CppClass get_class() const;

    template<typename T>
    T unbox() const {
        if (is_null()) throw std::runtime_error("Attempt to unbox null object");
        return *(T*)(ptr + 16); // 偏移 +16
    }
};


class CIl2CppArray : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

    // ---- basic fields ----

    inline CIl2CppClass klass() const;
    inline uintptr_t monitor() const;
    inline uintptr_t bounds() const;

    // +0x18 length
    __forceinline size_t length() const {
        return *reinterpret_cast<const size_t*>(ptr + 0x18);
    }

    inline bool empty() const { return length() == 0; }

    // ---- raw data ----
//private:
    inline uintptr_t first_item_ptr() const
    {
        // data starts at +0x20
        return ptr + 0x20;
    }

public:
    // ---- element access (template, header-only) ----

    template<typename T>
    inline const T& get(size_t index) const
    {
        static_assert(!std::is_void_v<T>, "T must not be void");
        return *reinterpret_cast<const T*>(
            first_item_ptr() + index * sizeof(T)
            );
    }

    template<typename T>
    inline T& get_mut(size_t index)
    {
        static_assert(!std::is_void_v<T>, "T must not be void");
        return *reinterpret_cast<T*>(first_item_ptr() + index * sizeof(T));
    }

    template<typename T>
    inline std::vector<T> to_vec() const
    {
        static_assert(std::is_copy_constructible_v<T>,
            "T must be copyable");

        const T* begin = reinterpret_cast<const T*>(first_item_ptr());
        return std::vector<T>(begin, begin + length());
    }

    template<typename T>
    inline std::vector<T> to_vec_sized(size_t size) const
    {
        static_assert(std::is_copy_constructible_v<T>,
            "T must be copyable");

        const T* begin = reinterpret_cast<const T*>(first_item_ptr());
        return std::vector<T>(begin, begin + size);
    }
};


class CNativeList : public CIl2CppWrapBase {
public:
    CNativeList() : CIl2CppWrapBase() {}
    explicit CNativeList(uintptr_t p) : CIl2CppWrapBase(p) {}

    // 获取 Il2CppClass
    inline CIl2CppClass Class() const {
        return CIl2CppClass(*reinterpret_cast<uintptr_t*>(ptr));
    }

    // 获取 monitor
    inline uintptr_t Monitor() const {
        return *reinterpret_cast<uintptr_t*>(ptr + 0x8);
    }

    // 获取 items（返回封装的 CIl2CppArray）
    inline CIl2CppArray Items() const {
        uintptr_t items_ptr = *reinterpret_cast<uintptr_t*>(ptr + 0x10);
        return CIl2CppArray(items_ptr);
    }

    // 获取 size
    inline int Size() const {
        return *reinterpret_cast<int*>(ptr + 0x18);
    }

    // 转 std::vector
    template<typename T>
    std::vector<T> ToVector() const {
        CIl2CppArray items = Items();
        uintptr_t first = items.first_item_ptr();
        return std::vector<T>(reinterpret_cast<T*>(first), reinterpret_cast<T*>(first) + Size());
    }

    // 调用 Add 方法
    inline bool Add(const CIl2CppObject& item, const std::string& class_name) const {
        CIl2CppClass cls = CIl2CppObject(ptr).get_class();
        auto method = cls.find_method("Add", { class_name });
        if (!method)
            return false;

        using FuncType = uintptr_t(__fastcall*)(uintptr_t, CIl2CppObject);
        FuncType func = reinterpret_cast<FuncType>(method.va());

        try {
            func(ptr, item);
        }
        catch (...) {
			DebugPrintA("[NativeList] 调用 Add 函数出现异常\n");
            return false;
        }
        return true;
    }
};

template<typename T>
class CNativeArray {
public:
    void* klass;
    void* monitor;
    void* bounds;
    int32_t max_length;
    T vector[65535];

public:
    CNativeArray()
        : klass(nullptr), monitor(nullptr), bounds(nullptr), max_length(0) {
    }

    explicit CNativeArray(int32_t len)
        : klass(nullptr), monitor(nullptr), bounds(nullptr), max_length(len)
    {
        static_assert(sizeof(vector) / sizeof(T) >= 65535, "Array too small!");
    }

    // 安全访问
    T get(size_t index) const {
        if (index >= static_cast<size_t>(max_length)) {
            throw std::out_of_range("CNativeArray index out of bounds");
        }
        return vector[index];
    }

    // 尝试获取，返回指针 nullptr 如果越界
    const T* get_ptr(size_t index) const {
        if (index >= static_cast<size_t>(max_length)) return nullptr;
        return &vector[index];
    }

    // 是否包含
    bool contains(const T& item) const {
        for (int32_t i = 0; i < max_length; i++) {
            if (vector[i] == item) {
                return true;
            }
        }
        return false;
    }

    // 获取长度
    int32_t length() const { return max_length; }

    // operator[]，可直接访问但不安全
    T operator[](size_t index) const { return get(index); }
    T& operator[](size_t index) {
        if (index >= static_cast<size_t>(max_length)) throw std::out_of_range("CNativeArray index out of bounds");
        return vector[index];
    }
};

// ------------------- Entry -------------------
template<typename TKey, typename TValue>
struct CEntry {
    int32_t hash_code;
    int32_t next;
    TKey key;
    TValue value;
};

// ------------------- NativeDictionary -------------------
template<typename TKey, typename TValue>
class CNativeDictionary {
public:
    uintptr_t klass;       // 0x0
    uintptr_t monitor;     // 0x8
    CNativeArray<int32_t>* buckets;   // 0x10
    CNativeArray<CEntry<TKey, TValue>>* entries; // 0x18
    int32_t count;         // 0x20
    int32_t version;       // 0x24
    int32_t free_list;     // 0x28
    int32_t free_count;    // 0x2C
    uintptr_t comparer;    // 0x30
    void* keys;            // 0x38
    void* values;          // 0x40
    uintptr_t sync_root;   // 0x48

    // ------------------- 实例方法 -------------------

    inline int find_entry(const TKey& key) const {
        if (!entries) return -1;
        auto arr = reinterpret_cast<CEntry<TKey, TValue>*>(entries);
        for (int i = 0; i < count; i++) {
            if (arr[i].key == key) return i;
        }
        return -1;
    }

    inline bool contains_key(const TKey& key) const {
        return find_entry(key) >= 0;
    }

    inline bool contains_value(const TValue& value) const {
        if (!entries) return false;
        auto arr = reinterpret_cast<CEntry<TKey, TValue>*>(entries);
        for (int i = 0; i < count; i++) {
            if (arr[i].hash_code >= 0 && arr[i].value == value) return true;
        }
        return false;
    }

    inline TValue try_get_value(const TKey& key, bool* found = nullptr) const {
        int i = find_entry(key);
        if (i >= 0) {
            if (found) *found = true;
            auto arr = reinterpret_cast<CEntry<TKey, TValue>*>(entries);
            return arr[i].value;
        }
        if (found) *found = false;
        return TValue{};
    }

    inline TValue get_value_or_default(const TKey& key) const {
        return try_get_value(key);
    }

    inline TValue get_item(const TKey& key) const {
        return try_get_value(key);
    }

    inline int get_count() const { return count; }
    inline uintptr_t get_comparer() const { return comparer; }

    // ------------------- 调用 C# Remove 方法 -------------------
    inline static bool remove(uintptr_t dict_ptr, uintptr_t key_ptr, const std::string& class_name) {
        Il2CppClass* cls_rp = il2cpp_object_get_class((Il2CppObject*)dict_ptr);
		auto cls = CIl2CppClass((uintptr_t)cls_rp);
        auto method = cls.find_method("Remove", { class_name });
        if (!method.va()) return false;

        using FuncType = bool(__fastcall*)(uintptr_t, uintptr_t);
        FuncType func = reinterpret_cast<FuncType>(method.va());

        try {
            return func(dict_ptr, key_ptr);
        }
        catch (...) {
            DebugPrintA("[CNativeDictionary] Remove exception\n");
            return false;
        }
    }
};

// ------------------- KeysCollection -------------------
template<typename TKey, typename TValue>
class CKeysCollection {
public:
    CNativeDictionary<TKey, TValue>* dictionary;

    inline TKey get(size_t index) const {
        if (!dictionary || !dictionary->entries) return TKey{};
        auto arr = reinterpret_cast<CEntry<TKey, TValue>*>(dictionary->entries->first_item_ptr());
        return arr[index].key;
    }

    inline int get_count() const {
        return dictionary ? dictionary->get_count() : 0;
    }
};

// ------------------- ValuesCollection -------------------
template<typename TKey, typename TValue>
class CValuesCollection {
public:
    CNativeDictionary<TKey, TValue>* dictionary;

    inline TValue get(size_t index) const {
        if (!dictionary || !dictionary->entries) return TValue{};
        auto arr = reinterpret_cast<CEntry<TKey, TValue>*>(dictionary->entries->first_item_ptr());
        return arr[index].value;
    }

    inline int get_count() const {
        return dictionary ? dictionary->get_count() : 0;
    }
};

