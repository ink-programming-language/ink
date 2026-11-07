#include "ink/vm/object/object_file_validator.h"

#include "ink/core/config/config_manager.h"
#include "ink/vm/object/object_file.h"

#include <cstddef>
#include <limits>
#include <string_view>
#include <unordered_set>

namespace ink::vm
{
  namespace
  {
    constexpr bool hasTooManyRecords(std::size_t Count) noexcept
    {
      return Count != 0 && Count - 1 > std::numeric_limits<std::uint32_t>::max();
    }

    constexpr bool hasValidAlignment(std::uint64_t Alignment) noexcept
    {
      return Alignment != 0 && (Alignment & (Alignment - 1)) == 0;
    }
  } // namespace

  std::optional<ObjectValidationError> ObjectFileValidator::validate(const ObjectFile &Object) noexcept
  {
    const auto &Header = Object.getHeader();
    const auto &Code = Object.getCode();
    const auto &Constants = Object.getConstants();
    const auto &GlobalStorage = Object.getGlobalStorage();
    const auto &Symbols = Object.getSymbols().getEntries();
    const auto &BaseReferences = Object.getBaseReferences().getEntries();
    const auto &DebugInfo = Object.getDebugInfo();

    const auto FormatVersion = core::ConfigManager::getVersion<core::ConfigKind::ObjectFormatVersion>();
    const auto InstructionSetVersion = core::ConfigManager::getVersion<core::ConfigKind::ObjectInstructionSetVersion>();
    if (Header.getMagic() != ObjectFileMagic || Header.getFormatVersion() != FormatVersion || Header.getInstructionSetVersion() != InstructionSetVersion || Header.getAddressBitWidth() != 64 || (Header.getByteOrder() != ObjectByteOrder::Little && Header.getByteOrder() != ObjectByteOrder::Big))
    {
      return ObjectValidationError{ObjectValidationErrorKind::InvalidHeader, 0};
    }
    if (hasTooManyRecords(Symbols.size()) || hasTooManyRecords(BaseReferences.size()) || (DebugInfo && hasTooManyRecords(DebugInfo->getSourceFiles().size())))
    {
      return ObjectValidationError{ObjectValidationErrorKind::TooManyRecords, 0};
    }

    if (!hasValidAlignment(Constants.getAlignment()))
    {
      return ObjectValidationError{ObjectValidationErrorKind::InvalidConstantSection, 0};
    }
    if (!hasValidAlignment(GlobalStorage.getAlignment()) || GlobalStorage.getInitialBytes().size() > GlobalStorage.getByteSize())
    {
      return ObjectValidationError{ObjectValidationErrorKind::InvalidGlobalStorage, 0};
    }

    std::unordered_set<std::string_view> ExternalNames;
    for (std::size_t Index = 0; Index < Symbols.size(); ++Index)
    {
      const auto &Symbol = Symbols[Index];
      if (Symbol.Name.empty() || (Symbol.Linkage != ObjectSymbolLinkage::Local && Symbol.Linkage != ObjectSymbolLinkage::Export && Symbol.Linkage != ObjectSymbolLinkage::Import))
      {
        return ObjectValidationError{ObjectValidationErrorKind::InvalidSymbol, Index};
      }
      if (Symbol.isImport())
      {
        if (Symbol.Definition)
        {
          return ObjectValidationError{ObjectValidationErrorKind::InvalidSymbol, Index};
        }
      }
      else
      {
        if (!Symbol.Definition)
        {
          return ObjectValidationError{ObjectValidationErrorKind::InvalidSymbol, Index};
        }
        const auto &Definition = *Symbol.Definition;
        std::uint64_t SectionSize = 0;
        switch (Definition.Section)
        {
        case ObjectSectionKind::Code:
          SectionSize = Code.byteSize();
          break;
        case ObjectSectionKind::Constants:
          SectionSize = Constants.byteSize();
          break;
        case ObjectSectionKind::GlobalStorage:
          SectionSize = GlobalStorage.getByteSize();
          break;
        default:
          return ObjectValidationError{ObjectValidationErrorKind::InvalidSymbol, Index};
        }
        if (Definition.ByteOffset > SectionSize || Definition.ByteSize > SectionSize - Definition.ByteOffset)
        {
          return ObjectValidationError{ObjectValidationErrorKind::InvalidSymbol, Index};
        }
      }
      if (Symbol.Linkage != ObjectSymbolLinkage::Local && !ExternalNames.insert(Symbol.Name).second)
      {
        return ObjectValidationError{ObjectValidationErrorKind::InvalidSymbol, Index};
      }
    }

    for (std::uint32_t Index = 0; Index < ObjectBaseReferenceTable::ReservedSlotCount; ++Index)
    {
      if (Index >= BaseReferences.size() || BaseReferences[Index].SymbolIndex)
      {
        return ObjectValidationError{ObjectValidationErrorKind::InvalidBaseReference, Index};
      }
    }
    for (std::size_t Index = ObjectBaseReferenceTable::ReservedSlotCount; Index < BaseReferences.size(); ++Index)
    {
      const auto &Reference = BaseReferences[Index];
      if (!Reference.SymbolIndex || *Reference.SymbolIndex >= Symbols.size())
      {
        return ObjectValidationError{ObjectValidationErrorKind::InvalidBaseReference, Index};
      }
    }

    if (DebugInfo)
    {
      const auto &Lines = DebugInfo->getLines();
      for (std::size_t Index = 0; Index < Lines.size(); ++Index)
      {
        const auto &Line = Lines[Index];
        if (Line.FileIndex >= DebugInfo->getSourceFiles().size() || Line.Line == 0 || Line.Column == 0)
        {
          return ObjectValidationError{ObjectValidationErrorKind::InvalidDebugInfo, Index};
        }
        if (Line.CodeOffset >= Code.byteSize())
        {
          return ObjectValidationError{ObjectValidationErrorKind::InvalidDebugInfo, Index};
        }
      }
    }
    return std::nullopt;
  }
} // namespace ink::vm
