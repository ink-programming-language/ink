#include "ink/backend/vm/executable/executable_file_validator.h"

#include "ink/backend/vm/executable/executable_file.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"

#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ink::vm
{
  namespace
  {
    struct RelocationRange
    {
        std::uint64_t Begin;
        std::uint64_t End;
        std::uint64_t RecordIndex;
    };

    [[noreturn]] void failValidation(const core::DiagnosticEngine &Diagnostics, std::string_view Context, std::string_view Description)
    {
      Diagnostics.reportICE<core::DiagnosticKind::VMExecutableValidationFailed>({}, Context, Description);
    }

    [[noreturn]] void failRecord(const core::DiagnosticEngine &Diagnostics, std::string_view Kind, std::uint64_t Index, std::string_view Description)
    {
      failValidation(Diagnostics, fmt::format("{} #{}", Kind, Index), Description);
    }

    constexpr bool hasTooManyRecords(std::size_t Count) noexcept
    {
      return Count != 0 && Count - 1 > std::numeric_limits<std::uint32_t>::max();
    }

    constexpr bool hasValidProvider(NativeSymbolProvider Provider) noexcept
    {
      return Provider == NativeSymbolProvider::Native || Provider == NativeSymbolProvider::SystemApi;
    }

    void validateSections(const ExecutableFile &Executable, const core::DiagnosticEngine &Diagnostics)
    {
      const auto &Sections = Executable.getSections();
      if (Sections.empty())
      {
        return;
      }
      std::uint64_t NextOffset = 0;
      bool EntryInCode = false;
      for (std::size_t Index = 0; Index < Sections.size(); ++Index)
      {
        const ExecutableSection &Section = Sections[Index];
        if (Section.Kind != ExecutableSectionKind::Code && Section.Kind != ExecutableSectionKind::ReadOnlyData && Section.Kind != ExecutableSectionKind::WritableData)
        {
          failRecord(Diagnostics, "section", Index, "kind is invalid");
        }
        if (Section.ByteSize == 0)
        {
          failRecord(Diagnostics, "section", Index, "section is empty");
        }
        if (Section.ImageOffset != NextOffset)
        {
          failRecord(Diagnostics, "section", Index, "sections must be ordered, contiguous, and nonoverlapping");
        }
        if (Section.ImageOffset > Executable.byteSize() || Section.ByteSize > Executable.byteSize() - Section.ImageOffset)
        {
          failRecord(Diagnostics, "section", Index, "section is outside the image");
        }
        NextOffset = Section.ImageOffset + Section.ByteSize;
        if (Section.Kind == ExecutableSectionKind::Code && Executable.getEntryOffset() >= Section.ImageOffset && Executable.getEntryOffset() < NextOffset)
        {
          EntryInCode = true;
        }
      }
      if (NextOffset != Executable.byteSize())
      {
        failValidation(Diagnostics, "section table", "sections do not cover the complete image");
      }
      if (!EntryInCode)
      {
        failValidation(Diagnostics, "image", "entry offset is outside code sections");
      }
    }

    void validateImports(const std::vector<NativeFunctionImport> &Imports, const core::DiagnosticEngine &Diagnostics)
    {
      const core::NamePool &Names = core::NamePool::instance();
      std::unordered_map<NativeSymbolProvider, std::unordered_set<core::Name>> NamesByProvider;
      for (std::size_t Index = 0; Index < Imports.size(); ++Index)
      {
        const NativeFunctionImport &Import = Imports[Index];
        if (!Names.contains(Import.Name))
        {
          failRecord(Diagnostics, "native function import", Index, "name is not registered in the name pool");
        }
        if (!hasValidProvider(Import.Provider))
        {
          failRecord(Diagnostics, "native function import", Index, "provider is invalid");
        }
        if (Import.CallKind != NativeCallKind::C && Import.CallKind != NativeCallKind::InkNative)
        {
          failRecord(Diagnostics, "native function import", Index, "call kind is invalid");
        }
        if (!NamesByProvider[Import.Provider].insert(Import.Name).second)
        {
          failRecord(Diagnostics, "native function import", Index, "provider and name are duplicated");
        }
      }
    }

    void validateImports(const std::vector<NativeDataImport> &Imports, const core::DiagnosticEngine &Diagnostics)
    {
      const core::NamePool &Names = core::NamePool::instance();
      std::unordered_set<core::Name> ImportedNames;
      for (std::size_t Index = 0; Index < Imports.size(); ++Index)
      {
        const NativeDataImport &Import = Imports[Index];
        if (!Names.contains(Import.Name))
        {
          failRecord(Diagnostics, "native data import", Index, "name is not registered in the name pool");
        }
        if (!ImportedNames.insert(Import.Name).second)
        {
          failRecord(Diagnostics, "native data import", Index, "name is duplicated");
        }
      }
    }
  } // namespace

  void ExecutableFileValidator::validate(const ExecutableFile &Executable, const core::DiagnosticEngine &Diagnostics) noexcept
  {
    const auto &Functions = Executable.getNativeFunctions();
    const auto &Data = Executable.getNativeData();
    const auto &Relocations = Executable.getNativeDataRelocations();
    const std::uint64_t ImageSize = static_cast<std::uint64_t>(Executable.getImage().size());
    if (Executable.getEntryOffset() >= ImageSize)
    {
      failValidation(Diagnostics, "image", "entry offset is outside the image");
    }
    if (Executable.getByteOrder() != ByteOrder::Little && Executable.getByteOrder() != ByteOrder::Big)
    {
      failValidation(Diagnostics, "image", "byte order is invalid");
    }
    validateSections(Executable, Diagnostics);
    if (hasTooManyRecords(Functions.size()))
    {
      failValidation(Diagnostics, "native function table", "import count exceeds the u32 index range");
    }
    if (hasTooManyRecords(Data.size()))
    {
      failValidation(Diagnostics, "native data table", "import count exceeds the u32 index range");
    }

    validateImports(Functions, Diagnostics);
    validateImports(Data, Diagnostics);

    std::vector<RelocationRange> Ranges;
    Ranges.reserve(Relocations.size());
    for (std::size_t Index = 0; Index < Relocations.size(); ++Index)
    {
      const NativeDataRelocation &Relocation = Relocations[Index];
      if (Relocation.SymbolIndex >= Data.size())
      {
        failRecord(Diagnostics, "native data relocation", Index, "symbol index does not name a native data import");
      }
      if (Relocation.ImageOffset > ImageSize || NativeDataRelocation::ByteSize > ImageSize - Relocation.ImageOffset)
      {
        failRecord(Diagnostics, "native data relocation", Index, "address field is outside the image");
      }
      Ranges.push_back({Relocation.ImageOffset, Relocation.ImageOffset + NativeDataRelocation::ByteSize, Index});
    }
    const auto ByOffset = [](const RelocationRange &LHS, const RelocationRange &RHS)
    {
      return LHS.Begin < RHS.Begin;
    };
    std::sort(Ranges.begin(), Ranges.end(), ByOffset);
    for (std::size_t Index = 1; Index < Ranges.size(); ++Index)
    {
      if (Ranges[Index].Begin < Ranges[Index - 1].End)
      {
        const std::uint64_t RecordIndex = std::max(Ranges[Index - 1].RecordIndex, Ranges[Index].RecordIndex);
        const std::uint64_t OtherIndex = std::min(Ranges[Index - 1].RecordIndex, Ranges[Index].RecordIndex);
        failRecord(Diagnostics, "native data relocation", RecordIndex, fmt::format("address field overlaps native data relocation #{}", OtherIndex));
      }
    }
  }
} // namespace ink::vm
