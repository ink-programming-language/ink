#include "ink/backend/vm/interpreter/loader.h"
#include "relocation_address.h"
#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/core/name/name_pool.h"
#include "ink/core/support/core_define.h"

#include <spdlog/fmt/fmt.h>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

#if INK_PLATFORM_BIG_ENDIAN
#error "Ink VM runtime requires a little-endian target"
#endif

namespace ink::vm
{
    namespace
    {
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

        constexpr bool isValidSymbolMetadata(ExecutableSymbolKind Kind, const DataSymbolInfo& Data, const FunctionSymbolInfo& Function) noexcept
        {
            if (!isValidSymbolKind(Kind))
            {
                return false;
            }
            if (Kind == ExecutableSymbolKind::Function)
            {
                return !Data.IsConstant && isValidCallingConvention(Function.Convention);
            }
            return Function == FunctionSymbolInfo{};
        }

        constexpr bool isValidRelocationKind(RelocationKind Kind) noexcept
        {
            return Kind == RelocationKind::AbsoluteAddress || Kind == RelocationKind::DirectCall;
        }

        constexpr bool isDirectCallHeader(std::uint64_t Header) noexcept
        {
            return Header == InstructionEncoding::encodeOpcode(Opcode::Calli) || Header == InstructionEncoding::encodeOpcode(Opcode::Callvmi);
        }

        bool reportLinkFailure(const core::DiagnosticEngine& Diagnostics, core::Name Name, std::size_t Index, std::string_view Reason)
        {
            const auto Context = fmt::format("dynamic import #{} ('{}')", Index, core::NamePool::instance().text(Name));
            Diagnostics.report<core::DiagnosticKind::VMLibraryLinkFailed>({}, Context, Reason);
            return false;
        }
    } // namespace

    Loader::Loader(vm::Interpreter& Interpreter) noexcept
        : Interpreter(Interpreter)
    {
    }

    /**
     * Bind an image already copied into runtime storage. This function does not allocate or grow CodeSegment.
     * CodeSegment is a writable view of the entire valid segment, including any earlier images or suffix.
     * ImageBase is the image's byte offset from CodeSegment.data(), not a host address.
     * Executable supplies the original image size, exports, imports, and relocations; none are modified.
     *
     * CodeSegment.data()
     *   |--- ImageBase bytes ---|--- ImageOffset bytes ---| uint64_t field | ...
     *                          ^ image start             ^ patch destination
     * Destination = CodeSegment.data() + ImageBase + Relocation.ImageOffset
     * Dynamic import: *Destination = SymbolAddresses[Relocation.SymbolIndex] + Relocation.Addend
     * Image-relative: *Destination = host(CodeSegment.data() + ImageBase + Relocation.TargetOffset) + Relocation.Addend
     *
     * Register exports first so the image can import its own symbols; later registrations replace names.
     * Resolve the complete import table once and cache by SymbolIndex, including imports with no relocation.
     * Dynamic imports read that cache; image-relative references use TargetOffset within this image, without
     * symbol lookup or interposition. TargetOffset may equal ImageSize to represent its one-past address.
     * Both kinds apply their signed Addend and write directly without a patch array.
     * Addend changes the stored address, not the destination. The actual field must be eight-byte aligned;
     * the uint64_t store relies on a little-endian target, enforced at compile time. No padding is inserted.
     *
     * Missing imports report a link diagnostic and return false. The caller must roll back exports and code;
     * Interpreter provides that transaction. Invalid bounds, indices, or arithmetic report fatal ICEs.
     * Overlaps are validated upstream. Only the copied code is patched; Executable remains reusable.
     */
    bool Loader::loadLibrary(std::span<std::uint8_t> CodeSegment, std::uint64_t ImageBase, ExecutableFile& Executable)
    {
        const auto& Diagnostics = Interpreter.getDiagnostics();
        const auto CodeSize = static_cast<std::uint64_t>(CodeSegment.size());
        const auto ImageSize = Executable.byteSize();
        const auto CodeAddress = reinterpret_cast<std::uintptr_t>(CodeSegment.data());
        const auto ImageAddress = reinterpret_cast<std::uintptr_t>(Executable.getImage().data());
        const bool AliasesImage = CodeSize != 0 && ImageSize != 0 && (CodeAddress <= ImageAddress ? ImageAddress - CodeAddress < CodeSize : CodeAddress - ImageAddress < ImageSize);
        REPORT_ICE_IF(AliasesImage, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "image", "code segment aliases the unbound image");
        REPORT_ICE_IF(ImageBase > CodeSize || ImageSize > CodeSize - ImageBase, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "image", "loaded image is outside the code segment");
        for (const auto& Export : Executable.getExportSymbols())
        {
            REPORT_ICE_IF(Export.Offset >= ImageSize, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "export symbol", "offset is outside the image");
            REPORT_ICE_IF(!isValidSymbolMetadata(Export.Kind, Export.Data, Export.Function), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "export symbol", "symbol metadata is invalid");
            const auto Address = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(CodeSegment.data() + ImageBase + Export.Offset));
            Interpreter.ExportSymbolMap.insert_or_assign(Export.Name, RuntimeSymbol{Address, Export.Kind, SymbolProviderKind::VM, Export.Data, Export.Function});
        }

