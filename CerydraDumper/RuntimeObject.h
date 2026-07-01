#pragma once

#include "Il2CppFunctions.h"
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace Cerydra::CSharp
{
    class ObjectRef
    {
    protected:
        uintptr_t ptr{};

    public:
        ObjectRef() = default;
        explicit ObjectRef(uintptr_t value) : ptr(value) {}

        explicit operator bool() const { return ptr != 0; }
        bool operator!() const { return ptr == 0; }
        bool IsNull() const { return ptr == 0; }
        bool is_null() const { return IsNull(); }
        uintptr_t RawPtr() const { return ptr; }
        uintptr_t raw_ptr() const { return ptr; }
    };

    class RuntimeObject : public ObjectRef
    {
    public:
        using ObjectRef::ObjectRef;

        static RuntimeObject FromUIntPtr(uintptr_t value)
        {
            return RuntimeObject(value);
        }

        Il2CppClass* GetNativeClass() const
        {
            if (!ptr) {
                return nullptr;
            }

            return *reinterpret_cast<Il2CppClass**>(ptr);
        }

        template <typename T>
        T Unbox() const
        {
            if (!ptr) {
                throw std::runtime_error("尝试拆箱空对象");
            }

            return *reinterpret_cast<T*>(ptr + 16);
        }
    };

    class ArrayObject : public ObjectRef
    {
    public:
        using ObjectRef::ObjectRef;

        Il2CppClass* GetNativeClass() const
        {
            return ptr ? *reinterpret_cast<Il2CppClass**>(ptr) : nullptr;
        }

        uintptr_t Monitor() const
        {
            return *reinterpret_cast<uintptr_t*>(ptr + 0x08);
        }

        uintptr_t Bounds() const
        {
            return *reinterpret_cast<uintptr_t*>(ptr + 0x10);
        }

        size_t Length() const
        {
            return ptr ? *reinterpret_cast<const size_t*>(ptr + 0x18) : 0;
        }

        size_t length() const
        {
            return Length();
        }

        bool Empty() const
        {
            return Length() == 0;
        }

        uintptr_t FirstItemPtr() const
        {
            return ptr + 0x20;
        }

        uintptr_t first_item_ptr() const
        {
            return FirstItemPtr();
        }

        template <typename T>
        const T& Get(size_t index) const
        {
            static_assert(!std::is_void_v<T>, "T must not be void");
            return *reinterpret_cast<const T*>(FirstItemPtr() + index * sizeof(T));
        }

        template <typename T>
        const T& get(size_t index) const
        {
            return Get<T>(index);
        }

        template <typename T>
        T& GetMutable(size_t index)
        {
            static_assert(!std::is_void_v<T>, "T must not be void");
            return *reinterpret_cast<T*>(FirstItemPtr() + index * sizeof(T));
        }

        template <typename T>
        std::vector<T> ToVector() const
        {
            static_assert(std::is_copy_constructible_v<T>, "T must be copyable");
            const T* begin = reinterpret_cast<const T*>(FirstItemPtr());
            return std::vector<T>(begin, begin + Length());
        }

        template <typename T>
        std::vector<T> to_vec() const
        {
            return ToVector<T>();
        }
    };

    class NativeList : public ObjectRef
    {
    public:
        using ObjectRef::ObjectRef;

        ArrayObject Items() const
        {
            return ArrayObject(ptr ? *reinterpret_cast<uintptr_t*>(ptr + 0x10) : 0);
        }

        int Size() const
        {
            return ptr ? *reinterpret_cast<int*>(ptr + 0x18) : 0;
        }

        template <typename T>
        std::vector<T> ToVector() const
        {
            auto items = Items();
            const auto first = items.FirstItemPtr();
            return std::vector<T>(reinterpret_cast<T*>(first), reinterpret_cast<T*>(first) + Size());
        }
    };

    template <typename T>
    class NativeArray
    {
    public:
        void* klass{};
        void* monitor{};
        void* bounds{};
        int32_t maxLength{};
        T vector[65535];

        T Get(size_t index) const
        {
            if (index >= static_cast<size_t>(maxLength)) {
                throw std::out_of_range("NativeArray 下标越界");
            }

            return vector[index];
        }

        int32_t Length() const
        {
            return maxLength;
        }
    };

    template <typename TKey, typename TValue>
    struct Entry
    {
        int32_t hashCode;
        int32_t next;
        TKey key;
        TValue value;
    };

    template <typename TKey, typename TValue>
    class NativeDictionary
    {
    public:
        uintptr_t klass;
        uintptr_t monitor;
        NativeArray<int32_t>* buckets;
        NativeArray<Entry<TKey, TValue>>* entries;
        int32_t count;
        int32_t version;
        int32_t freeList;
        int32_t freeCount;
        uintptr_t comparer;
        void* keys;
        void* values;
        uintptr_t syncRoot;
    };
}
