#pragma once

#include "Il2CppFunctions.h"
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace Cerydra::CSharp
{
    class Object
    {
    public:
        union
        {
            ::Il2CppClass* klass;
            void* vtable;
        } nativeClass{};

        void* monitor{};

        ::Il2CppClass* GetNativeClass() const
        {
            return nativeClass.klass;
        }

        uintptr_t Address() const
        {
            return reinterpret_cast<uintptr_t>(this);
        }

        template <typename T>
        T Unbox() const
        {
            return *reinterpret_cast<const T*>(Address() + sizeof(Object));
        }
    };

    using RuntimeObject = Object;

    template <typename T>
    class Array : public Object
    {
    public:
        class Bounds
        {
        public:
            uintptr_t length;
            int32_t lowerBound;
        };

        Bounds* bounds{};
        uintptr_t maxLength{};
        T vector[1]{};

        uintptr_t Length() const
        {
            return maxLength;
        }

        uintptr_t length() const
        {
            return Length();
        }

        bool Empty() const
        {
            return Length() == 0;
        }

        uintptr_t FirstItemAddress() const
        {
            return reinterpret_cast<uintptr_t>(&vector);
        }

        uintptr_t first_item_ptr() const
        {
            return FirstItemAddress();
        }

        T& At(size_t index)
        {
            if (index >= Length()) {
                throw std::out_of_range("Array 下标越界");
            }

            return *reinterpret_cast<T*>(FirstItemAddress() + sizeof(T) * index);
        }

        const T& At(size_t index) const
        {
            if (index >= Length()) {
                throw std::out_of_range("Array 下标越界");
            }

            return *reinterpret_cast<const T*>(FirstItemAddress() + sizeof(T) * index);
        }

        T& operator[](size_t index)
        {
            return At(index);
        }

        const T& operator[](size_t index) const
        {
            return At(index);
        }

        template <typename U = T>
        U Get(size_t index) const
        {
            static_assert(std::is_convertible_v<T, U> || std::is_same_v<T, U>, "数组元素类型不兼容");
            return static_cast<U>(At(index));
        }

        template <typename U = T>
        U get(size_t index) const
        {
            return Get<U>(index);
        }

        std::vector<T> ToVector() const
        {
            const auto begin = reinterpret_cast<const T*>(FirstItemAddress());
            return std::vector<T>(begin, begin + Length());
        }

        std::vector<T> to_vec() const
        {
            return ToVector();
        }
    };

    template <typename T>
    class List : public Object
    {
    public:
        Array<T>* items{};
        int32_t size{};
        int32_t version{};
        void* syncRoot{};

        Array<T>* Items() const
        {
            return items;
        }

        int32_t Size() const
        {
            return size;
        }

        T& At(size_t index)
        {
            if (!items) {
                throw std::runtime_error("List 内部数组为空");
            }

            return items->At(index);
        }

        const T& At(size_t index) const
        {
            if (!items) {
                throw std::runtime_error("List 内部数组为空");
            }

            return items->At(index);
        }

        T& operator[](size_t index)
        {
            return At(index);
        }

        const T& operator[](size_t index) const
        {
            return At(index);
        }

        std::vector<T> ToVector() const
        {
            if (!items || size <= 0) {
                return {};
            }

            const auto begin = reinterpret_cast<const T*>(items->FirstItemAddress());
            return std::vector<T>(begin, begin + size);
        }
    };

    template <typename TKey, typename TValue>
    class Entry
    {
    public:
        int32_t hashCode{};
        int32_t next{};
        TKey key{};
        TValue value{};
    };

    template <typename TKey, typename TValue>
    class Dictionary : public Object
    {
    public:
        Array<int32_t>* buckets{};
        Array<Entry<TKey, TValue>>* entries{};
        int32_t count{};
        int32_t version{};
        int32_t freeList{};
        int32_t freeCount{};
        void* comparer{};
        void* keys{};
        void* values{};
        void* syncRoot{};
    };
}
