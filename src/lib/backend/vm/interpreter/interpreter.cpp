#include "ink/backend/vm/interpreter/interpreter.h"
#include "relocation_address.h"
#include "ink/backend/vm/vm_runtime/runtime.h"
#include "ink/core/config/config_manager.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <utility>

namespace ink::vm
{
  class Interpreter::LoadTransaction
  {
    public:
      explicit LoadTransaction(Interpreter &Owner)
          : Owner(Owner),
            SavedCodeSize(Owner.CodeSegmentSize),
            SavedExportSymbolMap(std::move(Owner.ExportSymbolMap)),
            SavedNextLibraryHandle(Owner.NextLibraryHandle)
      {
        Owner.ExportSymbolMap = SavedExportSymbolMap;
      }

      ~LoadTransaction()
      {
        if (Committed)
        {
          return;
        }
        Owner.CodeSegmentSize = SavedCodeSize;
        Owner.ExportSymbolMap.swap(SavedExportSymbolMap);
        for (auto Position = Owner.Libraries.begin(); Position != Owner.Libraries.end();)
        {
          if (Position->first >= SavedNextLibraryHandle)
          {
            Position = Owner.Libraries.erase(Position);
          }
          else
          {
            ++Position;
          }
        }
        Owner.NextLibraryHandle = SavedNextLibraryHandle;
      }

      LoadTransaction(const LoadTransaction &) = delete;
      LoadTransaction &operator=(const LoadTransaction &) = delete;
      LoadTransaction(LoadTransaction &&) = delete;
      LoadTransaction &operator=(LoadTransaction &&) = delete;

      void commit() noexcept
      {
        Committed = true;
      }

    private:
      Interpreter &Owner;
      std::size_t SavedCodeSize;
      std::unordered_map<core::Name, std::uint64_t> SavedExportSymbolMap;
      std::uint32_t SavedNextLibraryHandle;
      bool Committed = false;
  };

  Interpreter::Interpreter() noexcept
      : Loader(*this)
  {
    reserveCodeSegment(core::ConfigManager::getSize<core::ConfigKind::VMInitialCodeSegmentBytes>());
    initialize();
  }

  Interpreter::~Interpreter()
  {
    std::free(CodeSegment);
  }

