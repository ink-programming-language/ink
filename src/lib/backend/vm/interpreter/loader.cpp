#include "ink/backend/vm/interpreter/loader.h"
#include "relocation_address.h"
#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/core/name/name_pool.h"
#include "ink/core/support/core_define.h"

#include <spdlog/fmt/fmt.h>

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#if INK_PLATFORM_BIG_ENDIAN
#error "Ink VM runtime requires a little-endian target"
#endif

namespace ink::vm
{
  namespace
  {
    bool reportLinkFailure(const core::DiagnosticEngine &Diagnostics, core::Name Name, std::size_t Index)
    {
      const auto Context = fmt::format("dynamic import #{} ('{}')", Index, core::NamePool::instance().text(Name));
      Diagnostics.report<core::DiagnosticKind::VMLibraryLinkFailed>({}, Context, "symbol address was not found");
      return false;
    }
  } // namespace

  Loader::Loader(vm::Interpreter &Interpreter) noexcept
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
  bool Loader::loadLibrary(std::span<std::uint8_t> CodeSegment, std::uint64_t ImageBase, ExecutableFile &Executable)
  {
    const auto &Diagnostics = Interpreter.getDiagnostics();
    const auto CodeSize = static_cast<std::uint64_t>(CodeSegment.size());
    const auto ImageSize = Executable.byteSize();
    const auto CodeAddress = reinterpret_cast<std::uintptr_t>(CodeSegment.data());
    const auto ImageAddress = reinterpret_cast<std::uintptr_t>(Executable.getImage().data());
    const bool AliasesImage = CodeSize != 0 && ImageSize != 0 && (CodeAddress <= ImageAddress ? ImageAddress - CodeAddress < CodeSize : CodeAddress - ImageAddress < ImageSize);
    REPORT_ICE_IF(AliasesImage, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "image", "code segment aliases the unbound image");
    REPORT_ICE_IF(ImageBase > CodeSize || ImageSize > CodeSize - ImageBase, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "image", "loaded image is outside the code segment");
    for (const auto &Export : Executable.getExportSymbols())
    {
      REPORT_ICE_IF(Export.Offset >= ImageSize, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "export symbol", "offset is outside the image");
      const auto Address = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(CodeSegment.data() + ImageBase + Export.Offset));
      Interpreter.ExportSymbolMap.insert_or_assign(Export.Name, Address);
    }

    const auto &Imports = Executable.getDynamicImportSymbols();
    std::vector<std::uint64_t> SymbolAddresses;
    SymbolAddresses.reserve(Imports.size());
    for (std::size_t Index = 0; Index < Imports.size(); ++Index)
    {
      const auto Address = Interpreter.resolveSymbolAddress(Imports[Index].Name);
      if (!Address)
      {
        return reportLinkFailure(Diagnostics, Imports[Index].Name, Index);
      }
      SymbolAddresses.push_back(static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(Address)));
    }

    for (const auto &Relocation : Executable.getDynamicImportSymbolRelocations())
    {
      REPORT_ICE_IF(Relocation.SymbolIndex >= SymbolAddresses.size(), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "dynamic import relocation", "symbol index is outside the import table");
      REPORT_ICE_IF(Relocation.ImageOffset > ImageSize || DynamicImportSymbolRelocation::ByteSize > ImageSize - Relocation.ImageOffset, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "dynamic import relocation", "address field is outside the image");
      const auto SymbolAddress = SymbolAddresses[Relocation.SymbolIndex];
      const auto Address = detail::addAddressOffset(SymbolAddress, Relocation.Addend, Diagnostics);
      const auto CodeOffset = static_cast<std::size_t>(ImageBase + Relocation.ImageOffset);
      auto *Destination = CodeSegment.data() + CodeOffset;
      check(reinterpret_cast<std::uintptr_t>(Destination) % DynamicImportSymbolRelocation::ByteSize == 0);
      *reinterpret_cast<std::uint64_t *>(Destination) = Address;
    }
    for (const auto &Relocation : Executable.getImageBaseAddressRelocations())
    {
      REPORT_ICE_IF(Relocation.ImageOffset > ImageSize || ImageBaseAddressRelocation::ByteSize > ImageSize - Relocation.ImageOffset, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "image-base address relocation", "address field is outside the image");
      REPORT_ICE_IF(Relocation.TargetOffset > ImageSize, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "image-base address relocation", "target offset is outside the image");
      const auto TargetAddress = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(CodeSegment.data() + ImageBase + Relocation.TargetOffset));
      const auto Address = detail::addAddressOffset(TargetAddress, Relocation.Addend, Diagnostics, "image-base address relocation");
      auto *Destination = CodeSegment.data() + ImageBase + Relocation.ImageOffset;
      check(reinterpret_cast<std::uintptr_t>(Destination) % ImageBaseAddressRelocation::ByteSize == 0);
      *reinterpret_cast<std::uint64_t *>(Destination) = Address;
    }
    return true;
  }
} // namespace ink::vm
