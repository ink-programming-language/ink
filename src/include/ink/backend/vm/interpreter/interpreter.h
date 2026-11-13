#pragma once

#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/interpreter/loader.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <unordered_map>

namespace ink::vm
{
  struct LibraryDescriptor
  {
      // The image's byte offset in CodeSegment, preserved when runtime storage moves.
      std::uint64_t ImageBase = 0;
      vm::ExecutableFile ExecutableFile;
  };

  class Interpreter
  {
    public:
      static constexpr std::uint32_t InvalidLibraryHandle = 0;

      Interpreter() noexcept;
      ~Interpreter();

      Interpreter(const Interpreter &) = delete;
      Interpreter &operator=(const Interpreter &) = delete;
      Interpreter(Interpreter &&) = delete;
      Interpreter &operator=(Interpreter &&) = delete;

      // Registers built-in native addresses without overwriting symbols already present in the table.
      void initialize();

      // Returned code addresses must be queried again after a load attempt that moves the code segment.
      [[nodiscard]] void *resolveSymbolAddress(core::Name Name) const noexcept;

      // Owns Executable; recoverable link failures restore logical state and return InvalidLibraryHandle.
      // Storage growth happens before linking and may invalidate borrowed code addresses even if linking fails.
      // Invalid internal metadata or address arithmetic reports an ICE and terminates.
      // Successful handles are never reused; handle exhaustion also returns InvalidLibraryHandle.
      [[nodiscard]] std::uint32_t loadLibrary(ExecutableFile Executable);

      /**
       * Register host addresses from Library's already loaded image; later same-name exports take precedence.
       * Overrides affect subsequent resolution and leave previously patched references bound to their targets.
       */
      void loadExportSymbol(const LibraryDescriptor &Library);

      // The borrowed file remains valid until this interpreter is destroyed; an unknown handle returns nullptr.
      const ExecutableFile *findExecutable(std::uint32_t Handle) const noexcept;

      // Callers can attach consumers to the interpreter-owned diagnostic engine.
      auto &getDiagnostics(this auto &&Self) noexcept
      {
        return Self.Diagnostics;
      }

      // This view borrows the allocation and must be refreshed after storage moves.
      std::span<const std::uint8_t> getCodeSegment() const noexcept
      {
        return {CodeSegment, CodeSegmentSize};
      }

      std::size_t getCodeSegmentCapacity() const noexcept
      {
        return CodeSegmentCapacity;
      }

      const std::unordered_map<core::Name, std::uint64_t> &getExportSymbolMap() const noexcept
      {
        return ExportSymbolMap;
      }

    private:
      friend class vm::Loader;
      class LoadTransaction;

      /** Ensure RequiredBytes total capacity without changing CodeSegmentSize; repatch if storage moves. */
      void reserveCodeSegment(std::size_t RequiredBytes);
      /** Rebase internal bindings from the old valid range after CodeSegment has moved to new storage. */
      void repatchCodeSegment(std::uint64_t OldAddress, std::size_t OldSize);

      std::unordered_map<std::uint32_t, LibraryDescriptor> Libraries;
      std::uint8_t *CodeSegment = nullptr;
      std::size_t CodeSegmentSize = 0;
      std::size_t CodeSegmentCapacity = 0;
      std::unordered_map<core::Name, std::uint64_t> ExportSymbolMap;
      core::DiagnosticEngine Diagnostics;
      vm::Loader Loader;
      std::uint32_t NextLibraryHandle = 1;
  };
} // namespace ink::vm