  void Interpreter::initialize()
  {
    for (const auto &Symbol : NativeSymbolAddresses)
    {
      ExportSymbolMap.try_emplace(Symbol.Name, static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(Symbol.Address)));
    }
  }

  void *Interpreter::resolveSymbolAddress(core::Name Name) const noexcept
  {
    const auto Position = ExportSymbolMap.find(Name);
    return Position == ExportSymbolMap.end() ? nullptr : reinterpret_cast<void *>(static_cast<std::uintptr_t>(Position->second));
  }

  /**
   * Ensure at least RequiredBytes total capacity, not an additional byte count; CodeSegmentSize is unchanged.
   * Existing capacity is reused. Growth normally doubles capacity without overflowing and always covers
   * RequiredBytes. The initial allocation uses malloc; an existing allocation first tries realloc.
   *
   * If realloc fails, the old block remains valid: allocate a replacement, copy only CodeSegmentSize bytes,
   * then release the old block. Preserve the old base as an integer because a successful realloc may also
   * move storage and invalidate the old pointer. An in-place extension needs no address repair; either kind
   * of move calls repatchCodeSegment after the new allocation is installed.
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
    const auto NewCapacity = CodeSegmentCapacity <= std::numeric_limits<std::size_t>::max() / 2 ? std::max(RequiredBytes, CodeSegmentCapacity * 2) : RequiredBytes;
    const auto OldAddress = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(CodeSegment));
    auto *Allocation = CodeSegment ? static_cast<std::uint8_t *>(std::realloc(CodeSegment, NewCapacity)) : static_cast<std::uint8_t *>(std::malloc(NewCapacity));
    if (!Allocation)
    {
      Allocation = static_cast<std::uint8_t *>(std::malloc(NewCapacity));
      REPORT_ICE_IF(!Allocation, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "code segment", "runtime storage allocation failed");
      if (CodeSegmentSize != 0)
      {
        std::memcpy(Allocation, CodeSegment, CodeSegmentSize);
      }
      std::free(CodeSegment);
    }
    CodeSegment = Allocation;
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
   * For every existing relocation, undo Addend with unsigned subtraction to recover the selected symbol's
   * original base, including when Addend is INT64_MIN. Test that base against the old range: the stored
   * address itself may lie outside the range because of Addend. Translate only code-segment targets,
   * then reapply the checked Addend and rewrite the field:
   *   OldAddress + Offset + Addend  ->  NewAddress + Offset + Addend
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
    for (auto &Symbol : ExportSymbolMap)
    {
      auto &Address = Symbol.second;
      if (Address >= OldAddress && Address - OldAddress < OldSize)
      {
        Address = NewAddress + (Address - OldAddress);
      }
    }
    for (const auto &Entry : Libraries)
    {
      const auto &Library = Entry.second;
      for (const auto &Relocation : Library.ExecutableFile.getDynamicImportSymbolRelocations())
      {
        REPORT_ICE_IF(Library.ImageBase > OldSize || Relocation.ImageOffset > OldSize - Library.ImageBase || DynamicImportSymbolRelocation::ByteSize > OldSize - Library.ImageBase - Relocation.ImageOffset, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "existing dynamic import relocation", "address field is outside the code segment");
        auto *Destination = CodeSegment + Library.ImageBase + Relocation.ImageOffset;
        check(reinterpret_cast<std::uintptr_t>(Destination) % DynamicImportSymbolRelocation::ByteSize == 0);
        auto SymbolAddress = *reinterpret_cast<const std::uint64_t *>(Destination) - static_cast<std::uint64_t>(Relocation.Addend);
        if (SymbolAddress >= OldAddress && SymbolAddress - OldAddress < OldSize)
        {
          SymbolAddress = NewAddress + (SymbolAddress - OldAddress);
        }
        *reinterpret_cast<std::uint64_t *>(Destination) = detail::addAddressOffset(SymbolAddress, Relocation.Addend, Diagnostics);
      }
    }
  }

  std::uint32_t Interpreter::loadLibrary(ExecutableFile Executable)
  {
    if (NextLibraryHandle == InvalidLibraryHandle)
    {
      return InvalidLibraryHandle;
    }
    REPORT_ICE_IF(Executable.getImage().size() > std::numeric_limits<std::size_t>::max() - CodeSegmentSize, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "image", "code segment size overflows");
    reserveCodeSegment(CodeSegmentSize + Executable.getImage().size());
    LoadTransaction Transaction(*this);
    const std::uint32_t Handle = NextLibraryHandle++;
    auto &Library = Libraries.try_emplace(Handle, LibraryDescriptor{static_cast<std::uint64_t>(CodeSegmentSize), std::move(Executable)}).first->second;
    const auto &Image = Library.ExecutableFile.getImage();
    if (!Image.empty())
    {
      std::memcpy(CodeSegment + CodeSegmentSize, Image.data(), Image.size());
    }
    CodeSegmentSize += Image.size();
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
  void Interpreter::loadExportSymbol(const LibraryDescriptor &Library)
  {
    REPORT_ICE_IF(Library.ImageBase > CodeSegmentSize || Library.ExecutableFile.byteSize() > CodeSegmentSize - Library.ImageBase, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "export symbol", "library image is outside the code segment");
    for (const auto &Symbol : Library.ExecutableFile.getExportSymbols())
    {
      REPORT_ICE_IF(Symbol.Offset >= Library.ExecutableFile.byteSize(), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "export symbol", "offset is outside the image");
      const auto Address = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(CodeSegment + Library.ImageBase + Symbol.Offset));
      ExportSymbolMap.insert_or_assign(Symbol.Name, Address);
    }
  }

  const ExecutableFile *Interpreter::findExecutable(std::uint32_t Handle) const noexcept
  {
    const auto Position = Libraries.find(Handle);
    return Position == Libraries.end() ? nullptr : &Position->second.ExecutableFile;
  }
} // namespace ink::vm
