#ifndef INK_TARGET_TARGET_CONTEXT_H
#define INK_TARGET_TARGET_CONTEXT_H

#include "ink/target/target_key.h"

#include <cstdint>

namespace ink::target
{
  class TargetContext
  {
  public:
    explicit TargetContext(TargetKey Key);

    static TargetContext host();

    const TargetKey &key() const noexcept;
    std::uint32_t pointerBitWidth() const noexcept;
    TargetEndianness endianness() const noexcept;
    bool supportsIntegerBitWidth(std::uint32_t BitWidth) const noexcept;

  private:
    TargetKey Key;
  };
} // namespace ink::target

#endif
