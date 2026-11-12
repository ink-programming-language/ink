#pragma once

#include "ink/backend/vm/executable/executable_file_type.h"

#include <cstdint>
#include <vector>

namespace ink::vm
{
  // Owns a flat, little-endian, unbound VM executable image, unified dynamic imports, address relocations, and named image exports.
  // EntryOffset and exported symbol offsets are relative to Image, never host pointers or bound runtime addresses.
  // File I/O, loading, bound addresses, execution state, and native ABI/call layouts are outside this model.
  class ExecutableFile final
  {
    public:
      auto &getImage(this auto &&Self) noexcept
      {
        return Self.Image;
      }

      auto &getEntryOffset(this auto &&Self) noexcept
      {
        return Self.EntryOffset;
      }

      auto &getDynamicImportSymbols(this auto &&Self) noexcept
      {
        return Self.DynamicImportSymbols;
      }

      auto &getDynamicImportSymbolRelocations(this auto &&Self) noexcept
      {
        return Self.DynamicImportSymbolRelocations;
      }

      auto &getImageBaseAddressRelocations(this auto &&Self) noexcept
      {
        return Self.ImageBaseAddressRelocations;
      }

      auto &getExportSymbols(this auto &&Self) noexcept
      {
        return Self.ExportSymbols;
      }

      std::uint64_t byteSize() const noexcept
      {
        return static_cast<std::uint64_t>(Image.size());
      }

    private:
      std::vector<std::uint8_t> Image;
      std::uint64_t EntryOffset = 0;
      std::vector<DynamicImportSymbol> DynamicImportSymbols;
      std::vector<DynamicImportSymbolRelocation> DynamicImportSymbolRelocations;
      std::vector<ImageBaseAddressRelocation> ImageBaseAddressRelocations;
      std::vector<ExportSymbol> ExportSymbols;
  };
} // namespace ink::vm