        const auto& Imports = Executable.getDynamicImportSymbols();
        std::vector<RuntimeSymbol> ResolvedImports;
        ResolvedImports.reserve(Imports.size());
        for (std::size_t Index = 0; Index < Imports.size(); ++Index)
        {
            const auto& Import = Imports[Index];
            REPORT_ICE_IF(!isValidSymbolMetadata(Import.Kind, Import.Data, Import.Function), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "dynamic import symbol", "symbol metadata is invalid");
            const auto Position = Interpreter.ExportSymbolMap.find(Import.Name);
            if (Position == Interpreter.ExportSymbolMap.end())
            {
                return reportLinkFailure(Diagnostics, Import.Name, Index, "symbol address was not found");
            }
            const RuntimeSymbol& Resolved = Position->second;
            if (Resolved.Kind != Import.Kind)
            {
                return reportLinkFailure(Diagnostics, Import.Name, Index, "symbol kind does not match export");
            }
            if (Import.Kind == ExecutableSymbolKind::Data && Resolved.Data != Import.Data)
            {
                return reportLinkFailure(Diagnostics, Import.Name, Index, "data symbol attributes do not match export");
            }
            if (Import.Kind == ExecutableSymbolKind::Function && Resolved.Function != Import.Function)
            {
                return reportLinkFailure(Diagnostics, Import.Name, Index, "function calling convention does not match export");
            }
            ResolvedImports.push_back(Resolved);
        }

        for (const auto& Relocation : Executable.getDynamicImportSymbolRelocations())
        {
            REPORT_ICE_IF(Relocation.SymbolIndex >= ResolvedImports.size(), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "dynamic import relocation", "symbol index is outside the import table");
            REPORT_ICE_IF(!isValidRelocationKind(Relocation.Kind), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "dynamic import relocation", "relocation kind is invalid");
            REPORT_ICE_IF(Relocation.ImageOffset > ImageSize || DynamicImportSymbolRelocation::ByteSize > ImageSize - Relocation.ImageOffset, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "dynamic import relocation", "address field is outside the image");
            const auto& Resolved = ResolvedImports[Relocation.SymbolIndex];
            const auto Address = detail::addAddressOffset(Resolved.Address, Relocation.Addend, Diagnostics);
            const auto CodeOffset = static_cast<std::size_t>(ImageBase + Relocation.ImageOffset);
            auto* Destination = CodeSegment.data() + CodeOffset;
            REPORT_ICE_IF(reinterpret_cast<std::uintptr_t>(Destination) % DynamicImportSymbolRelocation::ByteSize != 0, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "dynamic import relocation", "address field is not eight-byte aligned");
            if (Relocation.Kind == RelocationKind::DirectCall)
            {
                REPORT_ICE_IF(Resolved.Kind != ExecutableSymbolKind::Function, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "dynamic import relocation", "direct call relocation must reference a function symbol");
                REPORT_ICE_IF(Relocation.Addend != 0, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "dynamic import relocation", "direct call relocation must have a zero addend");
                REPORT_ICE_IF(Relocation.ImageOffset < InstructionEncoding::WordBytes, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "dynamic import relocation", "direct call relocation has no preceding instruction header");
                auto* Header = reinterpret_cast<std::uint64_t*>(Destination - InstructionEncoding::WordBytes);
                REPORT_ICE_IF(!isDirectCallHeader(*Header), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "dynamic import relocation", "direct call relocation does not follow a direct call instruction header");
                const Opcode TargetOpcode = Resolved.Provider == SymbolProviderKind::VM ? Opcode::Callvmi : Opcode::Calli;
                *Header = InstructionEncoding::encodeOpcode(TargetOpcode);
            }
            *reinterpret_cast<std::uint64_t*>(Destination) = Address;
        }
        for (const auto& Relocation : Executable.getImageBaseAddressRelocations())
        {
            REPORT_ICE_IF(!isValidRelocationKind(Relocation.Kind), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "image-base address relocation", "relocation kind is invalid");
            REPORT_ICE_IF(Relocation.ImageOffset > ImageSize || ImageBaseAddressRelocation::ByteSize > ImageSize - Relocation.ImageOffset, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "image-base address relocation", "address field is outside the image");
            REPORT_ICE_IF(Relocation.TargetOffset > ImageSize, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "image-base address relocation", "target offset is outside the image");
            const auto TargetAddress = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(CodeSegment.data() + ImageBase + Relocation.TargetOffset));
            const auto Address = detail::addAddressOffset(TargetAddress, Relocation.Addend, Diagnostics, "image-base address relocation");
            auto* Destination = CodeSegment.data() + ImageBase + Relocation.ImageOffset;
            REPORT_ICE_IF(reinterpret_cast<std::uintptr_t>(Destination) % ImageBaseAddressRelocation::ByteSize != 0, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "image-base address relocation", "address field is not eight-byte aligned");
            if (Relocation.Kind == RelocationKind::DirectCall)
            {
                REPORT_ICE_IF(Relocation.Addend != 0, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "image-base address relocation", "direct call relocation must have a zero addend");
                REPORT_ICE_IF(Relocation.ImageOffset < InstructionEncoding::WordBytes, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "image-base address relocation", "direct call relocation has no preceding instruction header");
                auto* Header = reinterpret_cast<std::uint64_t*>(Destination - InstructionEncoding::WordBytes);
                REPORT_ICE_IF(!isDirectCallHeader(*Header), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "image-base address relocation", "direct call relocation does not follow a direct call instruction header");
                *Header = InstructionEncoding::encodeOpcode(Opcode::Callvmi);
            }
            *reinterpret_cast<std::uint64_t*>(Destination) = Address;
        }
        return true;
    }
} // namespace ink::vm
