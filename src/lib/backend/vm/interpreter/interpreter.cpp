#include "ink/backend/vm/interpreter/interpreter.h"
#include "load_transaction.h"
#include "relocation_address.h"
#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/backend/vm/vm_runtime/runtime.h"
#include "ink/core/config/config_manager.h"
#include "ink/core/support/core_define.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <utility>

namespace ink::vm
{
    Interpreter::Interpreter() noexcept
        : Loader(*this)
    {
        initialize();
    }

    Interpreter::~Interpreter() = default;

    void Interpreter::initialize()
    {
        if (!Initialized)
        {
            reserveCodeSegment(core::ConfigManager::getSize<core::ConfigKind::VMInitialCodeSegmentBytes>());
            const auto StackBytes = core::ConfigManager::getSize<core::ConfigKind::VMStackBytes>();
            auto* const Stack = StackBytes != 0 ? static_cast<std::uint8_t*>(std::malloc(StackBytes)) : nullptr;
            REPORT_ICE_IF(StackBytes != 0 && !Stack, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "stack", "runtime storage allocation failed");
            StackAllocation.reset(Stack);
            StackSize = StackBytes;
            Initialized = true;
        }
        for (const auto& Symbol : NativeSymbolAddresses)
        {
            const auto Address = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(Symbol.Address));
            NativeStubAddresses.insert(Address);
            ExportSymbolMap.try_emplace(Symbol.Name, RuntimeSymbol{Address, ExecutableSymbolKind::Function, SymbolProviderKind::Native});
        }
    }

    bool Interpreter::execute(const void* Entry)
    {
        const auto EntryAddress = reinterpret_cast<std::uintptr_t>(Entry);
        const auto CodeAddress = reinterpret_cast<std::uintptr_t>(CodeSegment);
        const bool OutsideCodeSegment = EntryAddress < CodeAddress || EntryAddress - CodeAddress >= CodeSegmentSize;
        const bool MisalignedEntry = !OutsideCodeSegment && (EntryAddress - CodeAddress) % InstructionEncoding::WordBytes != 0;
        REPORT_IF(OutsideCodeSegment, Diagnostics, core::DiagnosticKind::VMExecutionFailed, {}, "entry", "entry address is outside the loaded code segment");
        REPORT_IF(MisalignedEntry, Diagnostics, core::DiagnosticKind::VMExecutionFailed, {}, "entry", "entry address is not aligned to an instruction word");
        if (OutsideCodeSegment || MisalignedEntry)
        {
            return false;
        }
        Frame = vm::Frame{};
        const auto StackAddress = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(StackAllocation.get()));
        WRITE_REGISTER(RegisterId::BP, Frame, StackAddress);
        WRITE_REGISTER(RegisterId::SP, Frame, StackAddress);
        WRITE_REGISTER(RegisterId::PC, Frame, static_cast<std::uint64_t>(EntryAddress));
        return dispatch();
    }

    void* Interpreter::resolveSymbolAddress(core::Name Name) const noexcept
    {
        const auto Position = ExportSymbolMap.find(Name);
        return Position == ExportSymbolMap.end() ? nullptr : reinterpret_cast<void*>(static_cast<std::uintptr_t>(Position->second.Address));
    }

    /**
     * Ensure at least RequiredBytes total capacity, not an additional byte count; CodeSegmentSize is unchanged.
     * Existing capacity is reused. Growth normally doubles capacity without overflowing and rounds up to
     * complete uint64_t words. The runtime allocation contains live integer objects so instruction words
     * can be loaded directly; global data continues to be accessed through its byte representation.
     * A replacement allocation preserves existing words, then repatches their embedded host addresses.
     *
     * Growth precedes the load transaction, so a subsequent link failure can retain the increased capacity
     * and repaired addresses. External cached pointers are not repaired and must be queried again after a move.
     */
    void Interpreter::reserveCodeSegment(std::size_t RequiredBytes)
    {
        if (RequiredBytes <= CodeSegmentCapacity)
        {
            return;
        }
        constexpr auto MaximumCapacity = std::numeric_limits<std::size_t>::max() - (InstructionEncoding::WordBytes - 1);
        REPORT_ICE_IF(RequiredBytes > MaximumCapacity, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "code segment", "aligned runtime storage size overflows");
        const auto RequestedCapacity = CodeSegmentCapacity <= MaximumCapacity / 2 ? std::max(RequiredBytes, CodeSegmentCapacity * 2) : RequiredBytes;
        const auto NewCapacity = (RequestedCapacity + InstructionEncoding::WordBytes - 1) / InstructionEncoding::WordBytes * InstructionEncoding::WordBytes;
        const auto OldAddress = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(CodeSegment));
        std::vector<std::uint64_t> Replacement(NewCapacity / InstructionEncoding::WordBytes);
        std::copy_n(CodeWords.begin(), (CodeSegmentSize + InstructionEncoding::WordBytes - 1) / InstructionEncoding::WordBytes, Replacement.begin());
        CodeWords.swap(Replacement);
        CodeSegment = reinterpret_cast<std::uint8_t*>(CodeWords.data());
        CodeSegmentCapacity = NewCapacity;
        if (OldAddress != static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(CodeSegment)))
        {
            repatchCodeSegment(OldAddress, CodeSegmentSize);
        }
    }

    /**
     * Repair internal bindings after CodeSegment has moved and its previous bytes have been preserved.
     * OldAddress is the former base as an integer; OldSize is the former valid byte count, not capacity.
     * The old allocation may already be freed, so all fields are read from the current buffer.
     *
     * Map entries inside [OldAddress, OldAddress + OldSize) retain their offsets relative to the new base.
     * Native host addresses lie outside that range and keep their values. Library bases and field positions
     * are already segment-relative offsets and do not move within the buffer.
     *
     * For each dynamic import relocation, undo Addend with unsigned subtraction to recover the selected symbol's
     * original base, including when Addend is INT64_MIN. Test that base against the old range: the stored
     * address itself may lie outside the range because of Addend. Translate only code-segment targets,
     * then reapply the checked Addend and rewrite the field:
     *   OldAddress + Offset + Addend  ->  NewAddress + Offset + Addend
     *
     * Image-base address relocations are recomputed from each library's original TargetOffset and Addend:
     *   NewAddress + Library.ImageBase + TargetOffset + Addend
     * This also handles one-past targets and results displaced outside the image by Addend; no old-address
     * range test is needed because the metadata already identifies the owning image.
     *
     * Do not resolve names again: later same-name exports must not rebind references that were already patched.
     * This repairs registered map entries and relocation fields, not pointers cached by external callers.
     */
    void Interpreter::repatchCodeSegment(std::uint64_t OldAddress, std::size_t OldSize)
    {
        if (OldSize == 0)
        {
            return;
        }
        const auto NewAddress = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(CodeSegment));
        for (auto& Symbol : ExportSymbolMap)
        {
            auto& Address = Symbol.second.Address;
            if (Address >= OldAddress && Address - OldAddress < OldSize)
            {
                Address = NewAddress + (Address - OldAddress);
            }
        }
        for (const auto& Entry : Libraries)
        {
            const auto& Library = Entry.second;
            const auto ImageSize = Library.ExecutableFile.byteSize();
            REPORT_ICE_IF(Library.ImageBase > OldSize || ImageSize > OldSize - Library.ImageBase, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "existing image", "loaded image is outside the code segment");
            for (const auto& Relocation : Library.ExecutableFile.getDynamicImportSymbolRelocations())
            {
                REPORT_ICE_IF(Relocation.ImageOffset > ImageSize || DynamicImportSymbolRelocation::ByteSize > ImageSize - Relocation.ImageOffset, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "existing dynamic import relocation", "address field is outside the image");
                auto* Destination = CodeSegment + Library.ImageBase + Relocation.ImageOffset;
                check(reinterpret_cast<std::uintptr_t>(Destination) % DynamicImportSymbolRelocation::ByteSize == 0);
                auto SymbolAddress = *reinterpret_cast<const std::uint64_t*>(Destination) - static_cast<std::uint64_t>(Relocation.Addend);
                if (SymbolAddress >= OldAddress && SymbolAddress - OldAddress < OldSize)
                {
                    SymbolAddress = NewAddress + (SymbolAddress - OldAddress);
                }
                *reinterpret_cast<std::uint64_t*>(Destination) = detail::addAddressOffset(SymbolAddress, Relocation.Addend, Diagnostics);
            }
            for (const auto& Relocation : Library.ExecutableFile.getImageBaseAddressRelocations())
            {
                REPORT_ICE_IF(Relocation.ImageOffset > ImageSize || ImageBaseAddressRelocation::ByteSize > ImageSize - Relocation.ImageOffset, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "existing image-base address relocation", "address field is outside the image");
                REPORT_ICE_IF(Relocation.TargetOffset > ImageSize, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "existing image-base address relocation", "target offset is outside the image");
                const auto TargetAddress = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(CodeSegment + Library.ImageBase + Relocation.TargetOffset));
                auto* Destination = CodeSegment + Library.ImageBase + Relocation.ImageOffset;
                check(reinterpret_cast<std::uintptr_t>(Destination) % ImageBaseAddressRelocation::ByteSize == 0);
                *reinterpret_cast<std::uint64_t*>(Destination) = detail::addAddressOffset(TargetAddress, Relocation.Addend, Diagnostics, "existing image-base address relocation");
            }
        }
    }

    std::uint32_t Interpreter::loadLibrary(ExecutableFile Executable)
    {
        if (NextLibraryHandle == InvalidLibraryHandle)
        {
            return InvalidLibraryHandle;
        }
        const auto ImageSize = Executable.getImage().size();
        const auto Padding = ImageSize == 0 ? std::size_t{0} : (InstructionEncoding::WordBytes - CodeSegmentSize % InstructionEncoding::WordBytes) % InstructionEncoding::WordBytes;
        REPORT_ICE_IF(Padding > std::numeric_limits<std::size_t>::max() - CodeSegmentSize || ImageSize > std::numeric_limits<std::size_t>::max() - CodeSegmentSize - Padding, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "image", "code segment size overflows");
        const auto ImageBase = CodeSegmentSize + Padding;
        reserveCodeSegment(ImageBase + ImageSize);
        LoadTransaction Transaction(*this);
        const std::uint32_t Handle = NextLibraryHandle++;
        auto& Library = Libraries.try_emplace(Handle, LibraryDescriptor{static_cast<std::uint64_t>(ImageBase), std::move(Executable)}).first->second;
        if (Padding != 0)
        {
            std::memset(CodeSegment + CodeSegmentSize, 0, Padding);
        }
        const auto& Image = Library.ExecutableFile.getImage();
        if (!Image.empty())
        {
            std::memcpy(CodeSegment + ImageBase, Image.data(), Image.size());
        }
        CodeSegmentSize = ImageBase + Image.size();
        if (!Loader.loadLibrary({CodeSegment, CodeSegmentSize}, Library.ImageBase, Library.ExecutableFile))
        {
            return InvalidLibraryHandle;
        }
        Transaction.commit();
        return Handle;
    }

    /**
     * Register exports from an image already present in CodeSegment. Library.ImageBase is its byte offset
     * in the segment; each export's Offset is relative to that image. Store the actual host address:
     *   Address = CodeSegment + Library.ImageBase + Symbol.Offset
     *
     * insert_or_assign gives later loaded or registered symbols precedence over earlier same-name VM or
     * Native bindings. This provides LD_PRELOAD-style symbol interposition with a last-registration-wins rule.
     * Subsequent name queries and import resolutions see the replacement; previously patched references
     * retain their original targets and are not retroactively rebound.
     */
    void Interpreter::loadExportSymbol(const LibraryDescriptor& Library)
    {
        REPORT_ICE_IF(Library.ImageBase > CodeSegmentSize || Library.ExecutableFile.byteSize() > CodeSegmentSize - Library.ImageBase, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "export symbol", "library image is outside the code segment");
        for (const auto& Symbol : Library.ExecutableFile.getExportSymbols())
        {
            REPORT_ICE_IF(Symbol.Offset >= Library.ExecutableFile.byteSize(), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "export symbol", "offset is outside the image");
            const auto Address = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(CodeSegment + Library.ImageBase + Symbol.Offset));
            ExportSymbolMap.insert_or_assign(Symbol.Name, RuntimeSymbol{Address, Symbol.Kind, SymbolProviderKind::VM, Symbol.Data, Symbol.Function});
        }
    }

    const ExecutableFile* Interpreter::findExecutable(std::uint32_t Handle) const noexcept
    {
        const auto Position = Libraries.find(Handle);
        return Position == Libraries.end() ? nullptr : &Position->second.ExecutableFile;
    }
} // namespace ink::vm
