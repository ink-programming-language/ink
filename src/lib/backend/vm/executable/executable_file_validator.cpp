#include "ink/backend/vm/executable/executable_file_validator.h"

#include "ink/backend/vm/executable/executable_file.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"

#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>
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
                std::string_view Kind;
                std::size_t RecordOrder;
        };

        [[noreturn]] void failValidation(const core::DiagnosticEngine& Diagnostics, std::string_view Context, std::string_view Description)
        {
            Diagnostics.reportICE<core::DiagnosticKind::VMExecutableValidationFailed>({}, Context, Description);
        }

        [[noreturn]] void failRecord(const core::DiagnosticEngine& Diagnostics, std::string_view Kind, std::uint64_t Index, std::string_view Description)
        {
            failValidation(Diagnostics, fmt::format("{} #{}", Kind, Index), Description);
        }

        constexpr bool hasTooManyRecords(std::size_t Count) noexcept
        {
            return Count != 0 && Count - 1 > std::numeric_limits<std::uint32_t>::max();
        }

        constexpr bool isValidSymbolKind(ExecutableSymbolKind Kind) noexcept
        {
            return Kind == ExecutableSymbolKind::Function || Kind == ExecutableSymbolKind::Data;
        }

        constexpr bool isValidCallingConvention(CallingConvention Convention) noexcept
        {
            switch (Convention)
            {
            case CallingConvention::Default:
            case CallingConvention::CDecl:
            case CallingConvention::StdCall:
            case CallingConvention::FastCall:
            case CallingConvention::VectorCall:
                return true;
            }
            return false;
        }

        constexpr bool isValidRelocationKind(RelocationKind Kind) noexcept
        {
            return Kind == RelocationKind::AbsoluteAddress || Kind == RelocationKind::DirectCall;
        }

        void validateSymbolMetadata(std::string_view KindName, std::size_t Index, ExecutableSymbolKind Kind, const DataSymbolInfo& Data, const FunctionSymbolInfo& Function, const core::DiagnosticEngine& Diagnostics)
        {
            if (!isValidSymbolKind(Kind))
            {
                failRecord(Diagnostics, KindName, Index, "symbol kind is invalid");
            }
            if (Kind == ExecutableSymbolKind::Function)
            {
                if (Data.IsConstant)
                {
                    failRecord(Diagnostics, KindName, Index, "function symbol cannot be marked read-only");
                }
                if (!isValidCallingConvention(Function.Convention))
                {
                    failRecord(Diagnostics, KindName, Index, "function calling convention is invalid");
                }
            }
            else if (Function != FunctionSymbolInfo{})
            {
                failRecord(Diagnostics, KindName, Index, "data symbol cannot specify function call attributes");
            }
        }

        void validateImports(const std::vector<DynamicImportSymbol>& Imports, const core::DiagnosticEngine& Diagnostics)
        {
            const core::NamePool& Names = core::NamePool::instance();
            std::unordered_set<core::Name> ImportedNames;
            for (std::size_t Index = 0; Index < Imports.size(); ++Index)
            {
                const DynamicImportSymbol& Import = Imports[Index];
                if (!Names.contains(Import.Name))
                {
                    failRecord(Diagnostics, "dynamic import symbol", Index, "name is not registered in the name pool");
                }
                if (!ImportedNames.insert(Import.Name).second)
                {
                    failRecord(Diagnostics, "dynamic import symbol", Index, "name is duplicated");
                }
                validateSymbolMetadata("dynamic import symbol", Index, Import.Kind, Import.Data, Import.Function, Diagnostics);
            }
        }

        void validateExports(const ExecutableFile& Executable, const core::DiagnosticEngine& Diagnostics)
        {
            const core::NamePool& Names = core::NamePool::instance();
            std::unordered_set<core::Name> ExportedNames;
            const auto& Exports = Executable.getExportSymbols();
            for (std::size_t Index = 0; Index < Exports.size(); ++Index)
            {
                const ExportSymbol& Export = Exports[Index];
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
                validateSymbolMetadata("export symbol", Index, Export.Kind, Export.Data, Export.Function, Diagnostics);
            }
        }
    } // namespace

    void ExecutableFileValidator::validate(const ExecutableFile& Executable, const core::DiagnosticEngine& Diagnostics) noexcept
    {
        const auto& Imports = Executable.getDynamicImportSymbols();
        const auto& Relocations = Executable.getDynamicImportSymbolRelocations();
        const auto& ImageBaseRelocations = Executable.getImageBaseAddressRelocations();
        const std::uint64_t ImageSize = static_cast<std::uint64_t>(Executable.getImage().size());
        if (Executable.getEntryOffset() >= ImageSize)
        {
            failValidation(Diagnostics, "image", "entry offset is outside the image");
        }
        if (Executable.getEntryOffset() % sizeof(std::uint64_t) != 0)
        {
            failValidation(Diagnostics, "image", "entry offset is not eight-byte aligned");
        }
        if (hasTooManyRecords(Imports.size()))
        {
            failValidation(Diagnostics, "dynamic import symbol table", "import count exceeds the u32 index range");
        }

        validateImports(Imports, Diagnostics);
        validateExports(Executable, Diagnostics);

        std::vector<RelocationRange> Ranges;
        Ranges.reserve(Relocations.size() + ImageBaseRelocations.size());
        for (std::size_t Index = 0; Index < Relocations.size(); ++Index)
        {
            const DynamicImportSymbolRelocation& Relocation = Relocations[Index];
            if (Relocation.SymbolIndex >= Imports.size())
            {
                failRecord(Diagnostics, "dynamic import symbol relocation", Index, "symbol index does not name a dynamic import symbol");
            }
            if (!isValidRelocationKind(Relocation.Kind))
            {
                failRecord(Diagnostics, "dynamic import symbol relocation", Index, "relocation kind is invalid");
            }
            if (Relocation.ImageOffset > ImageSize || DynamicImportSymbolRelocation::ByteSize > ImageSize - Relocation.ImageOffset)
            {
                failRecord(Diagnostics, "dynamic import symbol relocation", Index, "address field is outside the image");
            }
            if (Relocation.Kind == RelocationKind::DirectCall)
            {
                if (Imports[Relocation.SymbolIndex].Kind != ExecutableSymbolKind::Function)
                {
                    failRecord(Diagnostics, "dynamic import symbol relocation", Index, "direct call relocation must reference a function symbol");
                }
                if (Relocation.Addend != 0)
                {
                    failRecord(Diagnostics, "dynamic import symbol relocation", Index, "direct call relocation must have a zero addend");
                }
                if (Relocation.ImageOffset < sizeof(std::uint64_t))
                {
                    failRecord(Diagnostics, "dynamic import symbol relocation", Index, "direct call relocation has no preceding instruction header");
                }
            }
            Ranges.push_back({Relocation.ImageOffset, Relocation.ImageOffset + DynamicImportSymbolRelocation::ByteSize, Index, "dynamic import symbol relocation", Ranges.size()});
        }
        for (std::size_t Index = 0; Index < ImageBaseRelocations.size(); ++Index)
        {
            const ImageBaseAddressRelocation& Relocation = ImageBaseRelocations[Index];
            if (!isValidRelocationKind(Relocation.Kind))
            {
                failRecord(Diagnostics, "image base address relocation", Index, "relocation kind is invalid");
            }
            if (Relocation.ImageOffset > ImageSize || ImageBaseAddressRelocation::ByteSize > ImageSize - Relocation.ImageOffset)
            {
                failRecord(Diagnostics, "image base address relocation", Index, "address field is outside the image");
            }
            if (Relocation.TargetOffset > ImageSize)
            {
                failRecord(Diagnostics, "image base address relocation", Index, "target offset exceeds the image end");
            }
            if (Relocation.Kind == RelocationKind::DirectCall)
            {
                if (Relocation.Addend != 0)
                {
                    failRecord(Diagnostics, "image base address relocation", Index, "direct call relocation must have a zero addend");
                }
                if (Relocation.ImageOffset < sizeof(std::uint64_t))
                {
                    failRecord(Diagnostics, "image base address relocation", Index, "direct call relocation has no preceding instruction header");
                }
            }
            Ranges.push_back({Relocation.ImageOffset, Relocation.ImageOffset + ImageBaseAddressRelocation::ByteSize, Index, "image base address relocation", Ranges.size()});
        }
        const auto ByOffset = [](const RelocationRange& LHS, const RelocationRange& RHS)
        {
            return LHS.Begin < RHS.Begin;
        };
        std::sort(Ranges.begin(), Ranges.end(), ByOffset);
        for (std::size_t Index = 1; Index < Ranges.size(); ++Index)
        {
            if (Ranges[Index].Begin < Ranges[Index - 1].End)
            {
                const auto& First = Ranges[Index - 1];
                const auto& Second = Ranges[Index];
                const auto& Record = First.RecordOrder > Second.RecordOrder ? First : Second;
                const auto& Other = First.RecordOrder > Second.RecordOrder ? Second : First;
                failRecord(Diagnostics, Record.Kind, Record.RecordIndex, fmt::format("address field overlaps {} #{}", Other.Kind, Other.RecordIndex));
            }
        }
        for (const auto& Range : Ranges)
        {
            if (Range.Begin % sizeof(std::uint64_t) != 0)
            {
                failRecord(Diagnostics, Range.Kind, Range.RecordIndex, "address field is not eight-byte aligned");
            }
        }
    }
} // namespace ink::vm
