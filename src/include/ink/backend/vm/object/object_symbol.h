#pragma once

#include "ink/backend/vm/object/object_section_kind.h"
#include "ink/core/name/name.h"

#include <cstdint>
#include <optional>

namespace ink::vm
{
  enum class ObjectSymbolLinkage : std::uint8_t
  {
    Local = 0,
    Export = 1,
    Import = 2,
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
      core::Name Name = core::NAME_None;
      ObjectSymbolLinkage Linkage = ObjectSymbolLinkage::Local;
      // Local and Export symbols require a definition; Import symbols must not have one.
      std::optional<ObjectSymbolDefinition> Definition;

      bool isImport() const noexcept
      {
        return Linkage == ObjectSymbolLinkage::Import;
      }
  };
} // namespace ink::vm
