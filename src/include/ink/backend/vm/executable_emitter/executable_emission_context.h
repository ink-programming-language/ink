#pragma once

#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/instruction/register.h"
#include "ink/core/name/name.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ink::core
{
    class DiagnosticEngine;
} // namespace ink::core

namespace ink::vm
{
    class MachineModule;

    // Owns the shared image, module symbols, and relocation state for one executable emission.
    class ExecutableEmissionContext
    {
        public:
            struct SymbolMetadata
            {
                    ExecutableSymbolKind Kind = ExecutableSymbolKind::Function;
                    DataSymbolInfo Data{};
                    FunctionSymbolInfo Function{};

                    constexpr bool operator==(const SymbolMetadata& Other) const noexcept = default;
            };

            struct StaticRelocationSymbol
            {
                    std::uint64_t ImageOffset = 0;
                    core::Name Name = core::NAME_None;
                    std::uint64_t InstructionOffset = 0;
                    std::int64_t Addend = 0;
                    RegisterId Register = RegisterId::PC;
                    RelocationKind Kind = RelocationKind::AbsoluteAddress;
            };

            // The borrowed diagnostic engine must outlive this context and every emitter that borrows it.
            explicit ExecutableEmissionContext(const core::DiagnosticEngine& Diagnostics) noexcept;

            const core::DiagnosticEngine& getDiagnostics() const noexcept
            {
                return Diagnostics;
            }

            const ExecutableFile& getExecutable() const noexcept
            {
                return Executable;
            }

            void setEntryOffset(std::uint64_t ImageOffset) noexcept
            {
                Executable.getEntryOffset() = ImageOffset;
            }

            std::uint64_t byteSize() const noexcept
            {
                return Executable.byteSize();
            }

            [[nodiscard]] ExecutableFile takeExecutable();
            void reset();

            // Imports are declared names without a function or global definition in this module; providers may be Ink libraries or native stubs.
            // Resolved image offsets are retained until reset.
            void scanUnResolvedSymbol(const MachineModule& Module);
            void resolveSymbol(core::Name Name, std::uint64_t ImageOffset);
            void resolveSymbol(core::Name Name, std::uint64_t ImageOffset, SymbolMetadata Metadata);
            std::optional<std::uint64_t> resolvedSymbolOffset(core::Name Name) const noexcept;
            std::optional<SymbolMetadata> findDefinedSymbolMetadata(core::Name Name) const noexcept;
            bool hasUnResolvedSymbol(core::Name Name) const noexcept;

            bool isImportSymbol(core::Name Name) const noexcept;
            std::optional<DynamicImportSymbol> findImportSymbol(core::Name Name) const noexcept;

            // Appends Count readable bytes from outside the image; zero Count permits a null source.
            void pad(const void* Src, std::size_t Count);
            void padZero(std::size_t Count);
            void padAlignment(std::size_t Alignment);
            void padInteger(std::uint64_t Bits, std::uint32_t Width);

            // Replaces an existing integer field; ImageOffset and Width must identify writable image bytes.
            void patchInteger(std::uint64_t ImageOffset, std::uint64_t Bits, std::uint32_t Width);

            // InstructionOffset is the instruction's image offset for PC-relative references; ZERO ignores it.
            std::uint64_t resolveRegisterRelativeOffset(RegisterId Register, std::uint64_t SymbolOffset, std::int64_t Addend, std::uint64_t InstructionOffset) const;
            std::uint32_t getOrAddImportSymbol(const DynamicImportSymbol& Import);
            void addStaticRelocation(StaticRelocationSymbol Relocation);
            void addImageBaseAddressRelocation(ImageBaseAddressRelocation Relocation);
            void addDynamicImportSymbolRelocation(DynamicImportSymbolRelocation Relocation);
            void addExportSymbol(ExportSymbol Symbol);

            // Every referenced VM symbol must have an image offset after module layout.
            void resolveStaticRelocations();

        private:
            std::uint64_t resolveSymbolOffset(std::uint64_t SymbolOffset, std::int64_t Addend) const;

            const core::DiagnosticEngine& Diagnostics;
            ExecutableFile Executable;
            std::unordered_map<core::Name, std::uint64_t> UnresolvedSymbol;
            std::unordered_map<core::Name, std::uint64_t> ResolvedSymbol;
            std::unordered_map<core::Name, SymbolMetadata> DefinedSymbolMetadata;
            std::unordered_map<core::Name, SymbolMetadata> ImportSymbols;
            std::vector<StaticRelocationSymbol> PendingStaticRelocationSymbol;
    };
} // namespace ink::vm
