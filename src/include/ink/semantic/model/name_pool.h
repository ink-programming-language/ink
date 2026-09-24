#ifndef INK_SEMANTIC_MODEL_NAME_POOL_H
#define INK_SEMANTIC_MODEL_NAME_POOL_H

#include "ink/semantic/model/name.h"

#include <memory>
#include <string_view>

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
      class Impl;
      std::unique_ptr<Impl> Storage;
  };
} // namespace ink::semantic

#endif
