#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ink::vm
{
  struct ObjectLineEntry
  {
      // Byte offset within the object's code buffer.
      std::uint64_t CodeOffset = 0;
      std::uint32_t FileIndex = 0;
      std::uint32_t Line = 1;
      std::uint32_t Column = 1;
  };

  class ObjectDebugInfo final
  {
    public:
      auto &getSourceFiles(this auto &&Self) noexcept
      {
        return Self.SourceFiles;
      }

      auto &getLines(this auto &&Self) noexcept
      {
        return Self.Lines;
      }

    private:
      std::vector<std::string> SourceFiles;
      std::vector<ObjectLineEntry> Lines;
  };
} // namespace ink::vm
