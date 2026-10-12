#ifndef INK_CORE_NAME_POOL_H
#define INK_CORE_NAME_POOL_H

#include "ink/core/name/name.h"
#include "ink/core/support/singleton.h"

#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>

namespace ink::core
{
  // Owns one process-wide copy of each spelling. Names and text views survive all insertions.
  // Input is already validated/normalized by the frontend; interning compares bytes.
  // Concurrent pool operations require external synchronization.
  class NamePool final : public Singleton<NamePool>
  {
    public:
      // Empty input or exhausted index space returns NAME_None.
      Name intern(std::string_view Text);
      // Missing or empty spellings return NAME_None without interning.
      Name find(std::string_view Text) const noexcept;
      // Invalid/out-of-range handles return an empty view.
      std::string_view text(Name Value) const noexcept;
      bool contains(Name Value) const noexcept;
      std::size_t size() const noexcept;

    private:
      friend class Singleton<NamePool>;

      NamePool();
      ~NamePool();

      // Appending immutable strings preserves their addresses, including short strings.
      // View keys allow allocation-free lookup without C++20 heterogeneous hashing.
      std::deque<std::string> Spellings;
      std::unordered_map<std::string_view, Name> Names;
  };
} // namespace ink::core

#endif
