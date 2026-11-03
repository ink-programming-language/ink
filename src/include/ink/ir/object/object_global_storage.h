#pragma once

#include <cstdint>
#include <vector>

namespace ink::ir
{
  // Loaded as writable storage. Copy InitialBytes to the start and zero the remaining bytes.
  class ObjectGlobalStorage final
  {
    public:
      auto &getAlignment(this auto &&Self) noexcept
      {
        return Self.Alignment;
      }

      auto &getByteSize(this auto &&Self) noexcept
      {
        return Self.ByteSize;
      }

      auto &getInitialBytes(this auto &&Self) noexcept
      {
        return Self.InitialBytes;
      }

    private:
      std::uint64_t Alignment = 1;
      std::uint64_t ByteSize = 0;
      std::vector<std::uint8_t> InitialBytes;
  };
} // namespace ink::ir
