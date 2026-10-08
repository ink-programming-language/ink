#ifndef INK_CORE_NAME_POOL_H
#define INK_CORE_NAME_POOL_H

#include "ink/core/name.h"

#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>

namespace ink::core
{
  // Owns one copy of each spelling. Names and text views survive all insertions.
  // Input is already validated/normalized by the frontend; interning compares bytes.
  class NamePool final
  {
    public:
      NamePool();
      ~NamePool();
      NamePool(const NamePool &) = delete;
      NamePool &operator=(const NamePool &) = delete;
      NamePool(NamePool &&) = delete;
      NamePool &operator=(NamePool &&) = delete;

      // Empty input or exhausted index space returns an invalid Name.
      Name intern(std::string_view Text);
      Name find(std::string_view Text) const noexcept;
      // Invalid/out-of-range handles return an empty view. Foreign pool indices
      // cannot be detected: callers must retain the associated pool/context.
      std::string_view text(Name Value) const noexcept;
      bool contains(Name Value) const noexcept;
      std::size_t size() const noexcept;

    private:
      // Appending immutable strings preserves their addresses, including short strings.
      // View keys allow allocation-free lookup without C++20 heterogeneous hashing.
      std::deque<std::string> Spellings;
      std::unordered_map<std::string_view, Name> Names;
  };
} // namespace ink::core

#endif
