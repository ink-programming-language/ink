#ifndef INK_LIB_SEMANTIC_MODEL_HASH_H
#define INK_LIB_SEMANTIC_MODEL_HASH_H

#include <cstddef>

namespace ink::semantic
{
  // In-memory lookup only; callers still compare complete keys after hashing.
  inline std::size_t combineHash(std::size_t Seed, std::size_t Value) noexcept
  {
    return Seed ^ (Value + static_cast<std::size_t>(0x9e3779b9U) + (Seed << 6) + (Seed >> 2));
  }
} // namespace ink::semantic

#endif
