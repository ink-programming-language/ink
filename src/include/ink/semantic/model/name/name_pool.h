#ifndef INK_SEMANTIC_MODEL_NAME_NAME_POOL_H
#define INK_SEMANTIC_MODEL_NAME_NAME_POOL_H

#include "ink/semantic/model/name/name.h"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ink::semantic
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
      struct StringHash
      {
          using is_transparent = void;

          std::size_t operator()(std::string_view Text) const noexcept;
      };

      // Map keys own the only spelling copy; node references survive rehashing.
      std::unordered_map<std::string, Name, StringHash, std::equal_to<>> Names;
      std::vector<std::string_view> Spellings;
  };
} // namespace ink::semantic

#endif
