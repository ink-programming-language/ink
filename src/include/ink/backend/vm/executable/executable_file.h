#pragma once

#include "ink/backend/vm/executable/native_data_import.h"
#include "ink/backend/vm/executable/native_data_relocation.h"
#include "ink/backend/vm/executable/native_function_import.h"

#include <cstdint>
#include <vector>

namespace ink::vm
{
  // Owns a flat, unbound VM executable image, host function imports, and host data imports with address relocations.
  // EntryOffset is relative to the beginning of Image, never a host pointer or a bound runtime address.
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

      auto &getNativeFunctions(this auto &&Self) noexcept
      {
        return Self.NativeFunctions;
      }

      auto &getNativeData(this auto &&Self) noexcept
      {
        return Self.NativeData;
      }

      auto &getNativeDataRelocations(this auto &&Self) noexcept
      {
        return Self.NativeDataRelocations;
      }

      std::uint64_t byteSize() const noexcept
      {
        return static_cast<std::uint64_t>(Image.size());
      }

    private:
      std::vector<std::uint8_t> Image;
      std::uint64_t EntryOffset = 0;
      std::vector<NativeFunctionImport> NativeFunctions;
      std::vector<NativeDataImport> NativeData;
      std::vector<NativeDataRelocation> NativeDataRelocations;
  };
} // namespace ink::vm
