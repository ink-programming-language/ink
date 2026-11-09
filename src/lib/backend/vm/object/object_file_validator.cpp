#include "ink/backend/vm/object/object_file_validator.h"

#include "ink/core/config/config_manager.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/backend/vm/object/object_file.h"

#include <spdlog/fmt/fmt.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>
#include <unordered_set>

namespace ink::vm
{
  namespace
  {
    [[noreturn]] void failValidation(const core::DiagnosticEngine &Diagnostics, std::string_view Context, std::string_view Description)
    {
      Diagnostics.reportICE<core::DiagnosticKind::VMObjectValidationFailed>({}, Context, Description);
    }

    [[noreturn]] void failRecord(const core::DiagnosticEngine &Diagnostics, std::string_view Kind, std::uint64_t Index, std::string_view Description)
    {
      failValidation(Diagnostics, fmt::format("{} #{}", Kind, Index), Description);
    }

    constexpr bool hasTooManyRecords(std::size_t Count) noexcept
    {
      return Count != 0 && Count - 1 > std::numeric_limits<std::uint32_t>::max();
    }

    constexpr bool hasValidAlignment(std::uint64_t Alignment) noexcept
    {
      return Alignment != 0 && (Alignment & (Alignment - 1)) == 0;
    }
  } // namespace

  void ObjectFileValidator::validate(const ObjectFile &Object, const core::DiagnosticEngine &Diagnostics) noexcept
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
    if (Header.getMagic() != ObjectFileMagic)
    {
      failValidation(Diagnostics, "header", "magic is invalid");
    }
    if (Header.getFormatVersion() != FormatVersion)
    {
      failValidation(Diagnostics, "header", "object format version is unsupported");
    }
    if (Header.getInstructionSetVersion() != InstructionSetVersion)
    {
      failValidation(Diagnostics, "header", "instruction-set version is unsupported");
    }
    if (Header.getAddressBitWidth() != 64)
    {
      failValidation(Diagnostics, "header", "address width must be 64 bits");
    }
    if (Header.getByteOrder() != ObjectByteOrder::Little && Header.getByteOrder() != ObjectByteOrder::Big)
    {
      failValidation(Diagnostics, "header", "byte order is invalid");
    }
    if (hasTooManyRecords(Symbols.size()))
    {
      failValidation(Diagnostics, "symbol table", "record count exceeds the u32 index range");
    }
    if (hasTooManyRecords(BaseReferences.size()))
    {
      failValidation(Diagnostics, "base reference table", "record count exceeds the u32 index range");
    }
    if (DebugInfo && hasTooManyRecords(DebugInfo->getSourceFiles().size()))
    {
      failValidation(Diagnostics, "debug source file table", "record count exceeds the u32 index range");
    }

    if (!hasValidAlignment(Constants.getAlignment()))
    {
      failValidation(Diagnostics, "constants", "alignment must be a nonzero power of two");
    }
    if (!hasValidAlignment(GlobalStorage.getAlignment()))
    {
      failValidation(Diagnostics, "global storage", "alignment must be a nonzero power of two");
    }
    if (GlobalStorage.getInitialBytes().size() > GlobalStorage.getByteSize())
    {
      failValidation(Diagnostics, "global storage", "initializer exceeds the declared storage size");
    }

    const core::NamePool &Names = core::NamePool::instance();
    std::unordered_set<core::Name> ExternalNames;
    for (std::size_t Index = 0; Index < Symbols.size(); ++Index)
    {
      const auto &Symbol = Symbols[Index];
      if (!Names.contains(Symbol.Name))
      {
        failRecord(Diagnostics, "symbol", Index, "name is not registered in the name pool");
      }
      if (Symbol.Linkage != ObjectSymbolLinkage::Local && Symbol.Linkage != ObjectSymbolLinkage::Export && Symbol.Linkage != ObjectSymbolLinkage::Import)
      {
        failRecord(Diagnostics, "symbol", Index, "linkage is invalid");
      }
      if (Symbol.isImport())
      {
        if (Symbol.Definition)
        {
          failRecord(Diagnostics, "symbol", Index, "import must not have a definition");
        }
      }
      else
      {
        if (!Symbol.Definition)
        {
          failRecord(Diagnostics, "symbol", Index, "local or exported symbol requires a definition");
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
          failRecord(Diagnostics, "symbol", Index, "definition section is invalid");
        }
        if (Definition.ByteOffset > SectionSize || Definition.ByteSize > SectionSize - Definition.ByteOffset)
        {
          failRecord(Diagnostics, "symbol", Index, "definition range is outside its section");
        }
      }
      if (Symbol.Linkage != ObjectSymbolLinkage::Local && !ExternalNames.insert(Symbol.Name).second)
      {
        failRecord(Diagnostics, "symbol", Index, "external name is duplicated");
      }
    }

    for (std::size_t Index = 0; Index < BaseReferences.size(); ++Index)
    {
      const auto &Reference = BaseReferences[Index];
      if (Reference.SymbolIndex >= Symbols.size())
      {
        failRecord(Diagnostics, "base reference", Index, "symbol index is outside the symbol table");
      }
    }

    if (DebugInfo)
    {
      const auto &Lines = DebugInfo->getLines();
      for (std::size_t Index = 0; Index < Lines.size(); ++Index)
      {
        const auto &Line = Lines[Index];
        if (Line.FileIndex >= DebugInfo->getSourceFiles().size())
        {
          failRecord(Diagnostics, "debug line", Index, "source file index is outside the source file table");
        }
        if (Line.Line == 0)
        {
          failRecord(Diagnostics, "debug line", Index, "line must be one-based");
        }
        if (Line.Column == 0)
        {
          failRecord(Diagnostics, "debug line", Index, "column must be one-based");
        }
        if (Line.CodeOffset >= Code.byteSize())
        {
          failRecord(Diagnostics, "debug line", Index, "code offset is outside the code section");
        }
      }
    }
  }
} // namespace ink::vm
