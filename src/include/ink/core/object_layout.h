#ifndef INK_CORE_OBJECT_LAYOUT_H
#define INK_CORE_OBJECT_LAYOUT_H

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>

namespace ink::core
{
  // Declaration-order aggregate layout shared by IR, execution and archive
  // validation. Size is the complete object size, including tail padding.
  class ObjectLayoutBuilder final
  {
    public:
      explicit ObjectLayoutBuilder(std::uint64_t Limit = std::numeric_limits<std::uint64_t>::max()) noexcept
          : Limit(Limit)
      {
      }

      std::optional<std::uint64_t> append(std::uint64_t FieldSize, std::uint64_t FieldAlignment) noexcept
      {
        if (!FieldAlignment || (FieldAlignment & (FieldAlignment - 1)) || End > Limit)
        {
          return std::nullopt;
        }
        const std::uint64_t Padding = (FieldAlignment - End % FieldAlignment) % FieldAlignment;
        if (Padding > Limit - End || FieldSize > Limit - End - Padding)
        {
          return std::nullopt;
        }
        const std::uint64_t Offset = End + Padding;
        End = Offset + FieldSize;
        Alignment = std::max(Alignment, FieldAlignment);
        return Offset;
      }

      std::optional<std::uint64_t> size() const noexcept
      {
        const std::uint64_t Minimum = std::max<std::uint64_t>(End, 1);
        const std::uint64_t Padding = (Alignment - Minimum % Alignment) % Alignment;
        return Minimum <= Limit && Padding <= Limit - Minimum ? std::optional<std::uint64_t>(Minimum + Padding) : std::nullopt;
      }

      std::uint64_t alignment() const noexcept
      {
        return Alignment;
      }

    private:
      std::uint64_t Limit;
      std::uint64_t End = 0;
      std::uint64_t Alignment = 1;
  };
} // namespace ink::core

#endif
