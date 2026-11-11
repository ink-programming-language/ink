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

    void validateImports(const std::vector<DynamicImportSymbol> &Imports, const core::DiagnosticEngine &Diagnostics)
    {
      const core::NamePool &Names = core::NamePool::instance();
      std::unordered_map<NativeSymbolProvider, std::unordered_set<core::Name>> NamesByProvider;
      for (std::size_t Index = 0; Index < Imports.size(); ++Index)
      {
        const DynamicImportSymbol &Import = Imports[Index];
        if (!Names.contains(Import.Name))
        {
          failRecord(Diagnostics, "dynamic import symbol", Index, "name is not registered in the name pool");
        }
        if (!hasValidProvider(Import.Provider))
        {
          failRecord(Diagnostics, "dynamic import symbol", Index, "provider is invalid");
        }
        if (!NamesByProvider[Import.Provider].insert(Import.Name).second)
        {
          failRecord(Diagnostics, "dynamic import symbol", Index, "provider and name are duplicated");
        }
      }
    }

    void validateExports(const ExecutableFile &Executable, const core::DiagnosticEngine &Diagnostics)
    {
      const core::NamePool &Names = core::NamePool::instance();
      std::unordered_set<core::Name> ExportedNames;
      const auto &Exports = Executable.getExportSymbols();
      for (std::size_t Index = 0; Index < Exports.size(); ++Index)
      {
        const ExportSymbol &Export = Exports[Index];
        if (!Names.contains(Export.Name))
        {
          failRecord(Diagnostics, "export symbol", Index, "name is not registered in the name pool");
        }
        if (!ExportedNames.insert(Export.Name).second)
        {
          failRecord(Diagnostics, "export symbol", Index, "name is duplicated");
        }
        if (Export.Offset >= Executable.byteSize())
        {
          failRecord(Diagnostics, "export symbol", Index, "offset is outside the image");
        }
      }
    }
  } // namespace

  void ExecutableFileValidator::validate(const ExecutableFile &Executable, const core::DiagnosticEngine &Diagnostics) noexcept
  {
    const auto &Imports = Executable.getDynamicImportSymbols();
    const auto &Relocations = Executable.getDynamicImportSymbolRelocations();
    const std::uint64_t ImageSize = static_cast<std::uint64_t>(Executable.getImage().size());
    if (Executable.getEntryOffset() >= ImageSize)
    {
      failValidation(Diagnostics, "image", "entry offset is outside the image");
    }
    if (Executable.getByteOrder() != ByteOrder::Little && Executable.getByteOrder() != ByteOrder::Big)
    {
      failValidation(Diagnostics, "image", "byte order is invalid");
    }
    if (hasTooManyRecords(Imports.size()))
    {
      failValidation(Diagnostics, "dynamic import symbol table", "import count exceeds the u32 index range");
    }

    validateImports(Imports, Diagnostics);
    validateExports(Executable, Diagnostics);

    std::vector<RelocationRange> Ranges;
    Ranges.reserve(Relocations.size());
    for (std::size_t Index = 0; Index < Relocations.size(); ++Index)
    {
      const DynamicImportSymbolRelocation &Relocation = Relocations[Index];
      if (Relocation.SymbolIndex >= Imports.size())
      {
        failRecord(Diagnostics, "dynamic import symbol relocation", Index, "symbol index does not name a dynamic import symbol");
      }
      if (Relocation.ImageOffset > ImageSize || DynamicImportSymbolRelocation::ByteSize > ImageSize - Relocation.ImageOffset)
      {
        failRecord(Diagnostics, "dynamic import symbol relocation", Index, "address field is outside the image");
      }
      Ranges.push_back({Relocation.ImageOffset, Relocation.ImageOffset + DynamicImportSymbolRelocation::ByteSize, Index});
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
        failRecord(Diagnostics, "dynamic import symbol relocation", RecordIndex, fmt::format("address field overlaps dynamic import symbol relocation #{}", OtherIndex));
      }
    }
  }
} // namespace ink::vm
