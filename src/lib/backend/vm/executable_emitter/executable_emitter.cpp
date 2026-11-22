#include "ink/backend/vm/executable_emitter/executable_emitter.h"

#include "ink/backend/vm/module/machine_module.h"
#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/core/support/core_define.h"

#include <unordered_set>

namespace ink::vm
{
    ExecutableEmitter::ExecutableEmitter(const core::DiagnosticEngine& Diagnostics) noexcept
        : Context(Diagnostics),
          FunctionEmitter(Context)
    {
    }

    ExecutableFile ExecutableEmitter::emit(const MachineModule& Module, core::Name EntryFunctionName, std::span<const core::Name> ExportNames)
    {
        Context.reset();
        scanUnResolvedSymbol(Module);
        padGlobals(Module);
        padFunction(Module);
        resolveStaticRelocations();
        resolveEntryOffset(Module, EntryFunctionName);
        generateExportSymbols(ExportNames);
        return Context.takeExecutable();
    }

    void ExecutableEmitter::scanUnResolvedSymbol(const MachineModule& Module)
    {
        Context.scanUnResolvedSymbol(Module);
    }

    void ExecutableEmitter::padGlobals(const MachineModule& Module)
    {
        const auto& Diagnostics = Context.getDiagnostics();
        for (const MachineGlobalVariable& Global : Module.globalVariables())
        {
            const std::uint64_t Remainder = Context.byteSize() % Global.alignment();
            const std::uint64_t Padding = Remainder == 0 ? 0 : Global.alignment() - Remainder;
            Context.padZero(static_cast<std::size_t>(Padding));
            const std::uint64_t Offset = Context.byteSize();
            const auto InitialBytes = Global.initialBytes();
            Context.pad(InitialBytes.data(), InitialBytes.size());
            Context.padZero(static_cast<std::size_t>(Global.byteSize() - InitialBytes.size()));
            Context.resolveSymbol(Global.name(), Offset, {ExecutableSymbolKind::Data, DataSymbolInfo{Global.isConstant()}, {}});
            for (const MachineGlobalRelocation& Relocation : Global.relocations())
            {
                REPORT_ICE_IF(!core::NamePool::instance().contains(Relocation.Symbol), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "global relocation", "symbol name must be registered and nonempty");
                REPORT_ICE_IF(Global.alignment() < MachineGlobalRelocation::ByteSize || Relocation.ByteOffset % MachineGlobalRelocation::ByteSize != 0, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "global relocation", "global relocation field must be eight-byte aligned");
                REPORT_ICE_IF(Relocation.ByteOffset > Global.byteSize() || MachineGlobalRelocation::ByteSize > Global.byteSize() - Relocation.ByteOffset, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "global relocation", "global relocation field is outside its global variable");
                const std::uint64_t FieldOffset = Offset + Relocation.ByteOffset;
                REPORT_ICE_IF(FieldOffset % MachineGlobalRelocation::ByteSize != 0, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "global relocation", "global relocation image offset must be eight-byte aligned");
                const auto Position = Context.resolvedSymbolOffset(Relocation.Symbol);
                const bool Import = Context.isImportSymbol(Relocation.Symbol);
                REPORT_ICE_IF(Import && (Position || Context.hasUnResolvedSymbol(Relocation.Symbol)), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "global relocation", "symbol has both image and import definitions");
                if (Position)
                {
                    Context.addImageBaseAddressRelocation({FieldOffset, *Position, Relocation.Addend, RelocationKind::AbsoluteAddress});
                }
                else if (Context.hasUnResolvedSymbol(Relocation.Symbol))
                {
                    Context.addStaticRelocation({FieldOffset, Relocation.Symbol, 0, Relocation.Addend, RegisterId::ZERO, RelocationKind::AbsoluteAddress});
                }
                else
                {
                    const auto ImportSymbol = Context.findImportSymbol(Relocation.Symbol);
                    REPORT_ICE_IF(!ImportSymbol, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "global relocation", "symbol has no resolved VM offset or unresolved symbol declaration");
                    const auto Index = Context.getOrAddImportSymbol(*ImportSymbol);
                    Context.addDynamicImportSymbolRelocation({FieldOffset, Index, Relocation.Addend, RelocationKind::AbsoluteAddress});
                }
            }
        }
    }

    void ExecutableEmitter::padFunction(const MachineModule& Module)
    {
        for (const MachineFunction& Function : Module.functions())
        {
            Context.padAlignment(InstructionEncoding::WordBytes);
            Context.resolveSymbol(Function.name(), Context.byteSize(), {ExecutableSymbolKind::Function, {}, FunctionSymbolInfo{}});
            FunctionEmitter.emit(Function);
        }
    }

    void ExecutableEmitter::resolveStaticRelocations()
    {
        Context.resolveStaticRelocations();
    }

    void ExecutableEmitter::resolveEntryOffset(const MachineModule& Module, core::Name EntryFunctionName)
    {
        const auto& Diagnostics = Context.getDiagnostics();
        REPORT_ICE_IF(!core::NamePool::instance().contains(EntryFunctionName), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "entry point", "entry function name must be registered and nonempty");
        const MachineFunction* EntryFunction = nullptr;
        for (const MachineFunction& Function : Module.functions())
        {
            if (Function.name() == EntryFunctionName)
            {
                EntryFunction = &Function;
                break;
            }
        }
        REPORT_ICE_IF(!EntryFunction, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "entry point", "entry must name a defined VM function");
        bool HasInstructions = false;
        for (const MachineBasicBlock& Block : EntryFunction->basicBlocks())
        {
            if (Block.instructionCount() != 0)
            {
                HasInstructions = true;
                break;
            }
        }
        REPORT_ICE_IF(!HasInstructions, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "entry point", "entry function must contain at least one instruction");
        const auto Offset = Context.resolvedSymbolOffset(EntryFunctionName);
        REPORT_ICE_IF(!Offset || *Offset >= Context.byteSize(), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "entry point", "entry function has no instruction offset in the image");
        Context.setEntryOffset(*Offset);
    }

    void ExecutableEmitter::generateExportSymbols(std::span<const core::Name> ExportNames)
    {
        const auto& Diagnostics = Context.getDiagnostics();
        std::unordered_set<core::Name> ExportedNames;
        for (const core::Name Name : ExportNames)
        {
            REPORT_ICE_IF(!core::NamePool::instance().contains(Name), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "export symbol", "export symbol name must be registered and nonempty");
            REPORT_ICE_IF(!ExportedNames.insert(Name).second, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "export symbol", "export symbol name is duplicated");
            const auto Offset = Context.resolvedSymbolOffset(Name);
            REPORT_ICE_IF(!Offset, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "export symbol", "export symbol has no VM image offset");
            REPORT_ICE_IF(*Offset >= Context.byteSize(), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "export symbol", "export symbol offset is outside the image");
            const auto Metadata = Context.findDefinedSymbolMetadata(Name).value_or(ExecutableEmissionContext::SymbolMetadata{});
            Context.addExportSymbol({Name, *Offset, Metadata.Kind, Metadata.Data, Metadata.Function});
        }
    }
} // namespace ink::vm
