#pragma once

#include <cstdint>
#include <vector>

namespace ink::ir
{
  class ObjectCodeSection final
  {
    public:
      auto &getBytes(this auto &&Self) noexcept
      {
        return Self.Bytes;
      }

      std::uint64_t byteSize() const noexcept
      {
        return static_cast<std::uint64_t>(Bytes.size());
      }

    private:
      std::vector<std::uint8_t> Bytes;
  };
} // namespace ink::ir
