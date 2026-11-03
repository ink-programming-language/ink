#pragma once

#include "ink/ir/object/object_section_kind.h"

#include <cstdint>
#include <optional>
#include <string>

namespace ink::ir
{
  enum class ObjectSymbolLinkage : std::uint8_t
  {
    Local,
    Export,
    Import,
  };

  struct ObjectSymbolDefinition
  {
      // Offsets and extents are relative to the selected region of this object.
      ObjectSectionKind Section = ObjectSectionKind::Code;
      std::uint64_t ByteOffset = 0;
      std::uint64_t ByteSize = 0;
  };

  struct ObjectSymbol
  {
      // Import and Export names share one external namespace; Local names are object-private.
      std::string Name;
      ObjectSymbolLinkage Linkage = ObjectSymbolLinkage::Local;
      // Local and Export symbols require a definition; Import symbols must not have one.
      std::optional<ObjectSymbolDefinition> Definition;

      bool isImport() const noexcept
      {
        return Linkage == ObjectSymbolLinkage::Import;
      }
  };
} // namespace ink::ir
