#pragma once

#include <cstdint>
#include <vector>

namespace ink::ir
{
  // Loaded as read-only storage.
  class ObjectConstantSection final
  {
    public:
      auto &getAlignment(this auto &&Self) noexcept
      {
        return Self.Alignment;
      }

      auto &getBytes(this auto &&Self) noexcept
      {
        return Self.Bytes;
      }

      std::uint64_t byteSize() const noexcept
      {
        return static_cast<std::uint64_t>(Bytes.size());
      }

    private:
      std::uint64_t Alignment = 1;
      std::vector<std::uint8_t> Bytes;
  };
} // namespace ink::ir
