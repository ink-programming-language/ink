#pragma once

#include "ink/ir/object/object_base_reference_table.h"
#include "ink/ir/object/object_code_section.h"
#include "ink/ir/object/object_constant_section.h"
#include "ink/ir/object/object_debug_info.h"
#include "ink/ir/object/object_global_storage.h"
#include "ink/ir/object/object_header.h"
#include "ink/ir/object/object_symbol_table.h"

#include <optional>

namespace ink::ir
{
  // Owns a linkable VM object's logical contents. Runtime addresses, file handles,
  // binary encoding/decoding, and linker state are outside this model.
  // References use object-local indices and section-relative byte offsets. Readers/writers must encode fields explicitly,
  // never dump this C++ layout. Symbol addresses are resolved through BaseReferences.
  class ObjectFile final
  {
    public:
      auto &getHeader(this auto &&Self) noexcept
      {
        return Self.Header;
      }

      auto &getCode(this auto &&Self) noexcept
      {
        return Self.Code;
      }

      auto &getConstants(this auto &&Self) noexcept
      {
        return Self.Constants;
      }

      auto &getGlobalStorage(this auto &&Self) noexcept
      {
        return Self.GlobalStorage;
      }

      auto &getSymbols(this auto &&Self) noexcept
      {
        return Self.Symbols;
      }

      auto &getBaseReferences(this auto &&Self) noexcept
      {
        return Self.BaseReferences;
      }

      auto &getDebugInfo(this auto &&Self) noexcept
      {
        return Self.DebugInfo;
      }

    private:
      ObjectHeader Header;
      ObjectCodeSection Code;
      ObjectConstantSection Constants;
      ObjectGlobalStorage GlobalStorage;
      ObjectSymbolTable Symbols;
      ObjectBaseReferenceTable BaseReferences;
      std::optional<ObjectDebugInfo> DebugInfo;
  };
} // namespace ink::ir
