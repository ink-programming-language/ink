#pragma once
#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <new>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>
namespace ink::parser
{
  template <typename T>
  using ASTArray = std::span<const T>;

  // A pointer array with transitive constness, without aliasing pointer storage.
  template <typename T>
  class ConstNodeArray
  {
    public:
      class Iterator
      {
        public:
          using value_type = const T *;
          using difference_type = std::ptrdiff_t;
          using iterator_category = std::forward_iterator_tag;
          explicit Iterator(T *const *Position)
              : Position(Position)
          {
          }
          const T *operator*() const
          {
            return *Position;
          }
          Iterator &operator++()
          {
            ++Position;
            return *this;
          }
          Iterator operator++(int)
          {
            Iterator Old = *this;
            ++*this;
            return Old;
          }
          bool operator==(const Iterator &) const = default;

        private:
          T *const *Position;
      };
      explicit ConstNodeArray(ASTArray<T *> Values)
          : Values(Values)
      {
      }
      Iterator begin() const
      {
        return Iterator(Values.data());
      }
      Iterator end() const
      {
        return Iterator(Values.empty() ? Values.data() : Values.data() + Values.size());
      }
      std::size_t size() const
      {
        return Values.size();
      }
      bool empty() const
      {
        return Values.empty();
      }
      const T *operator[](std::size_t Index) const
      {
        return Values[Index];
      }
      const T *front() const
      {
        return Values.front();
      }
      const T *back() const
      {
        return Values.back();
      }

    private:
      ASTArray<T *> Values;
  };

  struct ASTCheckpoint
  {
      std::size_t Blocks;
      std::size_t Offset;
      std::size_t Destructors;
      std::size_t Bytes;
  };

  class ASTContext
  {
    public:
      ASTContext() = default;
      ~ASTContext();
      ASTContext(const ASTContext &) = delete;
      ASTContext &operator=(const ASTContext &) = delete;
      template <typename T, typename... Args>
      T *make(Args &&...Values)
      {
        // All Ink targets disable exceptions. Host allocation failure is fatal.
        // Reserve the callback before construction, so registration cannot fail
        // after a resource-owning object has been successfully constructed.
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
          if (Destructors.size() == Destructors.capacity())
          {
            Destructors.reserve(Destructors.empty() ? 16 : Destructors.size() * 2);
          }
        }
        T *Value = ::new (allocate(sizeof(T), alignof(T))) T(std::forward<Args>(Values)...);
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
          Destructors.push_back({Value, 1, &destroy<T>});
        }
        return Value;
      }
      template <typename T>
      ASTArray<T> copyArray(std::span<const T> Values)
      {
        if (Values.empty())
        {
          return {};
        }
        assert(Values.size() <= std::numeric_limits<std::size_t>::max() / sizeof(T));
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
          if (Destructors.size() == Destructors.capacity())
          {
            Destructors.reserve(Destructors.empty() ? 16 : Destructors.size() * 2);
          }
        }
        T *Data = static_cast<T *>(allocate(sizeof(T) * Values.size(), alignof(T)));
        for (std::size_t Index = 0; Index < Values.size(); ++Index)
        {
          ::new (Data + Index) T(Values[Index]);
        }
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
          Destructors.push_back({Data, Values.size(), &destroy<T>});
        }
        return {Data, Values.size()};
      }
      template <typename T>
      ASTArray<T> copyArray(const std::vector<T> &Values)
      {
        return copyArray<T>(std::span<const T>(Values));
      }
      ASTCheckpoint checkpoint() const noexcept;
      void rollback(ASTCheckpoint Checkpoint);
      std::size_t allocatedBytes() const noexcept
      {
        return Bytes;
      }

    private:
      struct Block
      {
          void *Data;
          std::size_t Size;
          std::size_t Alignment;
          std::size_t Used;
      };
      struct Destructor
      {
          void *Data;
          std::size_t Count;
          void (*Run)(void *, std::size_t);
      };
      template <typename T>
      static void destroy(void *Data, std::size_t Count)
      {
        T *Values = static_cast<T *>(Data);
        while (Count != 0)
        {
          Values[--Count].~T();
        }
      }
      void *allocate(std::size_t Size, std::size_t Alignment);
      std::vector<Block> Blocks;
      std::vector<Destructor> Destructors;
      std::size_t Bytes = 0;
  };
} // namespace ink::parser
