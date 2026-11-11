#pragma once

#include "ink/backend/vm/executable/byte_order.h"
#include "ink/backend/vm/executable/dynamic_import_symbol.h"
#include "ink/backend/vm/executable/dynamic_import_symbol_relocation.h"
#include "ink/backend/vm/executable/export_symbol.h"

#include <cstdint>
#include <vector>

namespace ink::vm
{
  // Owns a flat, unbound VM executable image, unified dynamic imports, address relocations, and named image exports.
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

      auto &getByteOrder(this auto &&Self) noexcept
      {
        return Self.Order;
      }

      auto &getDynamicImportSymbols(this auto &&Self) noexcept
      {
        return Self.DynamicImportSymbols;
      }

      auto &getDynamicImportSymbolRelocations(this auto &&Self) noexcept
      {
        return Self.DynamicImportSymbolRelocations;
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
      ByteOrder Order = ByteOrder::Little;
      std::vector<DynamicImportSymbol> DynamicImportSymbols;
      std::vector<DynamicImportSymbolRelocation> DynamicImportSymbolRelocations;
      std::vector<ExportSymbol> ExportSymbols;
  };
} // namespace ink::vm
