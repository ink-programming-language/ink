#pragma once

#include <cstddef>

namespace ink::core
{
  namespace detail
  {
    template <typename T>
    concept HasMemberHash = requires(const T &Value)
    {
      Value.hash();
    };
  } // namespace detail

  // KeyType is the stored container key. Its static hash(Value) overloads handle queries without a hash() member.
  template <typename KeyType>
  struct Hash final
  {
    public:
      using is_transparent = void;

      template <typename T>
      std::size_t operator()(const T &Value) const noexcept
      {
        if constexpr (detail::HasMemberHash<T>)
        {
          static_assert(noexcept(static_cast<std::size_t>(Value.hash())), "Hash requires a non-throwing hash() result convertible to size_t");
          return static_cast<std::size_t>(Value.hash());
        }
        else
        {
          static_assert(noexcept(static_cast<std::size_t>(KeyType::hash(Value))), "Hash requires a non-throwing query hash result convertible to size_t");
          return static_cast<std::size_t>(KeyType::hash(Value));
        }
      }
  };
} // namespace ink::core
